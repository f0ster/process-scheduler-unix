#ifndef sch_H
#define sch_H

#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include "unixProcess.h"


using namespace std;
class sch {
	protected:
		vector<unixProcess> future_list;
		vector<unixProcess> done_list;
		vector<int> cpu_hist;
		int recalc_quantum;
		int rr_quantum;
		int current_queue;
		int queue_total;
		vector<unixProcess> queues[3];
		bool interrupt;
		int thetime;
		
	public:

		sch(int recalc_q, int timeQ) 
		{ 
			rr_quantum = timeQ;
			thetime = 0;
			queue_total = 3;
			interrupt = false;
			current_queue = 0;
			recalc_quantum = recalc_q;
		};
		~sch() { };

		class proxyComparePriority {
			sch& that;
			public:
			proxyComparePriority(sch &h) : that(h) {}
			bool operator()(unixProcess const &p1,unixProcess const &p2) const {
				return (p1.getPid() < p2.getPid());	
			}
		};
		int loadinput(char *filename) {
			ifstream testfile(filename); 
			//make sure file exists before calling sed
			if (!testfile) {
				cerr << "error loading input file!\n";
				return 0;
			}
			testfile.close();
			string cmd = "sed \'s/\t/ /g\' " + string(filename) + " > sample.clean";
			system(cmd.c_str());
			ifstream infile("sample.clean");
			if (!infile){
				cerr << "error loading file after clean-up!\n";
				return 0;
			}

			vector<string> input;
			string tmp_str;
			while( !infile.eof() )
			{
				getline(infile, tmp_str);
				input.push_back(tmp_str);
			}
			infile.close();
			system("rm sample.clean");

			//all lines loaded into string vector :input:
			//now parse through string vector and make unixProcess vector
			for(unsigned int i=0; i < input.size() ; i++) {
				int pid=0,burst=0,arrv=0,priority=0,nice=0;
				char *pch;
				/*char str[ strlen(input[i].c_str())+1 ];
				strcpy (str,input[i].c_str());*/
				char *str = new char[strlen(input[i].c_str())+1];
				strcpy (str,input[i].c_str());
				
				pch = strtok (str," ");
				int count = 1;
				while(pch!=NULL && count < 6){
					if ( atoi(pch) != 0 ) { // make sure it is a number
						if (count == 1)
							pid=atoi(pch);
						if (count == 2)
							burst=atoi(pch);
						if (count == 3)
							arrv=atoi(pch);
						if (count == 4)
							priority=atoi(pch);
						if (count == 5)
							nice=atoi(pch);
					}
					count++;
					pch = strtok(NULL," ");
				}
				delete[] str;
				if (pid != 0)
					future_list.push_back(unixProcess(pid,burst,arrv,priority,nice));
			}
			
			return 0;
		};
		
		void UPDATE(){
			unixProcess* it = &queues[this->queue_total-1][0];
			//recalculate priorities
			// priority = base * (burst/4) + nice
			if(thetime % recalc_quantum == 0)
			for(unsigned int i=0; i < 3; i++){
				for(unsigned int j=0; j<queues[i].size() ; j++){
					queues[i][j].calc();
				}
			}
			unsigned int sizes[3];
			sizes[0] = queues[0].size();
			sizes[1] = queues[1].size();
			sizes[2] = queues[2].size();
			int k=0;
			for(unsigned int i=0; i < future_list.size() ; i++){
				it= &future_list[i];
				if(thetime >= it->arrival)
				{
					//cout << "new proc from future " << endl << *it << endl;
					if(it->priority < 30)
						k=0;
					if(it->priority >= 30 && it->priority < 60)
						k=1;
					if(it->priority >= 60)
						k=2;
					queues[k].push_back(*it);
					future_list.erase( future_list.begin()+i );
					if( current_queue != 0)
						interrupt = true;
				}	
			}
			for(int i=0; i < 3; i++){
				if (sizes[i] != queues[i].size())
					sort( queues[i].begin(), queues[i].end(), proxyComparePriority(*this) );
			}

		}
		
		void run(){
			bool done = false;
			while(!done){
				interrupt = false;
				for( int i=0; i < queue_total && !interrupt ; i++ ) {
					current_queue = i;
					if (!queues[i].empty())
						do_priority(i); // do rr on q(i) 
					else {
							if (!future_list.empty()){
								update_clock();
								cpu_hist.push_back(-1);
							}
							else{
								bool allEmpty=true;
								for(int j=0; j<queue_total && allEmpty; j++){
									if (!queues[j].empty())
										allEmpty=false;
								}
								if (allEmpty)
									done=true;
							}
					}
				}
			}
			print_all(1);
		};
		
		/*
		int do_rr_q(int i){
			//cout << "do_rr_q(" << i << ")..." << endl;
			int subtime = 0;
			while(!interrupt && !queues[i].empty()){
				unixProcess* first = &queues[i][0];
				if (first->burst == first->timeRemaining)
					first->doneWaiting = thetime;
				for(subtime = 0; (subtime < timeQ*(i+1)) && !interrupt && first->timeRemaining > 0; subtime++){
					//cout << "pid " << first->getPid() << " remaining " << first->timeRemaining << endl; 	
					first->timeRemaining--;
					cpu_hist.push_back(first->getPid());
					update_clock(); //calls update;
					first = &queues[i][0];
				}			
				if( first->timeRemaining <= 0){
					first->finishTime = thetime;
					done_list.push_back( *first );
					queues[i].erase( queues[i].begin() );
				} else{
					//cout << "moving pid " << first->getPid() << " to [" << i << "]" << endl;
					queues[i+1].push_back( unixProcess(*first) ); //push
					queues[i].erase( queues[i].begin() );				//pop
				}
			}
			return (int)interrupt;
		};*/
		int do_priority(int i){
			int subtime =0;
			while(!interrupt && !queues[i].empty()){
				unixProcess* first = &queues[i][0];
				if( first->burst == first->timeRemaining )
					first->doneWaiting = thetime;
				for(subtime =0; (subtime < rr_quantum) && !interrupt && first->timeRemaining >0; subtime++){
					first->timeRemaining--;
					cpu_hist.push_back( first->getPid() );
					update_clock();
					first = &queues[i][0];
				}
				if (first->timeRemaining <=0){
					first->finishTime = thetime;
					done_list.push_back( *first );
					queues[i].erase( queues[i].begin() );
				} else{
					queues[i].push_back( unixProcess(*first) ); //push
					queues[i].erase( queues[i].begin() );
				}
			}
			return (int)interrupt;
		};/*
		int do_fcfs(int i) {
				//cout << "do_fcfs(" << i << ")..." << endl; 		
			while(!interrupt && !queues[i].empty()){
				unixProcess* first = &queues[i][0];
				for(; !interrupt && first->timeRemaining > 0 ;){
					first->timeRemaining--;
					cpu_hist.push_back(first->getPid());
					update_clock();
					first = &queues[i][0];
				}
				if (first->timeRemaining <= 0 ){
					first->finishTime = thetime;
					done_list.push_back( *first );
					queues[i].erase( queues[i].begin() );
				}
			}
			return (int)interrupt;
		};*/
		void update_clock(){
			//cout << "clock "<< thetime+1 <<endl;
			thetime++;
			UPDATE();
		};
		void print_all(int k){
			vector<unixProcess>::iterator fut;
			vector<unixProcess>::iterator active;
			cout << "Future Processes" << endl;
			for(fut = future_list.begin(); fut != future_list.end(); fut++){
				cout << *fut << endl;
			}
			for(int i=0; i < queue_total; i++){
				cout << "Active Process Queue[" <<i<<"]" << endl;
				for(active = queues[i].begin(); active != queues[i].end(); active++){
					cout << *active << endl;
				}
			}
			if(k==1){
				cout << "CPU history\t(-1 signifies IDLE)" <<endl;
				for(unsigned int i=0; i<cpu_hist.size(); i++){
					cout << cpu_hist[i] << " ";
				} cout << endl;
			}
		};
	
		void stats() {
			double avgTurn = 0;
			double avgWait = 0;
			double count = 0;
			vector<unixProcess>::iterator it;
			for(it = done_list.begin(); it != done_list.end(); it++){
				count++;
				avgTurn += (it->finishTime+1 - it->arrival);
				avgWait += (it->doneWaiting - it->arrival);
			}
			avgTurn /= count;
			avgWait /= count;
			cout << "Average Turnaround Time: " << avgTurn << endl;
			cout << "Average Waiting Time: " << avgWait << endl;
				
			//	cout << avgTurn << "\t" << avgWait << endl;

		};
};




#endif

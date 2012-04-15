#ifndef UNIXPROCESS_H
#define UNIXPROCESS_H

#include <iostream>
#include <string>

using namespace std;

class unixProcess {
	protected:
		int pid;
	
	public:
		int burst;
		int arrival;
		int timeRemaining;
		int doneWaiting;
		int finishTime;
		int priority;
		int calc_priority;
		int nice;

		unixProcess() {
			this->pid = 0;
			this->burst = 0;
			this->arrival = 0;
			this->timeRemaining =0;
			this->doneWaiting = 0;
			this->finishTime = 0;
			this->priority = 0;
			this->calc_priority =0;
			this->nice = 0;
		};
		unixProcess(int pid, int burst, int arrival, int priority, int nice) {
			this->pid = pid;
			this->burst = burst;
			this->arrival = arrival;
			this->timeRemaining = burst;
			this->priority = priority;
			this->doneWaiting = 0;
			this->finishTime = 0;
			this->calc_priority = this->calc();
			this->nice = nice;
		};
		~unixProcess() {
			
		};
		unixProcess( const unixProcess &p) {
			//cout << "p being copied from" << endl << p << endl; 
			pid = p.getPid();
			burst = p.burst;
			arrival = p.arrival;
			timeRemaining = p.timeRemaining;
			priority = p.priority;
			doneWaiting = p.doneWaiting;
			finishTime = p.finishTime;
			calc_priority = p.calc_priority;
			nice = p.nice;
		};
		unixProcess& operator= (const unixProcess &p){
			pid = p.getPid();
			burst = p.burst;
			arrival = p.arrival;
			timeRemaining = p.timeRemaining;
			priority = p.priority;
			doneWaiting = p.doneWaiting;
			finishTime = p.finishTime;
			calc_priority = p.calc_priority;
			nice = p.nice;
			return *this;
		};
		bool operator== (const unixProcess &p) {
			return (this->pid==p.getPid() && this->arrival == p.arrival && this->burst == p.burst);
		};
		bool operator!= (const unixProcess &p){
			return !(this->pid==p.getPid() && this->arrival == p.arrival && this->burst == p.burst);
		};
		friend ostream& operator<< (ostream &os, const unixProcess &p) {
			p.display(os);
			return os;
		};
		void display(ostream &os) const {
			os << "\t" << getPid();
			os << "\t" << burst;
			os << "\t" << arrival;
			os << "\t" << priority;
			os << "\t" << calc_priority;
			os << "\t" << nice;
			os << "\t\t" << timeRemaining;
		};
		int calc(){
			int limit=0;
			if (0 <= priority && priority < 30)
				limit = 30;
			if (30 <= priority && priority < 60)
				limit = 60;
			if (priority >= 60)
				limit = 90;

			calc_priority = priority + ( (burst-timeRemaining)/4 ) + nice;
			if(calc_priority < limit-30)
				calc_priority = limit-30;
			if(calc_priority >= limit)
				calc_priority = limit-1;

			return calc_priority;

		};
		int getPid() const{
			return this->pid;
		};

};
#endif


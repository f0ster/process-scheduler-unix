#include "sch.h"
#include <cstdlib>



int main(int argc, char **argv){
	if( argc == 4) {
		if( (atoi(argv[3]) < 1) && (atoi(argv[4]) < 1) )   {
			cout << "Usage: unix <input_file> <recalculate quantum > 0> <time_quantum > 0>";
			return 0;
		}
	}
	if( argc != 2 && argc != 4) {
		cout << "Usage: unix <input_file> <recalculate quantum> <time_quantum>" << endl;
		cout << "OR\nUsage: unix <input_file>" << endl;
		return 0;
	}

		int quantum =0;
		int time = 0;
		if(argc == 2){
		cout << "Please choose a recalculate quantum" << endl;
		cin >> time;
		doover:
		cout << "Please choose a time quantum (>0) for RR" << endl;
		cin >> quantum;
		if (quantum <=0)
			goto doover;
		}else
		{
			time = atoi(argv[3]);
			quantum = atoi(argv[2]);
		}
		sch sched(time,quantum);
		sched.loadinput(argv[1]);
		sched.UPDATE();
		sched.print_all(0);
		sched.run();
		sched.stats();
		exit(0);
	return 0;
}

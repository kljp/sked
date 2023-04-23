#ifndef __HPP_WORKER__
#define __HPP_WORKER__

#include <iostream>
//#include <string>
//#include <cstdlib>
#include <random>
//#include <ctime>
#include <unistd.h>
#include "job.hpp"
#include "channel/channel.hpp"

class worker{

	private:
		int id_wk = -1;
		int type_wk = 0;
		int type_queue = 0;
		int cnt_job = 0;
		bool is_active = false;
	
	public:
		worker(){}
		~worker(){}
		worker(int id_wk, int type_wk, int type_queue){
			
			this->id_wk = id_wk;
			this->type_wk = type_wk;
			this->type_queue = type_queue;
			is_active = true;
		}
		
		void be_active(){is_active = true; return;}

		void be_inactive(){is_active = false; return;}

		void inc_cnt_job(){cnt_job++; return;}

		int get_cnt_job(){return cnt_job;}

		job batch(int req_core, int t_req){

			std::string id_jb = std::to_string(id_wk) + "_" + std::to_string(cnt_job);
			job jb(id_jb, id_wk, 3, type_queue, req_core, t_req);
			cnt_job++;
			return jb;
		}
};

void run_worker(int id_wk, msd::channel<job> &jobs, int num_queue){

	thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, 1000000);
	worker wk(id_wk, 1, distribution(generator) % num_queue);
	while(true){
		usleep(100000);
		bool pred = distribution(generator) % 100 < 7 ? true : false;
		if(pred){
			int req_core = (distribution(generator) % 8 + 1) * 2;
			int t_req = (distribution(generator) + 1) % 60 + 100;
			wk.batch(req_core, t_req) >> jobs;
//			break;
		}
	}
}

#endif

#ifndef __HPP_WORKER__
#define __HPP_WORKER__

#include <iostream>
//#include <string>
//#include <cstdlib>
#include <random>
#include <algorithm>
//#include <ctime>
#include <unistd.h>
#include "job.hpp"
#include "channel/channel.hpp"
#include "timer.hpp"

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

		job batch(int type_queue, int req_core, int t_req){

			std::string id_jb = std::to_string(id_wk) + "_" + std::to_string(cnt_job);
			job jb(id_jb, id_wk, 3, type_queue, 0, req_core, t_req);
			cnt_job++;
			return jb;
		}
};

void run_worker(int id_wk, msd::channel<job> &jobs, msd::channel<job> &jobs_log, bool flag_log, int num_queue){

	thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, 1000000);
	worker wk(id_wk, 1, distribution(generator) % num_queue);
	int t_st = timer();
	bool pred;
	std::vector<job> temp_vec;
	int t_curr = t_st;
	int t_prev = t_st;

	while(true){
		usleep(1000);
		if(jobs_log.size() == 0)
			break;
		job jb = jobs_log.get_queue().front();
		int t_submit = jb.get_t_submit();
		if(flag_log){
			t_curr = timer();
			pred = t_curr - t_st > t_submit ? true : false;
			if(pred){
				job jb;
				jb << jobs_log;
				jb.set_id_wk(id_wk);
				temp_vec.push_back(jb);
			}

			int sz_temp_vec = temp_vec.size();
			if(t_curr - t_prev > 2 && sz_temp_vec > 0){
				/******************************************
				 **** Scheduling policy should be here ****
				 ******************************************/
				// 1. FCFS
				// do nothing
				// 2. SJF
				std::sort(temp_vec.begin(), temp_vec.end(), [](job x, job y) -> bool{
					int t_x = x.get_t_req();
					int t_y = y.get_t_req();
					if(t_x == t_y) return x.get_t_submit() < y.get_t_submit();
					return t_x < t_y;
				}); // SJF finished
				for(int i = 0; i < sz_temp_vec; i++){
					usleep(65000);
					temp_vec[i] >> jobs;
				}
				temp_vec.clear(); // memory capcity is maintained
				t_prev = t_curr;
			}
		}
		else{
			pred = distribution(generator) % 100 < 50 ? true : false;
			if(pred){
				int type_queue = distribution(generator) % num_queue;
				int req_core = (distribution(generator) % 8 + 1) * 2;
				int t_req = distribution(generator) % 160 + 10;
				wk.batch(type_queue, req_core, t_req) >> jobs;
			}
		}
	}
}

#endif

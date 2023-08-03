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

		job batch(int type_queue, int t_submit, int req_core, int t_req){

			std::string id_jb = std::to_string(id_wk) + "_" + std::to_string(cnt_job);
			job jb(id_jb, id_wk, 3, type_queue, t_submit, req_core, t_req);
			cnt_job++;
			return jb;
		}
};

void run_worker(int id_wk, msd::channel<job> &jobs, msd::channel<job> &jobs_log, std::vector<std::vector<host>> &topo, bool flag_log, int num_queue){

	thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, 1000000);
	worker wk(id_wk, 1, distribution(generator) % num_queue);
	int t_st = timer();
	bool pred;
	std::vector<job> temp_vec;
	int t_curr = t_st;
	int t_prev = t_st;
	std::vector<std::vector<host>> virt_topo;

	while(true){
		usleep(1000);
		if(flag_log){
			if(jobs_log.size() == 0){
				while(true){
					virt_topo = topo;
					int ovr_core = 0;
					for(int i = 0; i < virt_topo.size(); i++){
						for(int j = 0; j < virt_topo[i].size(); j++)
							ovr_core += (virt_topo[i][j].max_core - virt_topo[i][j].avail_core);
					}
					if(ovr_core == 0)
						exit(-1);
				}

			}

			job jb = jobs_log.get_queue().front();
			int t_submit = jb.get_t_submit();
			t_curr = timer();
			pred = t_curr - t_st > t_submit ? true : false;
			if(pred){
				job jb;
				jb << jobs_log;
				jb.set_id_wk(id_wk);
				jb.set_t_submit(timer());
				temp_vec.push_back(jb);
			}
			int sz_temp_vec = temp_vec.size();
			int t_flush = t_curr - t_prev;
			if(t_flush > 2 && sz_temp_vec > 0){
				/******************************************
				 **** Scheduling policy should be here ****
				 ******************************************/
				// 1. FCFS
				
				for(int i = 0; i < sz_temp_vec; i++){
					usleep(65000);
					temp_vec[i] >> jobs;
				}
				temp_vec.clear(); // memory capcity is maintained
				
				// FCFS finished
				// 2. SJF
				/*
				std::sort(temp_vec.begin(), temp_vec.end(), [](job x, job y) -> bool{
					int t_x = x.get_t_req();
					int t_y = y.get_t_req();
					if(t_x == t_y) return x.get_t_submit() < y.get_t_submit();
					return t_x < t_y;
				});

				for(int i = 0; i < sz_temp_vec; i++){
					usleep(65000);
					temp_vec[i] >> jobs;
				}
				temp_vec.clear(); // memory capcity is maintained
				*/
				// SJF finished
				// 3. SDC algorithm
				/*
				virt_topo = topo;
				for(int i = 0; i < sz_temp_vec; i++)
					temp_vec[i].set_score((float) temp_vec[i].get_req_core() * (t_curr - temp_vec[i].get_t_submit()) / t_flush);
				std::sort(temp_vec.begin(), temp_vec.end(), [](job x, job y) -> bool{
					int t_x = x.get_score();
					int t_y = y.get_score();
					if(t_x == t_y) return x.get_req_core() > y.get_req_core();
					return t_x > t_y;
				});
				std::vector<job> vec_a;
				std::vector<job> vec_b;
				std::vector<job> vec_b_temp;
				std::vector<job> vec_c;
				for(int i = 0; i < sz_temp_vec; i++){
					int min_val = 4096;
					int min_idx = -1;
					int t_q = temp_vec[i].get_type_queue();
					for(int j = 0; j < virt_topo[t_q].size(); j++){
						if(virt_topo[t_q][j].avail_core >= temp_vec[i].get_req_core() && virt_topo[t_q][j].avail_core <= min_val){
							min_val = virt_topo[t_q][j].avail_core;
							min_idx = j;
						}
					}
					if(min_idx != -1){
						virt_topo[t_q][min_idx].avail_core -= temp_vec[i].get_req_core();
						vec_a.push_back(temp_vec[i]);
					}
					else
						vec_b_temp.push_back(temp_vec[i]);
				}
				int sz_vec_b_temp = vec_b_temp.size();
				for(int i = 0; i < sz_vec_b_temp; i++){
					int min_val = 4096;
					int min_idx_q = -1;
					int min_idx_h = -1;
					int t_q = vec_b_temp[i].get_type_queue();
					for(int j = 0; j < virt_topo.size(); j++){
						if(j == t_q)
							continue;
						for(int k = 0; k < virt_topo[j].size(); k++){
							if(virt_topo[j][k].avail_core >= vec_b_temp[i].get_req_core() && virt_topo[j][k].avail_core < min_val){
								min_val = virt_topo[j][k].avail_core;
								min_idx_q = j;
								min_idx_h = k;
							}
						}
					}
					if(min_idx_q != -1){
						virt_topo[min_idx_q][min_idx_h].avail_core -= vec_b_temp[i].get_req_core();
						vec_b_temp[i].set_type_queue(min_idx_q);
						vec_b.push_back(vec_b_temp[i]);
					}
					else
						vec_c.push_back(vec_b_temp[i]);
				}
				int sz_vec_a = vec_a.size();
				int sz_vec_b = vec_b.size();
				int sz_vec_c = vec_c.size();
				for(int i = 0; i < sz_vec_a; i++){
					usleep(65000);
					vec_a[i] >> jobs;
				}
				for(int i = 0; i < sz_vec_b; i++){
					usleep(65000);
					vec_b[i] >> jobs;
				}
				vec_a.clear();
				vec_b.clear();
				vec_b_temp.clear();
				temp_vec.clear(); // memory capacity is maintained
				for(int i = 0; i < sz_vec_c; i++)
					temp_vec.push_back(vec_c[i]);
				vec_c.clear();
				*/
				// SDC algorithm finished

				t_prev = t_curr;
			}
		}
		else{
			pred = distribution(generator) % 100 < 50 ? true : false;
			if(pred){
				int poll = distribution(generator) % 100;
				int type_queue;
				if(poll > 70)
					type_queue = 0;
				else if(poll > 2)
					type_queue = 1;
				else if(poll > 1)
					type_queue = 2;
				else
					type_queue = 3;
				poll = distribution(generator) % 100;
				int req_core;
				if(poll > 90)
					req_core = 1;
				else if(poll > 80)
					req_core = 2;
				else if(poll > 10)
					req_core = 4;
				else if(poll > 5)
					req_core = 8;
				else if(poll > 2)
					req_core = 16;
				else if(poll > 1)
					req_core = 32;
				else
					req_core = 64;
				int t_req = distribution(generator) % 290 + 10;
				int t_submit = timer();
				job jb = wk.batch(type_queue, t_submit, req_core, t_req);
				temp_vec.push_back(jb);
			}
			t_curr = timer();
			int sz_temp_vec = temp_vec.size();
			int t_flush = t_curr - t_prev;
			if(t_flush > 2 && sz_temp_vec > 0){
				/******************************************
				 **** Scheduling policy should be here ****
				 ******************************************/
				// 1. FCFS
			/*	
				for(int i = 0; i < sz_temp_vec; i++){
					usleep(65000);
					temp_vec[i] >> jobs;
				}
				temp_vec.clear(); // memory capacity is maintained
			*/	
				// FCFS finished
				// 2. SDC algorithm
				
				virt_topo = topo;
				for(int i = 0; i < sz_temp_vec; i++)
					temp_vec[i].set_score((float) temp_vec[i].get_req_core() * (t_curr - temp_vec[i].get_t_submit()) / t_flush);
				std::sort(temp_vec.begin(), temp_vec.end(), [](job x, job y) -> bool{
					int t_x = x.get_score();
					int t_y = y.get_score();
					if(t_x == t_y) return x.get_req_core() > y.get_req_core();
					return t_x > t_y;
				});
				std::vector<job> vec_a;
				std::vector<job> vec_b;
				std::vector<job> vec_b_temp;
				std::vector<job> vec_c;
				for(int i = 0; i < sz_temp_vec; i++){
					int min_val = 4096;
					int min_idx = -1;
					int t_q = temp_vec[i].get_type_queue();
					for(int j = 0; j < virt_topo[t_q].size(); j++){
						if(virt_topo[t_q][j].avail_core >= temp_vec[i].get_req_core() && virt_topo[t_q][j].avail_core <= min_val){
							min_val = virt_topo[t_q][j].avail_core;
							min_idx = j;
						}
					}
					if(min_idx != -1){
						virt_topo[t_q][min_idx].avail_core -= temp_vec[i].get_req_core();
						vec_a.push_back(temp_vec[i]);
					}
					else
						vec_b_temp.push_back(temp_vec[i]);
				}
				int sz_vec_b_temp = vec_b_temp.size();
				for(int i = 0; i < sz_vec_b_temp; i++){
					int min_val = 4096;
					int min_idx_q = -1;
					int min_idx_h = -1;
					int t_q = vec_b_temp[i].get_type_queue();
					for(int j = 0; j < virt_topo.size(); j++){
						if(j == t_q)
							continue;
						for(int k = 0; k < virt_topo[j].size(); k++){
							if(virt_topo[j][k].avail_core >= vec_b_temp[i].get_req_core() && virt_topo[j][k].avail_core < min_val){
								min_val = virt_topo[j][k].avail_core;
								min_idx_q = j;
								min_idx_h = k;
							}
						}
					}
					if(min_idx_q != -1){
						virt_topo[min_idx_q][min_idx_h].avail_core -= vec_b_temp[i].get_req_core();
						vec_b_temp[i].set_type_queue(min_idx_q);
						vec_b.push_back(vec_b_temp[i]);
					}
					else
						vec_c.push_back(vec_b_temp[i]);
				}
				int sz_vec_a = vec_a.size();
				int sz_vec_b = vec_b.size();
				int sz_vec_c = vec_c.size();
				for(int i = 0; i < sz_vec_a; i++){
					usleep(65000);
					vec_a[i] >> jobs;
				}
				for(int i = 0; i < sz_vec_b; i++){
					usleep(65000);
					vec_b[i] >> jobs;
				}
				vec_a.clear();
				vec_b.clear();
				vec_b_temp.clear();
				temp_vec.clear(); // memory capacity is maintained
				for(int i = 0; i < sz_vec_c; i++)
					temp_vec.push_back(vec_c[i]);
				vec_c.clear();
				
				// SDC algorithm finished
				t_prev = t_curr;
			}
		}
	}
}

#endif

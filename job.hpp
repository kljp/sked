#ifndef __HPP_JOB__
#define __HPP_JOB__

#include <iostream>
//#include <string>
//#include <cstdlib>
//#include <ctime>
//#include <unistd.h>
#include "channel/channel.hpp"

class job{

	private:
		std::string id_job;
		int id_wk = -1;
		int type_app = -1;
		int type_queue;
		int req_core = 0;
		int t_req = 0;
		char *t_submit;
		char *t_start;
		char *t_end;
		double runtime;
		int state = -1;
		int host_alloc = -1;
	
	public:
		job(){}
		~job(){}
		job(std::string id_job, int id_wk, int type_app, int type_queue, int req_core, int t_req){
			
			this->id_job = id_job;
			this->id_wk = id_wk;
			this->type_app = type_app;
			this->type_queue = type_queue;
			this->req_core = req_core;
			this->t_req = t_req;
		}

		std::string get_id_job(){return id_job;}

		int get_id_wk(){return id_wk;}

		int get_type_app(){return type_app;}

		int get_type_queue(){return type_queue;}

		int get_req_core(){return req_core;}

		int get_host_alloc(){return host_alloc;}

		int get_t_req(){return t_req;}

		void set_host_alloc(int host_alloc){this->host_alloc = host_alloc;}

		void copy_job(job jb_src){
		
			id_job = jb_src.get_id_job();
			id_wk = jb_src.get_id_wk();
			type_app = jb_src.get_type_app();
			type_queue = jb_src.get_type_queue();
			req_core = jb_src.get_req_core();
			t_req = jb_src.get_t_req();
			host_alloc = jb_src.get_host_alloc();
		}
};

void run_job(job &jb, msd::channel<job> &jobs_fin){

	sleep(jb.get_t_req());
	jb >> jobs_fin;
	return;
}

#endif
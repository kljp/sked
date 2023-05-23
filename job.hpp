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
		int t_submit = 0;
		char *t_start;
		char *t_end;
		double runtime;
		int state = -1;
		int host_alloc = -1;
	
	public:
		job(){}
		~job(){}
		job(std::string id_job, int id_wk, int type_app, int type_queue, int t_submit, int req_core, int t_req){
			
			this->id_job = id_job;
			this->id_wk = id_wk;
			this->type_app = type_app;
			this->type_queue = type_queue;
			this->t_submit = t_submit;
			this->req_core = req_core;
			this->t_req = t_req;
		}

		std::string get_id_job(){return id_job;}

		int get_id_wk(){return id_wk;}

		int get_type_app(){return type_app;}

		int get_type_queue(){return type_queue;}

		int get_req_core(){return req_core;}

		int get_host_alloc(){return host_alloc;}

		int get_t_submit(){return t_submit;}

		int get_t_req(){return t_req;}

		void set_id_wk(int id_wk){this->id_wk = id_wk;}

		void set_type_queue(int type_queue){this->type_queue = type_queue;}

		void set_req_core(int req_core){this->req_core = req_core;}

		void set_host_alloc(int host_alloc){this->host_alloc = host_alloc;}

		void copy_job(job jb_src){
		
			id_job = jb_src.get_id_job();
			id_wk = jb_src.get_id_wk();
			type_app = jb_src.get_type_app();
			type_queue = jb_src.get_type_queue();
			t_submit = jb_src.get_t_submit();
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

void parse_log(std::string filename, msd::channel<job> *&jobs_log){

	std::ifstream file;
	file.open(filename.c_str(), std::ios::in);
	if(!file.is_open()){exit(-1);}

	std::string line;
	while(true){
	
		std::getline(file, line);
		if(line[0] != '%')
			break;
	}

	int id_jb, type_queue, t_submit, req_core, t_req;
	while(std::getline(file, line)){
	
		if(sscanf(line.c_str(), "%d %d %d %d %d", &id_jb, &type_queue, &t_submit, &req_core, &t_req) < 5){exit(-1);};
		job jb(std::to_string(id_jb), -1, 3, type_queue, t_submit, req_core, t_req);
		jb >> *jobs_log;
	}

	file.close();
	return;
}

#endif

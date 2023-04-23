#ifndef __HPP_SCHEDULER__
#define __HPP_SCHEDULER__

#include <iostream>
#include <thread>
#include <vector>
//#include <string>
//#include <cstdlib>
//#include <ctime>
//#include <unistd.h>
#include "job.hpp"
#include "topology.hpp"
#include "channel/channel.hpp"

void run_fetcher(msd::channel<job> &jobs, std::vector<msd::channel<job> *> &jobs_wait){

	while(true){

		if(jobs.size() > 0){
		
			job jb;
			jb << jobs;
			int type_queue = jb.get_type_queue();
			jb >> *jobs_wait[type_queue];
		}
	}
}

void run_dispatcher(std::vector<host> &topo, msd::channel<job> &jobs_wait){

	msd::channel<job> *jobs_fin = new msd::channel<job>();
	std::vector<std::thread> jobs;
	while(true){
	
		if(jobs_wait.size() > 0){
			// Policy: first come first serve

			job jb = jobs_wait.get_queue().front();	
			int req_core = jb.get_req_core();
			for(int i = 0; i < topo.size(); i++){
			
				if(topo[i].avail_core >= req_core){

					topo[i].avail_core -= req_core;
					job jb;
					jb << jobs_wait;
					jb.set_host_alloc(i);
					job *jb_new = new job();
					(*jb_new).copy_job(jb);
					jobs.push_back(std::thread(run_job, std::ref(*jb_new), std::ref(*jobs_fin)));
					break;
				}
			}
		}

		if((*jobs_fin).size() > 0){

			job jb;
			jb << *jobs_fin;
			topo[jb.get_host_alloc()].avail_core += jb.get_req_core();
			jb.set_host_alloc(-1);
		}
	}

	for(int i = 0; i < jobs.size(); i++)
		jobs[i].join();
	delete jobs_fin;
}

#endif

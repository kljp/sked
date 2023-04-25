#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include "topology.hpp"
#include "worker.hpp"
#include "job.hpp"
#include "scheduler.hpp"
#include "monitor.hpp"
#include "channel/channel.hpp"

int main(int argc, char **argv){

	if(argc < 3){
	
		std::cout
			<< "Required argument:\n"
			<< "\t--topo : topology of cluster (e.g., --topo cluster)\n"
			<< "Optional argument:\n"
			<< "\t--log : use actual log (e.g., --log logfile)\n"
			<< "\t--res : export results (e.g., --res result)\n"
			<< std::endl;

		exit(-1);
	}

	std::string fn_cluster = "cluster";
	std::string fn_log = "logfile";
	std::string fn_res = "result";
	bool is_checked_fn_cluster = false;
	bool is_checked_fn_log = false;
	bool is_checked_fn_res = false;

	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "--topo") && i != argc - 1){
			if(!is_checked_fn_cluster){
				fn_cluster = std::string(argv[i + 1]);
				is_checked_fn_cluster = true;
			}
		}
		else if(!strcmp(argv[i], "--log") && i != argc - 1){
			if(!is_checked_fn_log){
				fn_log = std::string(argv[i + 1]);
				is_checked_fn_log = true;
			}
		}
		else if(!strcmp(argv[i], "--res") && i != argc - 1){
			if(!is_checked_fn_res){
				fn_res = std::string(argv[i + 1]);
				is_checked_fn_res = true;
			}
		}
	}

	topology tplg(fn_cluster);
	std::vector<std::vector<host>> topo;
	for(int i = 0; i < tplg.num_queue; i++){
		std::vector<host> temp_hosts;
		topo.push_back(temp_hosts);
	}
	for(int i = 0; i < tplg.hosts.size(); i++)
		topo[tplg.hosts[i].type_queue].push_back(tplg.hosts[i]);

	msd::channel<job> *jobs_log = new msd::channel<job>();
	if(is_checked_fn_log)
		parse_log(fn_log, jobs_log);

	msd::channel<job> jobs;
	std::vector<msd::channel<job> *> jobs_wait;
	for(int i = 0; i < tplg.num_queue; i++)
		jobs_wait.push_back(new msd::channel<job>());

	std::thread fetcher = std::thread(run_fetcher, std::ref(jobs), std::ref(jobs_wait));

	std::vector<std::thread> dispatchers;
	for(int i = 0; i < tplg.num_queue; i++)
		dispatchers.push_back(std::thread(run_dispatcher, std::ref(topo[i]), std::ref(*(jobs_wait[i]))));

	int num_worker = 1;
	std::vector<std::thread> workers;
	for(int i = 0; i < num_worker; i++)
		workers.push_back(std::thread(run_worker, i, std::ref(jobs), std::ref(*jobs_log), is_checked_fn_log, tplg.num_queue));
	
	std::thread monitor = std::thread(run_monitor, std::ref(tplg), std::ref(topo), fn_res, is_checked_fn_res);
	//run_monitor(tplg);
	for(int i = 0; i < num_worker; i++)
		workers[i].join();
	if(is_checked_fn_log)
		exit(-1);
	for(int i = 0; i < tplg.num_queue; i++)
		dispatchers[i].join();
	fetcher.join();
	monitor.join();
	for(int i = 0; i < tplg.num_queue; i++)
		delete jobs_wait[i];
	delete jobs_log;

	std::cout << "[Complete] Simulation" << std::endl;

	return 0;
}

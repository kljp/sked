#include <iostream>
#include <vector>
#include <thread>
#include "topology.hpp"
#include "worker.hpp"
#include "job.hpp"
#include "scheduler.hpp"
#include "monitor.hpp"
#include "channel/channel.hpp"

int main(int argc, char **argv){

	std::string filename = "host_setup";
	topology tplg(filename);
	std::vector<std::vector<host>> topo;
	for(int i = 0; i < tplg.num_queue; i++){
	
		std::vector<host> temp_hosts;
		topo.push_back(temp_hosts);
	}
	for(int i = 0; i < tplg.hosts.size(); i++)
		topo[tplg.hosts[i].type_queue].push_back(tplg.hosts[i]);

	msd::channel<job> jobs;
	std::vector<msd::channel<job> *> jobs_wait;
	for(int i = 0; i < tplg.num_queue; i++)
		jobs_wait.push_back(new msd::channel<job>());

	std::thread fetcher = std::thread(run_fetcher, std::ref(jobs), std::ref(jobs_wait));

	std::vector<std::thread> dispatchers;
	for(int i = 0; i < tplg.num_queue; i++)
		dispatchers.push_back(std::thread(run_dispatcher, std::ref(topo[i]), std::ref(*(jobs_wait[i]))));

	int num_worker = 16;
	std::vector<std::thread> workers;
	for(int i = 0; i < num_worker; i++)
		workers.push_back(std::thread(run_worker, i, std::ref(jobs), tplg.num_queue));
	
	std::thread monitor = std::thread(run_monitor, std::ref(tplg), std::ref(topo));
	//run_monitor(tplg);
	for(int i = 0; i < num_worker; i++)
		workers[i].join();
	for(int i = 0; i < tplg.num_queue; i++)
		dispatchers[i].join();
	fetcher.join();
	monitor.join();
	for(int i = 0; i < tplg.num_queue; i++)
		delete jobs_wait[i];

	std::cout << "[Complete] Simulation" << std::endl;

	return 0;
}

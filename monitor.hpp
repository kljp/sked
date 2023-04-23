#ifndef __HPP_MONITOR__
#define __HPP_MONITOR__

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <algorithm>
#include "topology.hpp"

void run_monitor(topology tplg, std::vector<std::vector<host>> &topo){
	
	int num_host = tplg.num_host;
	int num_core = tplg.num_core;
	std::vector<int> tot_core(num_host);
	std::vector<int> util_core(num_host);
	for(int i = 0; i < tplg.hosts.size(); i++)
		tot_core[tplg.hosts[i].type_host] += tplg.hosts[i].max_core;
	
	int iteration = 0;	
	while(true){

		sleep(3);
		int ovr_core = 0;
		std::string printer = "";
		std::fill(util_core.begin(), util_core.end(), 0);
		for(int i = 0; i < topo.size(); i++){
			for(int j = 0; j < topo[i].size(); j++)
				util_core[topo[i][j].type_host] += (topo[i][j].max_core - topo[i][j].avail_core);
		}
		printer += "===========[ Iteration ";
		if(iteration < 10)
			printer += "    ";
		else if(iteration < 100)
			printer += "   ";
		else if(iteration < 1000)
			printer += "  ";
		else if(iteration < 10000)
			printer += " ";
		printer += std::to_string(iteration);
		printer += " ]=================================================================================================\n";
		printer += "Host:   ";
		for(int i = 0; i < num_host; i++)
			printer += ("    " + std::to_string(i) + "        ");
		printer += "\nUtil:   ";
		for(int i = 0; i < num_host; i++){
			float util_per = (float) util_core[i] / tot_core[i];
			if(util_per < 10.0)
				printer += ("  " + std::to_string(util_per));
			else if(util_per < 100.0)
				printer += (" " + std::to_string(util_per));
			else
				printer += std::to_string(util_per);
			printer += "   ";
			ovr_core += util_core[i];
		}
		printer += "\nOverall utilization: ";	
		float util_ovr_per = (float) ovr_core / num_core;
		if(util_ovr_per < 10.0)
			printer += ("  " + std::to_string(util_ovr_per));
		else if(util_ovr_per < 100.0)
			printer += (" " + std::to_string(util_ovr_per));
		else
			printer += std::to_string(util_ovr_per);
		printer += "   \n";
		std::cout << printer;
		iteration++;
	}
}

#endif

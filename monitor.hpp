#ifndef __HPP_MONITOR__
#define __HPP_MONITOR__

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include "topology.hpp"

void run_monitor(topology tplg, std::vector<std::vector<host>> &topo, std::string fn_res, bool flag_fout){
	
	int num_host = tplg.num_host;
	int num_queue = tplg.num_queue;
	int num_core = tplg.num_core;
	std::vector<int> tot_core(num_host);
	std::vector<int> util_core(num_host);
	std::vector<int> tot_queue(num_queue);
	std::vector<int> util_queue(num_queue);
	for(int i = 0; i < tplg.hosts.size(); i++){
		tot_core[tplg.hosts[i].type_host] += tplg.hosts[i].max_core;
		tot_queue[tplg.hosts[i].type_queue] += tplg.hosts[i].max_core;
	}

	std::ofstream res_out;
	if(flag_fout){
		res_out.open(fn_res.c_str(), std::ios::out);
		if(!res_out.is_open()){exit(-1);}
	}
	
	int iteration = 0;	
	while(true){

		sleep(3);
		int ovr_core = 0;
		std::string printer = "";
		std::fill(util_core.begin(), util_core.end(), 0);
		std::fill(util_queue.begin(), util_queue.end(), 0);
		for(int i = 0; i < topo.size(); i++){
			for(int j = 0; j < topo[i].size(); j++){
				int curr_core = (topo[i][j].max_core - topo[i][j].avail_core);
				util_core[topo[i][j].type_host] += curr_core;
				util_queue[topo[i][j].type_queue] += curr_core;
			}
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
		printer += "\nH.Util:  ";
		for(int i = 0; i < num_host; i++){
			float util_per = (float) util_core[i] / tot_core[i];
			printer += ("  " + std::to_string(util_per));
			printer += "   ";
			ovr_core += util_core[i];
		}
		printer += "\nQueue:  ";
		for(int i = 0; i < num_queue; i++)
			printer += ("    " + std::to_string(i) + "        ");
		printer += "\nQ.Util:  ";
		for(int i = 0; i < num_queue; i++){
			float util_per = (float) util_queue[i] / tot_queue[i];
			printer += ("  " + std::to_string(util_per));
			printer += "   ";
		}
		printer += "\nOvr.Util: ";	
		float util_ovr_per = (float) ovr_core / num_core;
		printer += (" " + std::to_string(util_ovr_per));
		printer += "   \n";
		std::cout << printer;
		if(flag_fout)
			res_out << printer << std::endl;
		iteration++;

		if(iteration > 2640)
			break;
	}
	if(flag_fout)
		res_out.close();
}

#endif

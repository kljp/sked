#ifndef __HPP_TOPOLOGY__
#define __HPP_TOPOLOGY__

#include <iostream>
#include <fstream>
#include <string>
//#include <cstdlib>
#include <cstdio>
#include <vector>

class host{

	public:
		int type_host;
		int type_queue;
		int max_core;
		int avail_core;

		host(){}
		~host(){}
		host(int type_host, int type_queue, int core){
		
			this->type_host = type_host;
			this->type_queue = type_queue;
			max_core = core;
			avail_core = core;
		}
};

class topology{
	
	public:
		std::vector<host> hosts;
		int num_host;
		int num_queue;
		int num_core;

		topology(){}
		~topology(){}
		topology(std::string filename){
		
			std::ifstream file;
			file.open(filename.c_str(), std::ios::in);
			if(!file.is_open()){exit(-1);}

			std::string line;
			while(true){
			
				std::getline(file, line);
				if(line[0] != '%')
					break;
			}
			if(sscanf(line.c_str(), "%d %d %d", &num_host, &num_queue, &num_core) < 3){exit(-1);};

			int type_host, type_queue, core;
			while(std::getline(file, line)){
			
				if(sscanf(line.c_str(), "%d %d %d", &type_host, &type_queue, &core) < 3){exit(-1);};
				hosts.push_back(host(type_host, type_queue, core));
			}

			file.close();
		}
};

#endif

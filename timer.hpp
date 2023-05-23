#ifndef __HPP_TIMER__
#define __HPP_TIMER__

#include <sys/time.h>
#include <stdlib.h>

inline int timer(){

	int time;
	struct timeval time1;
	gettimeofday(&time1, NULL);
	time = (int) time1.tv_sec;

	return time;
}

#endif

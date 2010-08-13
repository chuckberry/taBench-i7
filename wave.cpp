/*
 * wave.cpp
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#include "wave.h"

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/prctl.h>

void wave::kickoff()
{
    int err;

    err = sem_init(&sem_tid, 0, 0);
    if (err != 0) {
        perror("wave::sem_tid");
    }

    pthread_t thr;
    err = pthread_create(&thr, NULL, start_routine, (void *)this);
    if (err != 0) {
        perror("wave::kickoff");
    }
}

pid_t wave::get_tid()
{
    /* can be called only once, otherwise it will block forever */
    int err = sem_wait(&sem_tid);
    if (err != 0 )
        perror("wave::sem_wait = sem_tid");
    if (!tid)
        fprintf(stderr, "error synchronizing to get tid.\n");

    return tid;
}

void *wave::start_routine(void *_this)
{
	char name[10];
	wave *obj = (wave *)_this;

	obj->tid = gettid();

	int err = sem_post(&obj->sem_tid);
	if (err != 0)
		perror("wave::sem_post - sem_tid");

	sprintf (name, "wave%d", obj->index);
	prctl(PR_SET_NAME, name, 0,0,0);

#ifdef CPUAFFINITY
	cpu_set_t cpuset;
        pthread_t thread;
        int r;

	thread = pthread_self();
        CPU_ZERO(&cpuset);
# ifdef OPTIM_AFFINITY
        if(obj->index == 0){
                CPU_SET(0, &cpuset);
        }
        
        if(obj->index == 1){
                CPU_SET(1, &cpuset);
        }
        
        if(obj->index == 2){
                CPU_SET(2, &cpuset);
        }
        
        if(obj->index == 3){
                CPU_SET(3, &cpuset);
        }
# endif
# ifdef WORST_AFFINITY
	
        if(obj->index == 0){
                CPU_SET(0, &cpuset);
        }
        
        if(obj->index == 1){
                CPU_SET(3, &cpuset);
        }
        
        if(obj->index == 2){
                CPU_SET(1, &cpuset);
        }
        
        if(obj->index == 3){
                CPU_SET(2, &cpuset);
        }
# endif
 
        if((r = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)) != 0){
                errno=r;
                perror("pthread_setaffinity_np");
                exit(r);
        }
       /* fprintf(stderr, "Wave %d is currently running on cpu #%d\n", obj->index, sched_getcpu());*/

#endif /* CPUAFFINITY */
	obj->loop();
	return NULL;
}

void wave::loop()
{
    t_sint16 sample = 0;

    bzero(buffer, BUFLENBYTE);

    while (1) {
        for (int i = 0; i < (BUFLENWORD-1); i += 2) {
            buffer[i] = sample;
            buffer[i+1] = sample;

            // fprintf(stderr, "+");

            if ((i % width) == 0)
                sample += height;
        }

        // write(1, buffer, BUFLENBYTE);

        my_mixer.emptyBuffer(buffer, pos);
    }
}

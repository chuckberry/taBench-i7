/*
 * mixer.cpp
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#include "mixer.h"

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/prctl.h>

mixer::mixer(int _index) : index(_index), tid(0)
{
    pthread_t thr;
    int err;

    inbuf0 = NULL;
    inbuf1 = NULL;

    pthread_mutex_init(&mutex, NULL);
    err = sem_init(&sema0, 0, 0);
    if (err != 0) {
        perror("mixer::sem_init - 0");
    }
    err = sem_init(&sema1, 0, 0);
    if (err != 0) {
        perror("mixer::sem_init - 1");
    }
    err = sem_init(&notifier, 0, 0);
    if (err != 0) {
        perror("mixer::sem_init - notifier");
    }

    err = sem_init(&sem_tid, 0, 0);
    if (err != 0) {
        perror("mixer::sem_tid");
    }

    err = sem_init(&sem_start, 0, 0);
    if (err != 0) {
        perror("mixer::sem_start - 0");
    }

    err = pthread_create(&thr, NULL, start_routine, (void *)this);
    if (err != 0) {
        perror("mixer::kickoff");
    }
}

pid_t mixer::get_tid()
{
    /* can be called only once, otherwise it will block forever */
    int err = sem_wait(&sem_tid);
    if (err != 0 )
        perror("mixer::sem_wait = sem_tid");
    if (!tid)
        fprintf(stderr, "error synchronizing to get tid.\n");

    return tid;
}

void *mixer::start_routine(void *_this)
{

	char name[10];
	mixer *obj = (mixer *)_this;

	obj->tid = gettid();

	int err = sem_post(&obj->sem_tid);
	if (err != 0)
		perror("mixer::sem_post - sem_tid");

	sprintf (name, "mixer%d", obj->index);
	prctl(PR_SET_NAME, name, 0,0,0);
#ifdef CPUAFFINITY
	cpu_set_t cpuset;
	pthread_t thread;
	int r;
	
	thread = pthread_self();
        CPU_ZERO(&cpuset);
# ifdef OPTIM_AFFINITY
        if(obj->index == 0){
                /* m0 on cpu0 */
                CPU_SET(0, &cpuset);
        }

        if(obj->index == 1){
                /* m1 on cpu2 */
                CPU_SET(2, &cpuset);
        }

        if(obj->index == 2){
                /* m2 on cpu2 */
                CPU_SET(2, &cpuset);
        }
# endif

# ifdef WORST_AFFINITY
        if(obj->index == 0){
                /* m0 on cpu1 (w0 -> 0 w1 -> 3*/
                CPU_SET(2, &cpuset);
        }

        if(obj->index == 1){
                /* m1 on cpu3 (w2 -> 1 w3 -> 2 */
                CPU_SET(3, &cpuset);
        }

        if(obj->index == 2){
                /* m2 on cpu2 */
                CPU_SET(1, &cpuset);
        }
# endif

        if((r = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)) != 0){
                errno=r;
                perror("pthread_setaffinity_np");
                exit(r);
        }
	
	/* fprintf(stderr, "Mixer %d is currently running on cpu #%d\n", obj->index, sched_getcpu()); */
#endif
	obj->loop();
	return NULL;
}

void mixer::start(pid_t t0, pid_t t1)
{
    dep0 = t0;
    dep1 = t1;

    int err = sem_post(&sem_start);
    if (err != 0)
        perror("mixer::start - sem_post");
}

void mixer::loop()
{
    t_sint16 sample;
    int err;

    sem_wait(&sem_start);

#ifdef TASKAFFINITY
    sched_add_taskaffinity(dep0);
    sched_add_taskaffinity(dep1);
#endif

    while (1) {
        sem_wait(&notifier);

        for (int i = 0; i < (BUFLENWORD-1); i += 2) {
            // if(inbuf0 == inbuf1) fprintf(stderr, "!");
            sample = (inbuf0[i]/2) + (inbuf1[i]/2);
            buffer[i] = sample;

            sample = (inbuf0[i+1]/2) + (inbuf1[i+1]/2);
            buffer[i+1] = sample;
        }

        inbuf0 = NULL;
        inbuf1 = NULL;

        err = sem_post(&sema0);
        if (err != 0) {
            perror("mixer::sem_post - 0");
        }
        err = sem_post(&sema1);
        if (err != 0) {
            perror("mixer::sem_post - 1");
        }

        output();
    }
}

void mixer::emptyBuffer(t_sint16 *buf, int pos)
{
    int err;

    err = pthread_mutex_lock(&mutex);
    if (err != 0) {
        perror("mixer::pthread_mutex_lock");
    }

    if (!pos) {
        inbuf0 = buf;
    } else {
        inbuf1 = buf;
    }

    if (inbuf0 && inbuf1) {
        err = sem_post(&notifier);
        if (err != 0) {
            perror("mixer::sem_post - notifier");
        }
    }

    err = pthread_mutex_unlock(&mutex);
    if (err != 0) {
        perror("pthread_mutex_unlock");
    }

    if (!pos) {
        sem_wait(&sema0);
    } else {
        sem_wait(&sema1);
    }
}

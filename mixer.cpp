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


mixer::mixer(int _index) : index(_index)
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

    err = pthread_create(&thr, NULL, start_routine, (void *)this);
    if (err != 0) {
        perror("mixer::kickoff");
    }
}

void *mixer::start_routine(void *_this)
{
    char name[10];
    mixer *obj = (mixer *)_this;
    sprintf (name, "wave%d", obj->index);
    prctl(PR_SET_NAME, name, 0,0,0);

    fprintf(stderr, "mixer%d %ld\n", obj->index, gettid());
    obj->loop();
    return NULL;
}

void mixer::loop()
{
    t_sint16 sample;
    int err;

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

    if (pos) {
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

    if (pos) {
        sem_wait(&sema0);
    } else {
        sem_wait(&sema1);
    }
}

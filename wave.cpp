/*
 * wave.cpp
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#include "wave.h"

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>
#include <sys/prctl.h>

void wave::kickoff()
{
    pthread_t thr;
    int err = pthread_create(&thr, NULL, start_routine, (void *)this);
    if (err != 0) {
        perror("wave::kickoff");
    }
}

void *wave::start_routine(void *_this)
{
    char name[10];
    wave *obj = (wave *)_this;

    sprintf (name, "wave%d", obj->index);
    prctl(PR_SET_NAME, name, 0,0,0);
    fprintf(stderr,"wave%d %ld\n", obj->index, gettid());
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

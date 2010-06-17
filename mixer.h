/*
 * mixer.h
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#ifndef MIXER_H_
#define MIXER_H_

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "const.h" // BUFLENWORD, BUFLENBYTE
#include "type.h" // t_sint16

class mixer
{
private:
    t_sint16 *inbuf0;
    t_sint16 *inbuf1;

    pthread_mutex_t mutex;
    sem_t sema0;
    sem_t sema1;
    sem_t notifier;
    sem_t sem_tid;
    int index;
    pid_t tid;

    void loop();
    virtual void output()=0;
    static void *start_routine(void *_this);

protected:
    t_sint16 buffer[BUFLENWORD];

public:
    mixer(int _index);
    pid_t get_tid();
    void emptyBuffer(t_sint16 *buf, int pos);
};

#endif /* MIXER_H_ */

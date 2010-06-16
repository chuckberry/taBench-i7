/*
 * mixer.h
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#ifndef MIXER_H_
#define MIXER_H_

#include "../components/const.h" // BUFLENWORD, BUFLENBYTE
#include "share/inc/type.h" // t_sint16

#include <pthread.h>
#include <semaphore.h>

class mixer {
 private:
	 t_sint16 *inbuf0;
	 t_sint16 *inbuf1;

	 pthread_mutex_t mutex;
	 sem_t sema0;
	 sem_t sema1;
	 sem_t notifier;

	 void loop();
	 virtual void output()=0;

	 static void *start_routine(void *_this);

 protected:
	 t_sint16 buffer[BUFLENWORD];

 public:
	 mixer();
	 void emptyBuffer(t_sint16 *buf, int pos);
};

#endif /* MIXER_H_ */

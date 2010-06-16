/*
 * mixer2sink.h
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#ifndef MIXER2SINK_H_
#define MIXER2SINK_H_

#include <stdio.h>

#include "mixer.h"

class mixer2sink: public mixer {
public:
	mixer2sink() {};

	virtual void output() {
		int written = write(1, buffer, BUFLENBYTE);
		if(written != BUFLENBYTE) {
			fprintf(stderr, "Wrote %i bytes\n", written);
		}
	}
};

#endif /* MIXER2SINK_H_ */

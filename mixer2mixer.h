/*
 * mixer2mixer.h
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#ifndef MIXER2MIXER_H_
#define MIXER2MIXER_H_

#include "mixer.h"

class mixer2mixer: public mixer {
private:
	mixer &out_mixer;
	int pos;

public:
	mixer2mixer(mixer &_out, int _pos): out_mixer(_out), pos(_pos) {};

	virtual void output() {
		out_mixer.emptyBuffer(buffer, pos);
	}
};

#endif /* MIXER2MIXER_H_ */

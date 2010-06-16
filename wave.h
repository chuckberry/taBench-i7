/*
 * wave.h
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#ifndef WAVE_H_
#define WAVE_H_

#include "const.h" // BUFLENWORD, BUFLENBYTE
#include "type.h" // t_sint16

#include "mixer.h"

class wave {
 private:
  t_sint16 buffer[BUFLENWORD];
  mixer &my_mixer;
  int pos;
  t_uint32 width;
  t_uint32 height;

  void kickoff();
  void loop();

  static void *start_routine(void *_this);
  int index;
 public:
  wave(int _index, mixer &_mixer, int _pos, t_uint32 _width, t_uint32 _height):
	  my_mixer(_mixer), pos(_pos),
	  width(_width), height(_height), index(_index) {
		  kickoff();
	}
};

#endif /* WAVE_H_ */

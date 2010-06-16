/*
 * nwBench.cpp
 *
 *  Created on: Mar 10, 2009
 *      Author: betzwlin
 */

#include <unistd.h>
#include <sys/mman.h>

#include "wave.h"
#include "mixer2mixer.h"
#include "mixer2sink.h"

static void bench_main() {
  mixer2sink sink;
  mixer2mixer mixer0(sink, 0);
  mixer2mixer mixer1(sink, 1);
  wave wave0(mixer0, 0, 2, 256);
  wave wave1(mixer0, 1, 5, 256);
  wave wave2(mixer1, 0, 3, 256);
  wave wave3(mixer1, 1, 8, 256);

  while(1) {
    pause(); // sleep forever ...
  }
}

int main(int argc , char **argv) {
  int st = mlockall(MCL_CURRENT | MCL_FUTURE);
  if(st != 0) perror("mlockall");

  bench_main();
}

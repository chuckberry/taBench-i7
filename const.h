#ifndef __CONST_IDT
#define __CONST_IDT

#define BUFLENWORD (2*1) /* betzw: must be a multiple of 2 because of stereo PCM audio data */
#define BUFLENBYTE (2*BUFLENWORD)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define gettid() syscall(SYS_gettid)

#endif // __CONST_IDT

#ifndef __CONST_IDT
#define __CONST_IDT

#define BUFLENWORD (2*1) /* betzw: must be a multiple of 2 because of stereo PCM audio data */
#define BUFLENBYTE (2*BUFLENWORD)

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define gettid() syscall(SYS_gettid)

#ifdef __x86_64__
# define __NR_sched_add_taskaffinity 299
# define sched_add_taskaffinity(tsk) \
       syscall(__NR_sched_add_taskaffinity, tsk)

# define __NR_sched_del_taskaffinity 300
# define sched_del_taskaffinity(tsk) \
       syscall(__NR_sched_del_taskaffinity, tsk)
#elif __i386__
# define __NR_sched_add_taskaffinity 337
# define sched_add_taskaffinity(tsk) \
	syscall(__NR_sched_add_taskaffinity, tsk)

# define __NR_sched_del_taskaffinity 338
# define sched_del_taskaffinity(tsk) \
	syscall(__NR_sched_del_taskaffinity, tsk)
#endif

#endif // __CONST_IDT

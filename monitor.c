/*
 * monitor.c
 *
 *	Simple program to perform real-time performance
 *	monitoring of the NMF Host Execution Engine Benchmark
 *
 *  Created on: Mar 6, 2009
 *      Author: betzwlin
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>		/* fork(), kill() */
#include <sys/wait.h>		/* waitpid() */
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>		/* kill() */
#include <time.h>
#include <sys/mman.h>
#include <math.h>
#include <errno.h>		/* perror() */
#include <sched.h>		/* sched_yield */
#include <sys/syscall.h>
#include <fcntl.h>

#include "const.h"		// BUFLENWORD, BUFLENBYTE
#include "type.h"		// t_sint16

#define FILE_NAME_BASE "bench_"
#define FILE_NAME_TRACE "_trace"
#define FILE_NAME_HIST "_histg"

#ifdef CONFIG_LONG_RUN
# define NR_SAMPLES 150
#else
# define NR_SAMPLES 150
#endif

#define US_SLEEP 250

#define RT_DATA_RATE_BYTE       32	// in bytes
#define RT_DATA_RATE_MS          1	// in millisecs

// in bytes / millisecs
#define RT_DATA_RATE \
	((double)RT_DATA_RATE_BYTE / (double)RT_DATA_RATE_MS)

#define USEC_PER_MSEC		1000
#define USEC_PER_SEC		1000000

#define HIST_BINS 4000
#define XRANGE_MARGIN 10

#define CMD_NAME_SIZE 100

static FILE *fout_trace;
static FILE *fout_hist;

static struct timespec wake_time[NR_SAMPLES];
static long wake_time_hist[HIST_BINS];


#if defined CONFIG_USE_TRACING 
# ifndef CONFIG_KDEBUG_MOUNT
#  define CONFIG_KDEBUG_MOUNT "/sys/kernel/debug/"
# endif
# define TRACING_PATH CONFIG_KDEBUG_MOUNT "tracing/"
# ifndef CONFIG_FTRACE_MEM
#  define CONFIG_FTRACE_MEM "100000"
# endif
# ifdef CONFIG_USE_FTRACE
#  ifndef CONFIG_TRACING_FUNCTIONS
#   define CONFIG_TRACING_FUNCTIONS ""	\
	 "try_to_wake_up "
#  endif
#  ifndef CONFIG_TRACING_FILTERS
#   define CONFIG_TRACING_FILTERS ""	\
	 "task_woken_rt " 		\
	 "try_to_wake_up " 		\
	 "select_task_rq_rt "
#  endif
# endif /* CONFIG_USE_FTRACE */
# define FTRACE_OPEN(FD,ATTR)				\
	fprintf(stderr, "Opening file "			\
			TRACING_PATH ATTR		\
			"...\n");			\
	FD = open(TRACING_PATH ATTR, O_WRONLY|O_SYNC);	\
	if(FD < 0){					\
		perror(ATTR);				\
		return -1;				\
	}
# define FTRACE_CONF(FD,VALUE)				\
	if (write(FD,VALUE,strlen(VALUE)) == -1) {	\
		perror("ftrace config: ");		\
		return -1;				\
	}

# define FTRACE_CLOSE(FD,ATTR)				\
	fprintf(stderr, "Closing file "			\
			TRACING_PATH ATTR		\
			"...\n");			\
	close(FD);	

int fd_tracing_enabled = 0;
int fd_tracing_current = 0;
int fd_tracing_buffersize = 0;
//int fd_funcgraph_printk = 0;

# ifdef CONFIG_USE_FTRACE
int fd_set_graph_function = 0;
int fd_set_ftrace_filter = 0;
int fd_funcgraph_cpu = 0;
int fd_funcgraph_proc = 0;
int fd_funcgraph_abstime = 0;
int fd_funcgraph_duration = 0;
# endif

int ftrace_setup(void) {

	fprintf(stderr, "Configuring Ftrace...\n");
	FTRACE_OPEN(fd_tracing_enabled,"tracing_enabled");
	FTRACE_OPEN(fd_tracing_current,"current_tracer");
	FTRACE_OPEN(fd_tracing_buffersize,"buffer_size_kb");
	FTRACE_CONF(fd_tracing_enabled, "0");
	FTRACE_CONF(fd_tracing_current, "nop\n");
	FTRACE_CONF(fd_tracing_buffersize, CONFIG_FTRACE_MEM);
// 	FTRACE_OPEN(fd_funcgraph_printk,"options/trace_printk");

# ifdef CONFIG_USE_FTRACE
	FTRACE_OPEN(fd_set_graph_function,"set_graph_function");
	FTRACE_OPEN(fd_set_ftrace_filter,"set_ftrace_filter");
//	FTRACE_CONF(fd_set_graph_function, CONFIG_TRACING_FUNCTIONS);
//	FTRACE_CONF(fd_set_graph_function,"find_lowest_rq\n");
//	FTRACE_CONF(fd_set_graph_function,"check_preempt_curr_rt\n");
	FTRACE_CONF(fd_set_graph_function, "cpupri_find\n");
//	FTRACE_CONF(fd_set_graph_function, "build_taskaff_mask\n");
//	FTRACE_CONF(fd_set_graph_function, "cpupri_taskaff_find\n");
//	FTRACE_CONF(fd_set_ftrace_filter, "try_to_wake_up\n");
//	FTRACE_CONF(fd_set_ftrace_filter, "push_rt_task\n");
//	FTRACE_CONF(fd_set_ftrace_filter, "find_lowest_rq\n"); 
//	FTRACE_CONF(fd_set_ftrace_filter, "task_woken_rt\n"); 
//	FTRACE_CONF(fd_set_ftrace_filter, "select_task_rq_rt\n"); 
//	FTRACE_CONF(fd_set_ftrace_filter, "build_taskaff_mask\n");
//	FTRACE_CONF(fd_set_ftrace_filter, "cpupri_taskaff_find\n");
	FTRACE_CONF(fd_set_ftrace_filter, "cpupri_find\n");
//	FTRACE_CONF(fd_set_ftrace_filter, "check_preempt_curr_rt\n");
# endif

	return 0;
}

int ftrace_start(void) {

# ifdef CONFIG_USE_FTRACE
	FTRACE_CONF(fd_tracing_current, "function_graph");
 	FTRACE_OPEN(fd_funcgraph_cpu,"options/funcgraph-cpu");
 	FTRACE_OPEN(fd_funcgraph_proc,"options/funcgraph-proc");
 	FTRACE_OPEN(fd_funcgraph_abstime,"options/funcgraph-abstime");
 	FTRACE_OPEN(fd_funcgraph_duration,"options/funcgraph-duration");

	FTRACE_CONF(fd_funcgraph_cpu, "1");
	FTRACE_CONF(fd_funcgraph_proc, "1");
	FTRACE_CONF(fd_funcgraph_abstime, "1");
	FTRACE_CONF(fd_funcgraph_duration, "1");
//	FTRACE_CONF(fd_funcgraph_printk, "1");
# endif

# ifdef CONFIG_USE_SCHED_SWITCH
//	FTRACE_CONF(fd_funcgraph_printk, "0");
	FTRACE_CONF(fd_tracing_current, "sched_switch");
# endif
	FTRACE_CONF(fd_tracing_enabled, "1");
	return 0;
}

int ftrace_stop(void) {
	FTRACE_CONF(fd_tracing_enabled, "0");
# ifdef CONFIG_USE_FTRACE
//	FTRACE_CONF(fd_funcgraph_printk, "0");
# endif
	return 0;
}

int ftrace_close(void) {
	FTRACE_CLOSE(fd_tracing_buffersize,"buffer_size_kb");
	FTRACE_CLOSE(fd_tracing_enabled,"tracing_enabled");
	FTRACE_CLOSE(fd_tracing_current,"current_tracer");
# ifdef CONFIG_USE_FTRACE
	FTRACE_CLOSE(fd_set_graph_function,"set_graph_function");
	FTRACE_CLOSE(fd_set_ftrace_filter,"set_ftrace_filter");
	FTRACE_CLOSE(fd_funcgraph_cpu,"options/funcgraph-cpu");
	FTRACE_CLOSE(fd_funcgraph_proc,"options/funcgraph-proc");
	FTRACE_CLOSE(fd_funcgraph_abstime,"options/funcgraph-abstime");
	FTRACE_CLOSE(fd_funcgraph_duration,"options/funcgraph-duration");
// 	FTRACE_CLOSE(fd_funcgraph_printk,"options/trace_printk");
# endif
	return 0;
}

#else
# define ftrace_setup() 0
# define ftrace_start() 0
# define ftrace_stop() 0
# define ftrace_close() 0
#endif

/*Returns the difference between b and a in nanoseconds, taking overflow in account*/
static long sat_ndiff(const struct timespec *a, const struct timespec *b)
{
    long long ret;

    ret = (a->tv_sec - b->tv_sec) * 1000000000LL
	+ (a->tv_nsec - b->tv_nsec);

    if (ret > (long long) LONG_MAX) {
	fprintf(stderr, "WARNING: Long long overflow\n");
	return LONG_MAX;
    }

    return ret;
}

static int check_timer(void)
{
    struct timespec ts;

    if (clock_getres(CLOCK_MONOTONIC, &ts))
	return 1;

    return (ts.tv_sec != 0 || ts.tv_nsec != 1);
}

static void timed_memset(FILE * f, const char *when, clockid_t clk_id,
			 void *p, size_t ps)
{
    struct timespec b_memset, a_memset;

    clock_gettime(clk_id, &b_memset);
    memset(p, 0, ps);
    clock_gettime(clk_id, &a_memset);

    fprintf(f, "# memset %u bytes at %8p %s in %ldus\n",
	    ps, p, when, sat_ndiff(&a_memset, &b_memset));
}


#define STACK_SIZE 65536	/*64KByte */

static void lock_all(FILE * f, clockid_t clk_id)
{
    char stack[STACK_SIZE];
    struct timespec b_mlock, a_mlock;
    int st;

    /* Allocate and touch some space on the stack; allocate so that the
       pages are mapped, touch to ensure that any copy-on-write takes
       place beforehand.
     */
    timed_memset(f, "before mlockall", clk_id, stack, sizeof(stack));

    fprintf(f, "# mlockall ");

    clock_gettime(clk_id, &b_mlock);
    st = mlockall(MCL_CURRENT | MCL_FUTURE);
    clock_gettime(clk_id, &a_mlock);

    if (st != 0)
	perror("mlockall");
    fprintf(f, "%s in %ldus\n",
	    st ? "*** failed ***" : "ok", sat_ndiff(&a_mlock, &b_mlock));

    /* For timing comparison */
    timed_memset(f, "after mlockall", clk_id, stack, sizeof(stack));
}

static t_sint16 inbuf[BUFLENWORD];

static int monitor()
{
    int i;

    if (ftrace_start()) {
	    fprintf(stderr, "ERROR: ftrace startup failed\n");
	    return -1;
    }

    for (i = 0; i < NR_SAMPLES; i++) {
	int n = read(0, inbuf, BUFLENBYTE);
	if (n != BUFLENBYTE) {
	    fprintf(stderr, "WARNING: Read %i bytes\n", n);
	}
	clock_gettime(CLOCK_MONOTONIC, &wake_time[i]);
    }

    ftrace_stop();

    return 0;
}


struct interval_stat {
    double mean_us;
    double s2;
    long min, min_pos;
    long max, max_pos;
    long bin_granularity;
    long xrange;
};

static inline void
calc_interval_stats(const struct timespec data[], int n_samples,
		    long hist[HIST_BINS], struct interval_stat *is)
{
    int i;
    double s = 0.0, ssq = 0.0;
    long min = LONG_MAX, max = 0;
    long min_pos = 0, max_pos = 0;
    double n_int = (double) n_samples;
    long d, range, tmp;
    int bin;
    long xrange = 0;

    for (i = 1; i < n_samples; i++) {
	d = sat_ndiff(&(data[i]), &(data[i - 1]));
	if (d > max) {
	    max = d;
	    max_pos = i;
	}
	if (d < min) {
	    min = d;
	    min_pos = i;
	}

	s += (double) d;
	ssq += (double) d *(double) d;
    }

    if (n_int)
	is->mean_us = s / n_int;
    if (n_int > 1)
	is->s2 = (ssq - n_int * (is->mean_us * is->mean_us)) / (n_int - 1);
    is->min = min;
    is->min_pos = min_pos;
    is->max = max;
    is->max_pos = max_pos;

    tmp = abs(is->min - is->mean_us);
    range = abs(is->max - is->mean_us);
    range = (range > tmp) ? range : tmp;
    if (range <= HIST_BINS / 2) {
	is->bin_granularity = 1;
    } else {
	is->bin_granularity =
	    ceill((double) range / (double) (HIST_BINS / 2));
    }

    for (i = 1; i < n_samples; i++) {
	d = sat_ndiff(&(data[i]), &(data[i - 1]));
	bin =
	    (int)
	    floor((((double) d -
		    is->mean_us) / (double) is->bin_granularity) + 0.5) +
	    HIST_BINS / 2;
	if (bin < 0)
	    bin = 0;
	else if (bin >= HIST_BINS)
	    bin = HIST_BINS - 1;

	hist[bin]++;
    }

    for (i = 0; i < HIST_BINS; i++) {
	if (hist[i] != 0) {
	    unsigned long dist = abs(i - (HIST_BINS / 2));
	    if (dist > xrange) {
		xrange = dist;
	    }
	}
    }
    xrange *= is->bin_granularity;
    xrange = (xrange / XRANGE_MARGIN) + 1;
    is->xrange = xrange * XRANGE_MARGIN;
}

static inline void
dump_interval_stat(FILE * f, const char *comment,
		   const struct interval_stat *is, const long data[])
{
    int i;

    fprintf(f,
	    "# %s: xrange = %ld\n#\tmin/max=%ld@[%ld]/%ld@[%ld]us, mean=%gus, s^2=%g\n",
	    comment, is->xrange, is->min, is->min_pos, is->max,
	    is->max_pos, is->mean_us, is->s2);
    fprintf(f, "# bin granularity: %ld\n", is->bin_granularity);

    if (data) {
	for (i = 0; i < HIST_BINS; i++) {
	    fprintf(f, "%10ld\n", data[i]);
	}
    }
}


#define BUF_SIZE 512

static void dump_cpuinfo(FILE * f)
{
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    char buf[BUF_SIZE];

    while (fgets(buf, BUF_SIZE, cpuinfo) != NULL) {
	fprintf(f, "# %s", buf);
    }

    fclose(cpuinfo);
}

static void dump_testinfo(FILE * f)
{
    fprintf(f, "# Buffer length (in bytes): %d\n", BUFLENBYTE);

    /* Dump sampling parameters as gnuplot comments */
    fprintf(f, "# Collected %d samples\n", NR_SAMPLES);
}

static struct interval_stat is;

static void do_stat()
{
    int i;

    /* Dump identification, both new (Aegis) and old (RCS) */
    /* fprintf(worker_arg->f, "# Produced by %s.%s, built on %s\n",
       PROJECT, VERSION, BUILD_DATE); */
    fprintf(fout_trace,
	    "# Code version: monitor" CONFIG_CODE_VERSION "\n");
    fprintf(fout_hist,
	    "# Code version: monitor" CONFIG_CODE_VERSION "\n");

    /* Dump cpuinfo */
    dump_cpuinfo(fout_trace);
    dump_cpuinfo(fout_hist);

    /* Dump test info */
    dump_testinfo(fout_trace);
    dump_testinfo(fout_hist);

    calc_interval_stats(wake_time, NR_SAMPLES, wake_time_hist, &is);
    dump_interval_stat(fout_trace, "wake_time interval", &is, NULL);

    /* Dump wake time trace */
    fprintf(fout_trace, "# Wake time latency trace (in us):\n");
    for (i = 1; i < NR_SAMPLES; i++) {
	long d = sat_ndiff(&(wake_time[i]), &(wake_time[i - 1]));
	fprintf(fout_trace, "%10ld\n", d);
    }

    /* Dump histograms */
    fprintf(fout_hist, "# Wake time histogram (in us):\n");
    dump_interval_stat(fout_hist, "wake_time interval", &is,
		       wake_time_hist);
}

static char filename_trace[CMD_NAME_SIZE] = FILE_NAME_BASE;
static char filename_hist[CMD_NAME_SIZE] = FILE_NAME_BASE;

int main(int argc, char *argv[])
{
    char *idString = "no_identifier";
    sigset_t blocked_signals;

    fprintf(stderr, "Code version: %s" CONFIG_CODE_VERSION "\n", argv[0]);

    if (argc > 1)
	idString = argv[1];

    strcat(filename_trace, idString);
    strcat(filename_trace, FILE_NAME_TRACE);
    strcat(filename_trace, ".txt");

    if ((fout_trace = fopen(filename_trace, "w")) == NULL) {
	fprintf(stderr,
		"Error: unable to open file %s; output trace on stdout\n",
		filename_trace);
	fout_trace = stdout;
    } else {
	fprintf(stderr, "Monitor started: buffer %d samples: %d trace will be saved in \"%s\"\n",
		BUFFER_SIZE*1024,NR_SAMPLES,filename_trace);
    }

    strcat(filename_hist, idString);
    strcat(filename_hist, FILE_NAME_HIST);
    strcat(filename_hist, ".txt");

    if ((fout_hist = fopen(filename_hist, "w")) == NULL) {
	fprintf(stderr,
		"Error: unable to open file %s; output hist on stdout\n",
		filename_hist);
	fout_hist = stdout;
    } else {
	fprintf(stderr, "             histogram will be saved in \"%s\"\n",
		filename_hist);
    }

    if (check_timer()) {
	fprintf(stderr, "WARNING: High resolution timers not available\n");
	fprintf(fout_trace,
		"WARNING: High resolution timers not available\n");
    }


    /* Block all signals, except SIGINT and SIGTERM */
    sigfillset(&blocked_signals);
    sigdelset(&blocked_signals, SIGINT);
    sigdelset(&blocked_signals, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &blocked_signals, NULL)) {
	perror("sigprocmask");
	return EXIT_FAILURE;
    }

    lock_all(fout_trace, CLOCK_MONOTONIC);

    timed_memset(fout_trace, "(wake_time)", CLOCK_MONOTONIC,
		 wake_time, sizeof(wake_time));

    timed_memset(fout_trace, "(wake_time_hist)", CLOCK_MONOTONIC,
		 wake_time_hist, sizeof(wake_time_hist));

    timed_memset(fout_trace, "(inbuf)", CLOCK_MONOTONIC,
		 inbuf, sizeof(inbuf));

    fflush(stderr);

    if (ftrace_setup()) {
	fprintf(stderr, "ERROR: ftrace setup failed\n");
	exit(1);

    }

    monitor();

    do_stat();

    ftrace_close();
    fclose(fout_trace);
    fclose(fout_hist);

    return EXIT_SUCCESS;
}

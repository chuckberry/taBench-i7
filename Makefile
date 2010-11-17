
# Check for all warnings and make them become an error
CFLAGS += -Wall -O3
CPPFLAGS = $(CFLASG) -pthread
ifdef CPUAFFINITY
	 CPPFLAGS += -DCPUAFFINITY
endif
ifdef OPTIM_AFFINITY
	CPPFLAGS += -DOPTIM_AFFINITY
endif
ifdef WORST_AFFINITY
	CPPFLAGS += -DWORST_AFFINITY
endif
ifdef TASKAFFINITY
	CPPFLAGS += -DTASKAFFINITY
endif
ifdef FTRACE
	CFLAGS += -DCONFIG_USE_TRACING -DCONFIG_USE_FTRACE
endif
ifdef SCHED_SWITCH
	CFLAGS += -DCONFIG_USE_TRACING -DCONFIG_USE_SCHED_SWITCH 
endif
ifndef BSIZE
	BSIZE = 4
endif
CFLAGS += -DBUFFER_SIZE=$(BSIZE)
CPPFLAGS += -DBUFFER_SIZE=$(BSIZE)

B_LDFLAGS += -lrt
B_OBJS += mixer.o wave.o nwBench.o

M_LDFLAGS += -lm -lrt
M_OBJS += monitor.o

VERSION = \"$(shell tools/setlocalversion)\"
CFLAGS += -DCONFIG_CODE_VERSION=$(VERSION)
CPPFLAGS += -DCONFIG_CODE_VERSION=$(VERSION)

BUILD_ALL='\#!/bin/sh\n					\
for S in 2 4 8 16 32; do make BSIZE=$$S; done	\
'
BUILD_CPUAFF='\#!/bin/sh\n					\
for S in 2 4 8 16 32; do make BSIZE=$$S cpuaffinity; done	\
'

all:
	@make soft-clean
	@make nwBench-vanilla
	@make soft-clean
	@make TASKAFFINITY=1 nwBench-taskaff
	@make soft-clean
	@make monitor-notrace
	@make soft-clean
	@make FTRACE=1 monitor-ftrace
	@make soft-clean
	@make SCHED_SWITCH=1 monitor-sched

cpuaffinity:
	@make CPUAFFINITY=1 OPTIM_AFFINITY=1 nwBench-optim-vanilla
	@make soft-clean
	@make CPUAFFINITY=1 WORST_AFFINITY=1 nwBench-worst-vanilla
	@make soft-clean
	@make CPUAFFINITY=1 OPTIM_AFFINITY=1 TASKAFFINITY=1 nwBench-optim-taskaff
	@make soft-clean
	@make CPUAFFINITY=1 WORST_AFFINITY=1 TASKAFFINITY=1 nwBench-worst-taskaff
	@make soft-clean
	@make monitor-notrace
	@make soft-clean
	@make FTRACE=1 monitor-ftrace
	@make soft-clean
	@make SCHED_SWITCH=1 monitor-sched

cpuaff_all_version:
	echo -e ${BUILD_CPUAFF} > build_cpuaff.sh
	chmod a+x build_cpuaff.sh
	./build_cpuaff.sh

all_version:
	echo -e ${BUILD_ALL} > build_all.sh
	chmod a+x build_all.sh
	./build_all.sh

nwBench-vanilla: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

nwBench-taskaff: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

nwBench-optim-vanilla: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

nwBench-worst-vanilla: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

nwBench-optim-taskaff: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

nwBench-worst-taskaff: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@_$(BSIZE)KB

monitor-notrace: $(M_OBJS)
	gcc $(M_LDFLAGS) $(M_OBJS) -o $@_$(BSIZE)KB

monitor-ftrace: $(M_OBJS)
	gcc $(M_LDFLAGS) $(M_OBJS) -o $@_$(BSIZE)KB

monitor-sched: $(M_OBJS)
	gcc $(M_LDFLAGS) $(M_OBJS) -o $@_$(BSIZE)KB

soft-clean:
	rm -rf *.o *~ cscope.*

clean: soft-clean
	rm -rf nwBench{,-vanilla,-taskaff,-optim,-worst}_* monitor{,-notrace,-sched,-ftrace}_* build_all.sh build_cpuaff.sh

index:
	find `pwd` -regex ".*\.[ch]\(pp\)?" -print > cscope.files
	cscope -b -q -k

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o : %.cpp
	g++ $(CPPFLAGS) -c -o $@ $<


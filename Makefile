# Check for all warnings and make them become an error
CFLAGS += -Wall -O3
CPPFLAGS = $(CFLASG) -pthread
ifdef TASKAFFINITY
	CPPFLAGS += -DTASKAFFINITY
endif

B_LDFLAGS += -lrt
B_OBJS += mixer.o wave.o nwBench.o

M_LDFLAGS += -lm -lrt
M_OBJS += monitor.o

all:
	@make nwBench
	@make soft-clean
	@make TASKAFFINITY=1 nwBench-taskaff
	@make monitor

nwBench: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@

nwBench-taskaff: $(B_OBJS)
	g++ $(B_LDFLAGS) $(B_OBJS) -o $@

monitor: $(M_OBJS)
	gcc $(M_LDFLAGS) $(M_OBJS) -o $@

soft-clean:
	rm -rf *.o *~

clean: soft-clean
	rm -rf nwBench{,-taskaff} monitor

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o : %.cpp
	g++ $(CPPFLAGS) -c -o $@ $<


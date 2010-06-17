# Check for all warnings and make them become an error
CPPFLAGS += -Wall -O3 -pthread
ifdef TASKAFFINITY
	CPPFLAGS += -DTASKAFFINITY
endif
LDFLAGS += -lrt

OBJS += mixer.o wave.o nwBench.o

all:
	@make nwBench
	@make soft-clean
	@make TASKAFFINITY=1 nwBench-taskaff

nwBench: $(OBJS)
	g++ $(OBJS) $(LDFLAGS) -pthread -o $@

nwBench-taskaff: $(OBJS)
	g++ $(OBJS) $(LDFLAGS) -pthread -o $@

soft-clean:
	rm -rf *.o *~

clean: soft-clean
	rm -rf nwBench{,-taskaff}

%.o : %.cpp
	g++ $(CPPFLAGS) -c -o $@ $<


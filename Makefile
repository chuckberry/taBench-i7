# Check for all warnings and make them become an error
CXXFLAGS += -Wall -O2

CPPFLAGS += -pthread

OBJS += mixer.o wave.o nwBench.o

all: nwBench

nwBench: $(OBJS)
	g++ $(OBJS) -pthread -o $@

clean:
	rm -rf nwBench *.o *~

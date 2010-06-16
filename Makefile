# Check for all warnings and make them become an error
CXXFLAGS += -Wall -Werror -O2

CPPFLAGS += -I$(NMF_DELIVERY_PATH)/api -pthread

OBJS += mixer.o wave.o nwBench.o

all: nwBench

nwBench: $(OBJS)
	g++ $(OBJS) -pthread -o $@

clean:
	rm -rf nwBench *.o *~

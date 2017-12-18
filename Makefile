TARGET = DIM

NTHREADS = 4
FLAGS_CACTI = -O3 -DNTHREADS=$(NTHREADS) 
FLAGS_DIM = -O3 -Wall#-ggdb -ggdb -Wall


CC    = g++
CPP   = g++
LIBS  = -lm  -lpthread -lz

CPP_SRCS_CACTI = cacti65/area.cc cacti65/bank.cc cacti65/mat.cc cacti65/Ucache.cc cacti65/io.cc cacti65/technology.cc cacti65/basic_circuit.cc cacti65/parameter.cc cacti65/decoder.cc cacti65/component.cc cacti65/uca.cc cacti65/subarray.cc cacti65/wire.cc cacti65/htree2.cc cacti65/cacti_interface.cc cacti65/router.cc cacti65/nuca.cc cacti65/crossbar.cc cacti65/arbiter.cc 
CPP_SRCS_DIM = main.cpp DIM.cpp array.cpp results.cpp cache.cpp DIM_core.cpp cache_block.cpp memory_interface.cpp configuration.cpp

CPP_OBJS_CACTI = $(patsubst %.cc,%.cc.o,$(CPP_SRCS_CACTI))
CPP_OBJS_DIM = $(patsubst %.cpp,%.cc.o,$(CPP_SRCS_DIM))


all: $(TARGET)

$(TARGET): $(CPP_OBJS_CACTI)  $(CPP_OBJS_DIM)
	$(CPP) $(FLAGS_CACTI) $(CPP_OBJS_CACTI) $(FLAGS_DIM)  $(CPP_OBJS_DIM) -o $@  -pthread  -lm   -lz

$(CPP_OBJS_CACTI): %.cc.o: %.cc
	$(CPP) $(FLAGS_CACTI) -c $< -o $@

$(CPP_OBJS_DIM): %.cc.o: %.cpp
	$(CPP) $(FLAGS_DIM) -c $< -o $@

clean:
	rm -rf *.o 
	rm -f cacti65/*.o	

mrproper: clean
	rm -v DIM

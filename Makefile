CXX                = g++ -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./ff

.PHONY: all clean cleanall 

sequential: wavefront.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront wavefront.cpp

fastflow: wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff wavefront_ff.cpp

mpi: wavefront_mpi.cpp
	mpicxx -Wall -O3 wavefront_mpi.cpp -o wavefront_mpi

all: sequential fastflow mpi

clean: 
	-rm -fr *.o wavefront wavefront_ff wavefront_mpi

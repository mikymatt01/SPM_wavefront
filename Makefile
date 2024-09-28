CXX                = g++ -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math -ftree-vectorize #-fopt-info-vec-missed
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./ff


.PHONY: all clean cleanall 

sequential:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront  ./wavefront_sequential/wavefront.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles ./wavefront_sequential/wavefront_triangles.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_collapsed ./wavefront_sequential/wavefront_triangles_collapsed.cpp

fastflow:
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff ./wavefront_fastflow/wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_ff ./wavefront_fastflow/wavefront_triangles_ff.cpp -pthread

mpi: wavefront_mpi.cpp
	mpicxx -Wall -O3 wavefront_mpi.cpp -o wavefront_mpi

all: sequential fastflow mpi

clean: 
	-rm -fr *.o wavefront wavefront_ff wavefront_mpi

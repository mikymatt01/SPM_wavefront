CXX                = g++ -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math -ftree-vectorize #-fopt-info-vec-missed
CXXFLAGS          += -Wall
INCLUDES	   = -I. -I./ff


.PHONY: all clean cleanall 
triangles:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o triangles  ./utils/triangles.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o triangles_collapsed  ./utils/triangles_collapsed.cpp

sequential:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_sequential  ./wavefront_sequential/wavefront_sequential.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles ./wavefront_sequential/wavefront_triangles.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_collapsed ./wavefront_sequential/wavefront_triangles_collapsed.cpp

fastflow:
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_diagonal_ff ./wavefront_fastflow/wavefront_diagonal_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_ff ./wavefront_fastflow/wavefront_triangles_ff.cpp -pthread
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_map_ff ./wavefront_fastflow/wavefront_triangles_map_ff.cpp -pthread
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_map_ff_comm ./wavefront_fastflow/wavefront_triangles_map_ff_comm.cpp -pthread

squares:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o squares  ./utils/squares.cpp

mpi:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangle_dependences ./wavefront_mpi/wavefront_triangle_dependences.cpp
#mpicxx -Wall -O3 wavefront_mpi.cpp -o wavefront_mpi

all: sequential fastflow mpi

clean: 
	-rm -fr *.o wavefront wavefront_ff wavefront_mpi triangles
	-rm -dr tests

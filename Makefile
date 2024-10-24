CXX                = g++ -std=c++20
OPTFLAGS	   = -O3 -ffast-math -ftree-vectorize #-fopt-info-vec-missed
CXXFLAGS          += -Wall
INCLUDES	   = -I. -I./ff


.PHONY: all clean cleanall 

sequential:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_diagonal  ./wavefront_sequential/wavefront_diagonal.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles ./wavefront_sequential/wavefront_triangles.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_collapsed ./wavefront_sequential/wavefront_triangles_collapsed.cpp

fastflow:
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_diagonal_ff ./wavefront_fastflow/wavefront_diagonal_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_ff ./wavefront_fastflow/wavefront_triangles_ff.cpp

mpi:
	mpicxx -std=c++20 -Wall $(OPTFLAGS) -o wavefront_diagonal_sg_mpi ./wavefront_mpi/wavefront_diagonal_sg_mpi.cpp
	mpicxx -std=c++20 -Wall $(OPTFLAGS) -o wavefront_diagonal_allg_mpi ./wavefront_mpi/wavefront_diagonal_allg_mpi.cpp
	mpicxx -std=c++20 -Wall $(OPTFLAGS) -o wavefront_triangles_allg_mpi ./wavefront_mpi/wavefront_triangles_allg_mpi.cpp

mpi_triangles_test:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_test ./wavefront_mpi/_wavefront_triangles_test.cpp

triangles:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o triangles  ./utils/triangles.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o triangles_collapsed  ./utils/triangles_collapsed.cpp

squares:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o squares  ./utils/squares.cpp

all: sequential fastflow mpi

clean: 
	-rm -fr *.o wavefront wavefront_ff wavefront_mpi triangles
	-rm -dr tests

CXX                = g++ -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math -ftree-vectorize -fopt-info-vec-missed
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./ff


.PHONY: all clean cleanall 

sequential: wavefront.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront wavefront.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles wavefront_triangles.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_1 wavefront_triangles_1.cpp

fastflow: wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_triangles_ff wavefront_triangles_ff.cpp

map: wavefront_map.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_map wavefront_map.cpp

mpi: wavefront_mpi.cpp
	mpicxx -Wall -O3 wavefront_mpi.cpp -o wavefront_mpi

squares: squares.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o squares squares.cpp

all: sequential fastflow mpi

clean: 
	-rm -fr *.o wavefront wavefront_ff wavefront_mpi

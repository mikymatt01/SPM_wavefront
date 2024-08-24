CXX                = g++ -std=c++20
MPICXX             = mpicxx -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./ff
LIBS               = 
SOURCES            = $(wildcard *.cpp)
TARGET             = $(SOURCES:.cpp=)

.PHONY: all clean cleanall 

seq: wavefront.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront wavefront.cpp $(LIBS)

ff: wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff wavefront_ff.cpp $(LIBS)

ff_v2: wavefront_ff_v2.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff_v2 wavefront_ff_v2.cpp $(LIBS)

ff_v3: wavefront_ff_v3.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff_v3 wavefront_ff_v3.cpp $(LIBS)

ff_v4: wavefront_ff_v4.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff_v4 wavefront_ff_v4.cpp $(LIBS)

mpi: wavefront_mpi.cpp
	$(MPICXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_mpi wavefront_mpi.cpp $(LIBS)

all: seq ff ff_v2 ff_v3 ff_v4 mpi

clean: 
	-rm -fr *.o *~
cleanall: clean
	-rm -fr $(TARGET)




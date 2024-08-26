CXX                = g++ -std=c++20
MPICXX             = mpicxx -std=c++20
OPTFLAGS	   = -O3 -march=native -ffast-math
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./ff
LIBS               = 
SOURCES            = $(wildcard *.cpp)
TARGET             = $(SOURCES:.cpp=)

.PHONY: all clean cleanall 

sequential: wavefront.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront wavefront.cpp $(LIBS)

fastflow: wavefront_ff.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_ff wavefront_ff.cpp $(LIBS)

mpi: wavefront_mpi.cpp
	$(MPICXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o wavefront_mpi wavefront_mpi.cpp $(LIBS)

all: sequential fastflow mpi

clean: 
	-rm -fr *.o *~
cleanall: clean
	-rm -fr $(TARGET)




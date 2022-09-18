CXXFLAGS = -Og  -std=c++2a -Wfatal-errors

release: ./makefile $(wildcard ./*.cpp)
	g++ -o st-visualizer -I ./eigen-3.4.0 $(CXXFLAGS) ./main.cpp $(wildcard ./*.cpp) ./tetgen1.6.0/tetgen.cxx ./tetgen1.6.0/tetgen.cxx
 
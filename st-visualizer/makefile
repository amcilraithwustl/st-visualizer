
# CXX = g++-13
CXX = clang++
CXXFLAGS = -Og -g -std=c++2a -Wfatal-errors -w
INCLUDES = -I ./eigen-3.4.0 -I ./tetgen-1.6.0 -I ./triangle-1.6

release: $(wildcard ./*.cpp)
	$(CXX) $(CXXFLAGS) -o st-visualizer $(wildcard ./*.cpp) $(INCLUDES) 

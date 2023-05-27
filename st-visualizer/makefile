
# CXX = g++-13
CXX = clang++
BIN = ./bin
CXXFLAGS = -Og -g -std=c++2a -Wfatal-errors -w
INCLUDES = -Ieigen-3.4.0 -Itetgen-1.6.0 -Itriangle-1.6

OBJECTS = 

release: $(wildcard ./*.cpp)
	$(CXX) $(CXXFLAGS) -o st-visualizer $(wildcard ./*.cpp) $(wildcard tetgen-1.6.0/*.cxx) $(wildcard triangle-1.6/*.c) $(INCLUDES) 

# UtilityFunctions.o: UtilityFunctions.cpp UtilityFunctions.h
# 	$(CXX) -c $(BIN)/UtilityFunctions.o


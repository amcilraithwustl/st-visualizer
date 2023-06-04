CC = clang
CXX = clang++
BIN = ./bin/
CCFLAGS = -Og -g -std=c99 -Wfatal-errors -w -lm -arch x86_64 -arch arm64
CXXFLAGS = -Og -g -std=c++2a -Wfatal-errors -w -lm -arch x86_64 -arch arm64
INCLUDES = -Ieigen-3.4.0 -Itetgen-1.6.0 -Itriangle-1.6

release: $(wildcard ./*.cpp) $(BIN)tetgen.o $(BIN)tricall.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -lm -o $(BIN)st-visualizer $(wildcard ./*.cpp) $(wildcard $(BIN)*.o)

dependencies: $(BIN)tetgen.o $(BIN)tricall.o

tetgen: $(BIN)tetgen.o

$(BIN)tetgen.o: tetgen-1.6.0/tetgen.cxx tetgen-1.6.0/tetgen.h $(BIN)predicates.o
	$(CXX) $(CXXFLAGS) -c -o $(BIN)tetgen.o tetgen-1.6.0/tetgen.cxx $(BIN)predicates.o

$(BIN)predicates.o: tetgen-1.6.0/predicates.cxx
	$(CXX) $(CXXFLAGS) -c -o $(BIN)predicates.o tetgen-1.6.0/predicates.cxx

triangle: $(BIN)tricall.o

$(BIN)tricall.o: triangle-1.6/tricall.c $(BIN)triangle.o
	$(CC) $(CCFLAGS) -c -o $(BIN)tricall.o triangle-1.6/tricall.c $(BIN)triangle.o

$(BIN)triangle.o: triangle-1.6/triangle.c triangle-1.6/triangle.h 
	$(CC) $(CCFLAGS) -c -o $(BIN)triangle.o triangle-1.6/triangle.c

clean:
	rm -rf $(BIN)*
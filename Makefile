CXX=g++
CXXFLAGS=-std=c++17 -Iinc

all: main

main: src/main.cpp src/query_processor.cpp src/engine.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f main
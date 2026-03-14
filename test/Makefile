CXX = g++
CXXFLAGS = -std=c++23 -Iinc -I../inc
PARENT_OBJS = ../src/query_processor.o ../src/engine.o

.PHONY: all parent test clean

all: test

parent: $(PARENT_OBJS)

../src/%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: query_processor.test.cpp parent
	$(CXX) $(CXXFLAGS) query_processor.test.cpp $(PARENT_OBJS) -o test.exe

clean:
	rm -f test.exe ../src/*.o
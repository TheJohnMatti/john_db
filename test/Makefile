CXX = g++
CXXFLAGS = -std=c++23 -Iinc -I../inc
PARENT_OBJS = ../src/query_processor.o ../src/page_io.o ../src/engine.o

.PHONY: all parent test clean

all: test

parent: $(PARENT_OBJS)

../src/%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

query_processor_test: query_processor.test.cpp parent
	$(CXX) $(CXXFLAGS) query_processor.test.cpp $(PARENT_OBJS) -o ../query_processor_test.exe

page_io_test: page_io.test.cpp parent
	$(CXX) $(CXXFLAGS) page_io.test.cpp $(PARENT_OBJS) -o ../page_io_test.exe

test: query_processor_test page_io_test

clean:
	rm -f *.exe ../src/*.o
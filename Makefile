CXX = g++
CXXFLAGS = -std=c++23 -Iinc

SRCS = src\main.cpp src\query_processor.cpp src\engine.cpp src\page_io.cpp# src\memory_layer.cpp
OBJS = $(SRCS:.cpp=.o)

all: main

main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-@del /Q main $(OBJS) 2>nul
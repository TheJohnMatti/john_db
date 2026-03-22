cmake_minimum_required(VERSION 3.20)

project(JohnDB LANGUAGES CXX)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include headers
include_directories(inc)

# Source files
set(SOURCES
    src/main.cpp
    src/query_processor.cpp
    src/engine.cpp
    src/page_io.cpp
    src/memory_layer.cpp
)

# Executable
add_executable(main ${SOURCES})
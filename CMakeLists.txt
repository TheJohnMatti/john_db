cmake_minimum_required(VERSION 3.20)

project(SteleDB LANGUAGES CXX)

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
    src/b_tree.cpp
)

# Executable
add_executable(main ${SOURCES})

# --- Tests (CTest) ---
enable_testing()

function(steledb_add_test_simple name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/inc)
    add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

steledb_add_test_simple(test_b_tree test/b_tree.test.cpp src/b_tree.cpp src/page_io.cpp)
steledb_add_test_simple(test_query_processor test/query_processor.test.cpp src/query_processor.cpp)
steledb_add_test_simple(test_page_io test/page_io.test.cpp src/page_io.cpp)
steledb_add_test_simple(test_memory_layer test/memory_layer.test.cpp src/memory_layer.cpp src/b_tree.cpp src/page_io.cpp)
steledb_add_test_simple(test_engine test/engine.test.cpp src/engine.cpp src/query_processor.cpp src/page_io.cpp src/memory_layer.cpp src/b_tree.cpp)
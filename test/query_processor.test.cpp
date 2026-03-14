#include <iostream>
#include <cassert>
#include "query_processor.hpp"

int main() {

    // is_float
    auto is_float = QueryProcessor::is_float;
    assert(is_float("a") == false);
    assert(is_float("1.234") == true);
    assert(is_float("-324.23432") == true);

    // is_int
    auto is_int = QueryProcessor::is_int;
    assert(is_int("-3214") == true);
    assert(is_int("a342132") == false);
    assert(is_int("29.2") == false);
    assert(is_int("+32098742314702") == true);

    // is_string
    auto is_string = QueryProcessor::is_string;
    assert(is_string("hello world") == false);
    assert(is_string("\"There once was a man\"") == true);
    assert(is_string("\"finders keepers") == false);

    std::cout << "query_processor.test.cpp: All Tests Passed" << std::endl; 

    return 0;
}
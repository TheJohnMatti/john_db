#include <iostream>
#include <string>
#include <string_view>
#include <csignal>
#include "query_processor.hpp"
#include "engine.hpp"

void print_intro() {
    std::cout << "Welcome to SteleDB!" << std::endl;
}

int main() {
    
    print_intro();
    Engine engine;
    engine.run();

    return 0;
}
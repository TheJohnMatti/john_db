#include <iostream>
#include <string>
#include <string_view>
#include "query_processor.hpp"
#include "engine.hpp"

void print_intro() {
    std::cout << "Welcome to JohnDB!" << std::endl;
}

void print_options() {
    
}

int main() {
    
    print_intro();
    QueryProcessor processor;

    while (1) {
        std::string query;
        std::getline(std::cin, query);
        if (query == "quit") break;
        if (query == "help") {
            print_options();
            continue;
        }
        processor.process(std::string_view(query));
    }

    return 0;
}
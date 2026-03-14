#include <iostream>
#include <string>
#include <string_view>
#include <csignal>
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
    Engine::instance().init();

    while (1) {
        std::cout << "johndb> ";
        std::string query;
        if (!std::getline(std::cin, query)) {
            std::cin.clear();
            std::cout << '\n';
            return -1;
        };
        if (query == "quit") break;
        if (query == "help") {
            print_options();
            continue;
        }
        processor.process(std::string_view(query));
    }

    return 0;
}
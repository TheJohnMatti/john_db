#include <iostream>
#include <filesystem>
#include "engine.hpp"

Engine::Engine() {};

Engine& Engine::instance() {
    static Engine inst;
    return inst;
}

void Engine::read_tables_folder() {
    std::filesystem::path path{"./tables"};
    std::filesystem::directory_iterator dir{path};
    for (auto &i : dir) {
        std::cout << i << std::endl;
    }
}
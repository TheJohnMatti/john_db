#pragma once
#include "table.hpp"
#include <vector>

class Engine {

    private:
    Engine();
    std::vector<Table> tables;

    public:
    static Engine& instance();
    void read_tables_folder();

};
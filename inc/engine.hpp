#pragma once
#include "table.hpp"
#include <vector>
#include <filesystem>

class Engine {

    private:
    Engine();
    std::vector<Table> tables;

    public:
    static Engine& instance();
    void read_tables_folder();
    Table parse_table_metadata(std::filesystem::directory_entry);
    void write_table_metadata(Table &table);

};
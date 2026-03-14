#pragma once
#include "table.hpp"
#include <vector>
#include <filesystem>

class Engine {

    private:
    Engine();
    std::vector<Table> tables;
    std::array<DataType, 256> char_to_type = {};

    public:
    static Engine& instance();
    void read_tables_folder();
    Table parse_table_metadata(std::filesystem::directory_entry);
    void write_table_metadata(Table &table);

};
#pragma once
#include "table.hpp"
#include <unordered_map>
#include <filesystem>
#include <string>
#include <string_view>
class Engine {

    private:
    Engine();
    std::unordered_map<std::string, Table> tables;

    public:
    static Engine& instance();
    void init();
    void read_tables_folder();
    Table parse_table_metadata(std::filesystem::directory_entry);
    void write_table_metadata(Table &table);
    
};
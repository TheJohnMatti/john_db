#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>
#include <string_view>
#include "table.hpp"
#include "query_processor.hpp"
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
    void run(Query&);
    QueryResult run_select(Query&);
    void run_create(Query&);
    void run_insert(Query&);
    void run_delete(Query&);

};
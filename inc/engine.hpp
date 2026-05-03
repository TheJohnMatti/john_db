#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>
#include <string_view>
#include "table.hpp"
#include "query_processor.hpp"
#include "query_result.hpp"
#include "query.hpp"
#include "memory_layer.hpp"
class Engine {

    public:
    Engine(std::string data_dir = "tables");
    void run_query(std::string_view query);
    void run();

    private:
    std::unordered_map<std::string, Table> tables;
    MemoryLayer memory_layer;
    QueryProcessor query_processor;
    void read_tables_folder();
    void run_parsed_query(Query&);
    void run_select(Query&);
    void run_create(Query&);
    void run_insert(Query&);
    void run_delete(Query&);
    void run_drop(Query&);

};
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include "table.hpp"
#include "data_type.hpp"
#include "predicate.hpp"
#include "page_io.hpp"

class MemoryLayer {
    
    public:
    MemoryLayer(std::string data_dir = "tables");
    void create_table(Table &table);
    void insert(Table &table, std::vector<Data> &values);
    void select(Table &table, std::vector<std::string> cols, VariablePredicate &pred);
    size_t remove(Table &table, VariablePredicate &pred);

    private:
    std::string data_dir;
    static std::optional<std::pair<size_t, size_t>> try_insert_at(Table &table, std::vector<Data> &values,
                                                                  size_t page_index);
};

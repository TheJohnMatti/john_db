#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "table.hpp"
#include "data_type.hpp"
#include "predicate.hpp"
#include "page_io.hpp"

class MemoryLayer {
    
    public:
    static MemoryLayer &instance();
    void insert(Table &table, std::vector<Data> &values);
    bool insert_at(Table &table, std::vector<Data> &values, const size_t page_index);
    void select(Table &table, std::vector<std::string> cols, VariablePredicate &pred);

    private:
    MemoryLayer();
};

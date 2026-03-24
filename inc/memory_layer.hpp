#pragma once

#include <iostream>
#include <vector>
#include "table.hpp"
#include "data_type.hpp"
#include "page_io.hpp"

class MemoryLayer {
    
    public:
    static MemoryLayer &instance();
    void insert(Table &table, std::vector<Data> &values);
    bool insert_at(Table &table, std::vector<Data> &values, const size_t page_index);

    private:
    MemoryLayer();



    // template<typename T>
    // QueryResult select_query(Table& table, std::vector<std::string> &columns, std::optional<Predicate<T>> predicate) {

    // }
};

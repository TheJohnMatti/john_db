#pragma once

#include <iostream>
#include "table.hpp"

class MemoryLayer {
    
    public:
    static MemoryLayer &instance();
    //void insert_query(Table &table);

    private:
    MemoryLayer();



    // template<typename T>
    // QueryResult select_query(Table& table, std::vector<std::string> &columns, std::optional<Predicate<T>> predicate) {

    // }
};

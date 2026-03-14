#pragma once
#include <string>
#include <vector>
#include "column.hpp"

struct Table {
    Table(std::string name) : name{name} {}
    std::string name;
    std::vector<Column> columns;
    void add_column(DataType col_type, std::string &&col_name) {
        columns.push_back(Column{col_name, col_type, false});
    }
};
#pragma once

#include <string>
#include <unordered_map>
#include "column.hpp"

struct Table {
    Table() : name{"unnamed_table"} {}
    Table(std::string name) : name{name} {}
    std::string name;
    std::unordered_map<std::string, Column> columns;
    void add_column(DataType col_type, std::string &&col_name) {
        columns[col_name] = Column{col_name, col_type, false};
    }
};
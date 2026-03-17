#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "column.hpp"

struct Table {
    Table() : name{"unnamed_table"} {}
    Table(std::string name) : name{std::move(name)} {}
    std::string name;
    std::vector<Column> columns;
    std::unordered_map<std::string, size_t> column_index;
    size_t row_size = 0, pages = 0;
    void add_column(DataType col_type, std::string &&col_name, bool is_index=false, bool is_primary=false) {
        size_t index = columns.size();
        columns.push_back(Column{std::move(col_name), col_type, is_index, is_primary});
        column_index[columns.back().name] = index;
        row_size += get_data_size(col_type);
    }
    bool has_column(const std::string &column_name) const {
        return column_index.count(column_name);
    }
    Column& get_column(const std::string &name) {
        return columns[column_index.at(name)];
    }
    size_t get_column_index(const std::string &name) const {
        return column_index.at(name);
    }
};
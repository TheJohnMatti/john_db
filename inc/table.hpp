#pragma once
#include <string>
#include <vector>
#include "column.hpp"

struct Table {
    std::string name;
    std::vector<Column> columns;

};
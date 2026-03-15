#pragma once

#include <string>
#include "data_type.hpp"

struct Column {
    std::string name;
    DataType type;
    bool is_index;
};
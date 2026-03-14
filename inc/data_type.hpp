#pragma once
#include <array>

enum DataType {
    UNDEFINED = 0,
    INT,
    VARCHAR,
    BOOL,
};

const std::array data_to_char = {
    'u',
    'i',
    's',
    'b'
};
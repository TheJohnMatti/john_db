#pragma once
#include <array>

enum class DataType : uint8_t {
    UNDEFINED = 0,
    INT,
    VARCHAR,
    BOOL,
};

constexpr std::array data_to_char = {
    'u',
    'i',
    's',
    'b'
};

constexpr std::array<DataType, 256> char_to_data = []{
    std::array<DataType,256> arr{};
    arr.fill(DataType::UNDEFINED);
    arr['i'] = DataType::INT;
    arr['s'] = DataType::VARCHAR;
    arr['b'] = DataType::BOOL;
    return arr;
}();
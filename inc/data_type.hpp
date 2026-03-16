#pragma once

#include <array>
#include <variant>

// not open to reordering
enum class DataType : uint8_t {
    UNDEFINED = 0,
    INT,
    VARCHAR_15,
    BOOL,
    FLOAT,
};

constexpr std::array data_to_char = {
    'u',
    'i',
    's',
    'b',
    'f',
};

constexpr std::array<DataType, 256> char_to_data = []{
    std::array<DataType,256> arr{};
    arr.fill(DataType::UNDEFINED);
    arr['i'] = DataType::INT;
    arr['s'] = DataType::VARCHAR_15;
    arr['b'] = DataType::BOOL;
    return arr;
}();

using Data = std::variant<std::monostate, std::string, int, bool, double>;
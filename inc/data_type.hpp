#pragma once

#include <array>
#include <variant>

using Data = std::variant<std::monostate, std::string, int, bool, double>;

// not open to reordering
enum class DataType : uint8_t {
    UNDEFINED = 0,
    VARCHAR_16,
    INT,
    BOOL,
    FLOAT,
};

constexpr std::array data_to_char = {
    'u',
    's',
    'i',
    'b',
    'f',
};

constexpr std::array<DataType, 256> char_to_data = []{
    std::array<DataType,256> arr{};
    arr.fill(DataType::UNDEFINED);
    arr['i'] = DataType::INT;
    arr['s'] = DataType::VARCHAR_16;
    arr['b'] = DataType::BOOL;
    arr['f'] = DataType::FLOAT;
    return arr;
}();

static constexpr std::array data_to_size = {
    0uz,
    16uz,
    4uz,
    1uz,
    8uz,
};

inline size_t get_data_size(DataType data_type) {
    return data_to_size[(size_t)data_type];
};
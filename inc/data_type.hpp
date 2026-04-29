#pragma once

#include <array>
#include <variant>

using Data = std::variant<std::monostate, std::string, int, bool, double>;

const size_t MAX_STRING_SIZE = 16;

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
    std::array<DataType, 256> arr{};
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

inline DataType data_index_to_data_type(Data &data) {
    switch (data.index()) {
        case 0:
            return DataType::UNDEFINED;
        case 1:
            return DataType::VARCHAR_16;
        case 2:
            return DataType::INT;
        case 3:
            return DataType::BOOL;
        case 4: 
            return DataType::FLOAT;
        default:
            return DataType::UNDEFINED;
    }
}

inline void write_to_bytes(Data &data, char *dest) {
    switch (data.index()) {
        case 0:
            break;
        case 1: {
            std::string &data_str = std::get<std::string>(data);
            if (data_str.size() > MAX_STRING_SIZE) throw std::runtime_error("String is too long");
            std::copy(data_str.begin(), data_str.end(), dest);
            size_t remaining = MAX_STRING_SIZE - data_str.size();
            if (remaining > 0) std::fill(dest + data_str.size(), dest + MAX_STRING_SIZE, '\0');
            break;
        }
        case 2: {
            int val = std::get<int>(data);
            std::memcpy(dest, &val, sizeof(int));
            break;
        }
        case 3: {
            bool bool_val = std::get<bool>(data);
            std::memcpy(dest, &bool_val, sizeof(bool));
            break;
        }
        case 4: {
            double double_val = std::get<double>(data);
            std::memcpy(dest, &double_val, sizeof(double));
            break;
        }
        default:
            break;
    }
}

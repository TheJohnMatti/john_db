#include "memory_layer.hpp"
#include "predicate.hpp"
#include <cstring>
#include <type_traits>

MemoryLayer::MemoryLayer() {}

bool MemoryLayer::insert_at(Table &table, std::vector<Data> &values, const size_t page_index) {
    LogicalPage page = PageIO::read_page(table.name, page_index);
    auto &occupancy_bits = page.occupancy;
    size_t entries = occupancy_bits.count(), max_occupancy = std::min(MAX_OCCUPANCY, SLOT_SPACE / table.row_size);
    if (entries >= max_occupancy) return false;
    for (size_t j{0}; j < max_occupancy; j++) {
        if (!occupancy_bits[j]) {
            occupancy_bits[j] = 1;
            size_t starting_byte = j * table.row_size;
            for (size_t col{0}; col < table.columns.size(); col++) {
                write_to_bytes(values[col], page.data+starting_byte);
                starting_byte += get_data_size(table.columns[col].type);
            }
            PageIO::write_page(table.name, page_index, page);
            return true;
        }
    }
    return false;
}

void MemoryLayer::insert(Table &table, std::vector<Data> &values) {
    size_t page_count = table.pages;
    for (size_t i{0}; i < page_count; i++) {
        if (insert_at(table, values, i)) return;
    }
    table.pages++;
    table.write_table_metadata();
    PageIO::create_page(table.name, page_count);
    insert_at(table, values, page_count);
}

void MemoryLayer::select(Table &table, std::vector<std::string> cols, VariablePredicate &pred) {
    size_t predicate_column_index = 0;
    bool has_predicate = false;
    switch (pred.index()) {
        case 0: {
            auto &typed_pred = std::get<Predicate<int>>(pred);
            if (!table.has_column(typed_pred.target_column)) throw ReferenceError("Column " + typed_pred.target_column + " does not exist in table " + table.name);
            predicate_column_index = table.get_column_index(typed_pred.target_column);
            if (table.columns[predicate_column_index].type != DataType::INT) throw TypeError("Predicate type does not match column type");
            has_predicate = true;
            break;
        }
        case 1: {
            auto &typed_pred = std::get<Predicate<std::string>>(pred);
            if (!table.has_column(typed_pred.target_column)) throw ReferenceError("Column " + typed_pred.target_column + " does not exist in table " + table.name);
            predicate_column_index = table.get_column_index(typed_pred.target_column);
            if (table.columns[predicate_column_index].type != DataType::VARCHAR_16) throw TypeError("Predicate type does not match column type");
            has_predicate = true;
            break;
        }
        case 2: {
            auto &typed_pred = std::get<Predicate<bool>>(pred);
            if (!table.has_column(typed_pred.target_column)) throw ReferenceError("Column " + typed_pred.target_column + " does not exist in table " + table.name);
            predicate_column_index = table.get_column_index(typed_pred.target_column);
            if (table.columns[predicate_column_index].type != DataType::BOOL) throw TypeError("Predicate type does not match column type");
            has_predicate = true;
            break;
        }
        case 3: {
            auto &typed_pred = std::get<Predicate<double>>(pred);
            if (!table.has_column(typed_pred.target_column)) throw ReferenceError("Column " + typed_pred.target_column + " does not exist in table " + table.name);
            predicate_column_index = table.get_column_index(typed_pred.target_column);
            if (table.columns[predicate_column_index].type != DataType::FLOAT) throw TypeError("Predicate type does not match column type");
            has_predicate = true;
            break;
        }
        case 4:
            break;
        default:
            throw LogicError("Invalid predicate type");
    }
    std::vector<size_t> selected_indices(cols.size());
    std::vector<size_t> column_offsets(table.columns.size());
    size_t running_offset = 0;
    for (size_t col_index = 0; col_index < table.columns.size(); col_index++) {
        column_offsets[col_index] = running_offset;
        running_offset += get_data_size(table.columns[col_index].type);
    }
    for (size_t i = 0; i < cols.size(); i++) {
        if (!table.has_column(cols[i])) throw ReferenceError("Column " + cols[i] + " does not exist in table " + table.name);
        selected_indices[i] = table.get_column_index(cols[i]);
    }
    for (auto &col : cols) std::cout << col << '\t';
    std::cout << '\n';
    auto print_value = [&](Data &entry) {
        switch (entry.index()) {
            case 0:
                std::cout << '\t';
                break;
            case 1:
                std::cout << std::get<std::string>(entry) << '\t';
                break;
            case 2:
                std::cout << std::get<int>(entry) << '\t';
                break;
            case 3:
                std::cout << std::get<bool>(entry) << '\t';
                break;
            case 4:
                std::cout << std::get<double>(entry) << '\t';
                break;
            default:
                std::cout << '\t';
                break;
        }
    };
    auto read_data = [&](DataType type, char *source) {
        switch (type) {
            case DataType::VARCHAR_16: {
                std::string value(source, source + MAX_STRING_SIZE);
                size_t terminator = value.find('\0');
                if (terminator != std::string::npos) value.resize(terminator);
                return Data{std::move(value)};
            }
            case DataType::INT: {
                int value;
                std::memcpy(&value, source, sizeof(int));
                return Data{value};
            }
            case DataType::BOOL: {
                bool value;
                std::memcpy(&value, source, sizeof(bool));
                return Data{value};
            }
            case DataType::FLOAT: {
                double value;
                std::memcpy(&value, source, sizeof(double));
                return Data{value};
            }
            default:
                return Data{std::monostate{}};
        }
    };
    size_t max_occupancy = std::min(MAX_OCCUPANCY, SLOT_SPACE / table.row_size);
    for (size_t page_index{0}; page_index < table.pages; page_index++) {
        LogicalPage page = PageIO::read_page(table.name, page_index);
        for (size_t slot{0}; slot < max_occupancy; slot++) {
            if (!page.occupancy[slot]) continue;
            size_t row_start = slot * table.row_size;
            if (row_start + table.row_size > SLOT_SPACE) continue;
            Data predicate_value = std::monostate{};
            if (has_predicate) {
                size_t offset = column_offsets[predicate_column_index];
                if (row_start + offset + get_data_size(table.columns[predicate_column_index].type) > SLOT_SPACE) continue;
                predicate_value = read_data(table.columns[predicate_column_index].type, page.data + row_start + offset);
            }
            bool matches = std::visit([&](auto &typed_pred) {
                return typed_pred.eval(predicate_value);
            }, pred);
            if (!matches) continue;
            for (size_t i = 0; i < selected_indices.size(); i++) {
                size_t offset = column_offsets[selected_indices[i]];
                if (row_start + offset + get_data_size(table.columns[selected_indices[i]].type) > SLOT_SPACE) continue;
                Data value = read_data(table.columns[selected_indices[i]].type, page.data + row_start + offset);
                print_value(value);
            }
            std::cout << '\n';
        }
    }
}

MemoryLayer &MemoryLayer::instance() {
    static MemoryLayer memory_layer;
    return memory_layer;
}
#include "memory_layer.hpp"
#include "predicate.hpp"
#include "token.hpp"
#include <cstring>
#include <type_traits>

namespace {

uint64_t pack_row_locator(size_t page_idx, size_t slot_idx) {
    return (static_cast<uint64_t>(page_idx) << 32)
           | static_cast<uint64_t>(static_cast<uint32_t>(slot_idx));
}

void unpack_row_locator(uint64_t packed, size_t &page_idx, size_t &slot_idx) {
    page_idx = static_cast<size_t>(packed >> 32);
    slot_idx = static_cast<size_t>(static_cast<uint32_t>(packed & 0xffffffffull));
}

bool pk_equality_fast_path(const Table &table, const VariablePredicate &pred, uint64_t &pk_out) {
    if (!table.primary_key_btree || table.columns.empty()) {
        return false;
    }
    size_t pk_idx = table.get_primary_key_index();
    DataType pk_type = table.columns[pk_idx].type;
    if (pred.index() != static_cast<size_t>(pk_type)) {
        return false;
    }
    bool is_equal = false;
    switch (pk_type) {
        case DataType::INT: {
            const auto &p = std::get<Predicate<int>>(pred);
            if (p.op.type != TokenType::EQUAL) return false;
            if (table.get_column_index(p.target_column) != pk_idx) return false;
            if (!std::holds_alternative<int>(p.arg2.data)) return false;
            pk_out = static_cast<uint64_t>(std::get<int>(p.arg2.data));
            is_equal = true;
            break;
        }
        case DataType::VARCHAR_16: {
            // For strings, we can't easily hash to uint64_t, so skip for now
            return false;
        }
        case DataType::BOOL: {
            const auto &p = std::get<Predicate<bool>>(pred);
            if (p.op.type != TokenType::EQUAL) return false;
            if (table.get_column_index(p.target_column) != pk_idx) return false;
            if (!std::holds_alternative<bool>(p.arg2.data)) return false;
            pk_out = static_cast<uint64_t>(std::get<bool>(p.arg2.data));
            is_equal = true;
            break;
        }
        case DataType::FLOAT: {
            // Floats are tricky for equality, skip
            return false;
        }
        default:
            return false;
    }
    return is_equal;
}

} // namespace

MemoryLayer::MemoryLayer(std::string data_dir) : data_dir(data_dir) {
    std::filesystem::create_directories(data_dir);
    PageIO::set_data_dir(data_dir);
}

void MemoryLayer::create_table(Table &table) {
    table.table_dir = data_dir + "/" + table.name;
    std::filesystem::create_directories(table.table_dir);
    // Initialize the primary key BTree
    if (!table.primary_key_btree) {
        table.primary_key_btree = std::make_unique<BTree>(
            "primary_key",
            (std::filesystem::path(table.table_dir) / "btrees").string()
        );
    }
}

std::optional<std::pair<size_t, size_t>> MemoryLayer::try_insert_at(Table &table, std::vector<Data> &values,
                                                                   size_t page_index) {
    LogicalPage page = PageIO::read_page(table.name, page_index);
    auto &occupancy_bits = page.occupancy;
    const size_t entries = occupancy_bits.count();
    const size_t max_occupancy = std::min(MAX_OCCUPANCY, SLOT_SPACE / table.row_size);
    if (entries >= max_occupancy) {
        return std::nullopt;
    }
    for (size_t j{0}; j < max_occupancy; j++) {
        if (!occupancy_bits[j]) {
            occupancy_bits[j] = 1;
            size_t starting_byte = j * table.row_size;
            for (size_t col{0}; col < table.columns.size(); col++) {
                write_to_bytes(values[col], page.data + starting_byte);
                starting_byte += get_data_size(table.columns[col].type);
            }
            PageIO::write_page(table.name, page_index, page);
            return std::pair<size_t, size_t>{page_index, j};
        }
    }
    return std::nullopt;
}

void MemoryLayer::insert(Table &table, std::vector<Data> &values) {
    uint64_t pk_value = 0;
    const bool btree_pk = !values.empty() && table.primary_key_btree;
    if (btree_pk) {
        const Data &pk_data = values[0];
        if (std::holds_alternative<int>(pk_data)) {
            pk_value = static_cast<uint64_t>(std::get<int>(pk_data));
        } else if (std::holds_alternative<bool>(pk_data)) {
            pk_value = static_cast<uint64_t>(std::get<bool>(pk_data));
        } else if (std::holds_alternative<std::string>(pk_data)) {
            throw std::runtime_error("String primary keys not supported yet");
        } else {
            throw std::runtime_error("Unsupported primary key type");
        }
        if (table.primary_key_btree->contains(pk_value)) {
            throw std::runtime_error("Primary key already exists");
        }
    }

    const size_t page_count = table.pages;
    for (size_t i{0}; i < page_count; i++) {
        if (auto loc = try_insert_at(table, values, i)) {
            if (btree_pk) {
                table.primary_key_btree->insert(pk_value, pack_row_locator(loc->first, loc->second));
            }
            return;
        }
    }
    table.pages++;
    table.write_table_metadata();
    PageIO::create_page(table.name, page_count);
    if (auto loc = try_insert_at(table, values, page_count)) {
        if (btree_pk) {
            table.primary_key_btree->insert(pk_value, pack_row_locator(loc->first, loc->second));
        }
        return;
    }
    throw std::runtime_error("Failed to insert row into new page");
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
    const size_t max_occupancy = std::min(MAX_OCCUPANCY, SLOT_SPACE / table.row_size);

    uint64_t pk_lookup = 0;
    if (has_predicate && pk_equality_fast_path(table, pred, pk_lookup)) {
        if (table.primary_key_btree->contains(pk_lookup)) {
            try {
                const uint64_t packed = table.primary_key_btree->search(pk_lookup);
                size_t page_idx = 0;
                size_t slot_idx = 0;
                unpack_row_locator(packed, page_idx, slot_idx);
                if (page_idx < table.pages && slot_idx < max_occupancy) {
                    LogicalPage page = PageIO::read_page(table.name, page_idx);
                    if (page.occupancy[slot_idx]) {
                        const size_t row_start = slot_idx * table.row_size;
                        if (row_start + table.row_size <= SLOT_SPACE) {
                            // For simplicity, assume pk is int for now, but can generalize
                            Data pk_val = read_data(table.columns[table.get_primary_key_index()].type, page.data + row_start + column_offsets[table.get_primary_key_index()]);
                            bool pk_matches = false;
                            if (std::holds_alternative<int>(pk_val) && static_cast<uint64_t>(std::get<int>(pk_val)) == pk_lookup) pk_matches = true;
                            else if (std::holds_alternative<bool>(pk_val) && static_cast<uint64_t>(std::get<bool>(pk_val)) == pk_lookup) pk_matches = true;
                            if (pk_matches) {
                                Data predicate_value = std::monostate{};
                                const size_t pred_off = column_offsets[predicate_column_index];
                                if (row_start + pred_off + get_data_size(table.columns[predicate_column_index].type)
                                    <= SLOT_SPACE) {
                                    predicate_value = read_data(table.columns[predicate_column_index].type,
                                                                  page.data + row_start + pred_off);
                                }
                                if (std::visit([&](auto &typed_pred) { return typed_pred.eval(predicate_value); },
                                               pred)) {
                                    for (size_t ci = 0; ci < selected_indices.size(); ci++) {
                                        const size_t off = column_offsets[selected_indices[ci]];
                                        if (row_start + off + get_data_size(table.columns[selected_indices[ci]].type)
                                            > SLOT_SPACE) {
                                            continue;
                                        }
                                        Data value = read_data(table.columns[selected_indices[ci]].type,
                                                               page.data + row_start + off);
                                        print_value(value);
                                    }
                                    std::cout << '\n';
                                    return;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
            }
        }
    }

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

size_t MemoryLayer::remove(Table &table, VariablePredicate &pred) {
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

    std::vector<size_t> column_offsets(table.columns.size());
    size_t running_offset = 0;
    for (size_t col_index = 0; col_index < table.columns.size(); col_index++) {
        column_offsets[col_index] = running_offset;
        running_offset += get_data_size(table.columns[col_index].type);
    }

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

    const size_t max_occupancy = std::min(MAX_OCCUPANCY, SLOT_SPACE / table.row_size);

    uint64_t pk_lookup = 0;
    if (has_predicate && pk_equality_fast_path(table, pred, pk_lookup)) {
        if (table.primary_key_btree->contains(pk_lookup)) {
            try {
                const uint64_t packed = table.primary_key_btree->search(pk_lookup);
                size_t page_idx = 0;
                size_t slot_idx = 0;
                unpack_row_locator(packed, page_idx, slot_idx);
                if (page_idx < table.pages && slot_idx < max_occupancy) {
                    LogicalPage page = PageIO::read_page(table.name, page_idx);
                    if (page.occupancy[slot_idx]) {
                        const size_t row_start = slot_idx * table.row_size;
                        if (row_start + table.row_size <= SLOT_SPACE) {
                            Data pk_val = read_data(table.columns[table.get_primary_key_index()].type, page.data + row_start + column_offsets[table.get_primary_key_index()]);
                            bool pk_matches = false;
                            if (std::holds_alternative<int>(pk_val) && static_cast<uint64_t>(std::get<int>(pk_val)) == pk_lookup) pk_matches = true;
                            else if (std::holds_alternative<bool>(pk_val) && static_cast<uint64_t>(std::get<bool>(pk_val)) == pk_lookup) pk_matches = true;
                            if (pk_matches) {
                                Data predicate_value = std::monostate{};
                                const size_t pred_off = column_offsets[predicate_column_index];
                                if (row_start + pred_off + get_data_size(table.columns[predicate_column_index].type)
                                    <= SLOT_SPACE) {
                                    predicate_value = read_data(table.columns[predicate_column_index].type,
                                                                  page.data + row_start + pred_off);
                                }
                                if (std::visit([&](auto &typed_pred) { return typed_pred.eval(predicate_value); },
                                               pred)) {
                                    table.primary_key_btree->remove(pk_lookup);
                                    page.occupancy[slot_idx] = 0;
                                    PageIO::write_page(table.name, page_idx, page);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
            }
        }
    }

    size_t removed_rows = 0;
    for (size_t page_index{0}; page_index < table.pages; page_index++) {
        LogicalPage page = PageIO::read_page(table.name, page_index);
        bool page_changed = false;
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

            if (table.primary_key_btree && !table.columns.empty() && table.columns[0].type == DataType::INT) {
                Data pk_data = read_data(table.columns[0].type, page.data + row_start + column_offsets[0]);
                if (std::holds_alternative<int>(pk_data)) {
                    table.primary_key_btree->remove(static_cast<uint64_t>(std::get<int>(pk_data)));
                }
            }

            page.occupancy[slot] = 0;
            page_changed = true;
            removed_rows++;
        }
        if (page_changed) {
            PageIO::write_page(table.name, page_index, page);
        }
    }
    return removed_rows;
}
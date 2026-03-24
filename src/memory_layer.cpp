#include "memory_layer.hpp"

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

MemoryLayer &MemoryLayer::instance() {
    static MemoryLayer memory_layer;
    return memory_layer;
}
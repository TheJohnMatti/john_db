#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <memory>
#include "column.hpp"
#include "b_tree.hpp"

struct Table {
    Table() : name{"unnamed_table"} {}
    Table(std::string name)
        : name{std::move(name)} {}
    std::string name;
    std::string table_dir;
    std::vector<Column> columns;
    std::unordered_map<std::string, size_t> column_index;
    size_t row_size = 0, pages = 0;
    std::unique_ptr<BTree> primary_key_btree;
    void add_column(DataType col_type, std::string &&col_name, bool is_index=false, bool is_primary=false) {
        size_t index = columns.size();
        columns.push_back(Column{std::move(col_name), col_type, is_index, is_primary});
        column_index[columns.back().name] = index;
        row_size += get_data_size(col_type);
    }
    bool has_column(const std::string &column_name) const {
        return column_index.count(column_name);
    }
    Column& get_column(const std::string &name) {
        return columns[column_index.at(name)];
    }
    size_t get_column_index(const std::string &name) const {
        return column_index.at(name);
    }
    size_t get_primary_key_index() const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].is_primary) return i;
        }
        return 0; // fallback
    }
    void write_table_metadata() {
        const std::filesystem::path path = std::filesystem::path("tables") / name;
        if (!std::filesystem::is_directory(path)) {
            std::filesystem::create_directory(path);
        }
        const std::filesystem::path metadata_path = path / "metadata.data";
        std::ofstream output_file{metadata_path};
        output_file << columns.size() << '\n';
        std::vector<std::string> indexes;
        for (const Column& col : columns) {
            output_file << data_to_char[(size_t)col.type] << ' ' << col.name << '\n';
            if (col.is_index) indexes.push_back(col.name);
        }
        output_file << indexes.size() << '\n';
        for (auto &i : indexes) output_file << i << '\n';
        output_file << pages << '\n';
    }
    static Table parse_table_metadata(const std::filesystem::directory_entry &dir) {
        Table table{ dir.path().filename().string() };
        const std::filesystem::path metadata_path = dir.path() / "metadata.data";
        std::ifstream input_file{metadata_path};
        if (!input_file.is_open()) {
            std::cerr << "Unable to open metadata at: " << metadata_path.string() << std::endl;
            return table;
        }
        int column_count;
        input_file >> column_count;
        for (int i = 0; i < column_count; i++){
            char col_type;
            std::string col_name;
            input_file >> col_type >> col_name;
            bool is_primary = (i == 0);
            table.add_column(char_to_data[col_type], std::move(col_name), false, is_primary);
        }
        int index_count;
        input_file >> index_count;
        for (int i = 0; i < index_count; i++) {
            std::string column_name;
            input_file >> column_name;
            if (table.has_column(column_name))
                table.get_column(column_name).is_index = true;
        }
        int page_count;
        input_file >> page_count;
        table.pages = page_count;
        return table;
    }
};
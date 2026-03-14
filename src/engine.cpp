#include <iostream>
#include <fstream>
#include "engine.hpp"

Engine::Engine() {
    char_to_type['i'] = INT;
    char_to_type['s'] = VARCHAR;
    char_to_type['b'] = BOOL;
};

Engine& Engine::instance() {
    static Engine inst;
    return inst;
}

void Engine::read_tables_folder() {
    std::filesystem::path path{".\\tables"};
    std::filesystem::directory_iterator dir{path};
    for (auto &i : dir) {
        std::cout << i << std::endl;
        tables.push_back(parse_table_metadata(i));
    }
}

Table Engine::parse_table_metadata(std::filesystem::directory_entry dir) {
    Table table{dir.path().filename().string()};
    std::filesystem::path metadata_path{dir.path().string() + "\\metadata.data"};
    std::ifstream input_file(metadata_path);
    if (input_file.is_open()) {
        int columns;
        input_file >> columns;
        for (int i{}; i < columns; i++) {
            char col_type; 
            std::string col_name;
            input_file >> col_type >> col_name;
            table.add_column(char_to_type[col_type], std::move(col_name));
            //std::cout << table.columns.back().name << ' ' << table.columns.back().type << std::endl;
        }
        int indexes;
        input_file >> indexes;
        for (int i{}; i < indexes; i++) {
            int column_idx;
            input_file >> column_idx;
            table.columns[column_idx].is_index = 1;
        }
    } else {
        std::cerr << "Unable to open metadata at: " << metadata_path.string() << std::endl;
    }

    return table;
}
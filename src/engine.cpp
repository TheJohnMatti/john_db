#include <iostream>
#include <fstream>
#include "engine.hpp"

Engine::Engine() {};

void Engine::init() {
    read_tables_folder();
};

Engine& Engine::instance() {
    static Engine inst;
    return inst;
}

void Engine::read_tables_folder() {
    std::filesystem::path path{ ".\\tables" };
    std::filesystem::directory_iterator dir{path};
    for (auto &i : dir) {
        std::cout << i.path().filename().string() << std::endl;
        tables[i.path().filename().string()] = parse_table_metadata(i);
    }
}

void Engine::write_table_metadata(Table& table) {
    std::filesystem::path path{ ".\\tables\\" + table.name };
    if (!std::filesystem::is_directory(path)) {
        std::filesystem::create_directory(path);
    }
    std::filesystem::directory_entry dir{ path };
    std::filesystem::path metadata_path{dir.path().string() + "\\metadata.data"};
    std::ofstream output_file{ metadata_path };
    std::vector<int> indexes;
    output_file << table.columns.size() << '\n';
    for (int i{}; i < table.columns.size(); i++) {
        Column &column = table.columns[i];
        if (column.is_index) indexes.push_back(i);
        output_file << data_to_char[(size_t)column.type] << ' ' << column.name << '\n';
    }
    output_file << indexes.size() << '\n';
    for (auto &i : indexes) output_file << i << '\n';
}

Table Engine::parse_table_metadata(std::filesystem::directory_entry dir) {
    Table table{dir.path().filename().string()};
    std::filesystem::path metadata_path{dir.path().string() + "\\metadata.data"};
    std::ifstream input_file{ metadata_path };
    if (input_file.is_open()) {
        int columns;
        input_file >> columns;
        for (int i{}; i < columns; i++) {
            char col_type; 
            std::string col_name;
            input_file >> col_type >> col_name;
            table.add_column(char_to_data[col_type], std::move(col_name));
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
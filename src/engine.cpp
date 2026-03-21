#include <iostream>
#include <fstream>
#include "engine.hpp"
#include "error.hpp"
#include "predicate.hpp"

Engine::Engine() {};

void Engine::init() {
    read_tables_folder();
    for (auto &i : tables) write_table_metadata(i.second);
};

Engine& Engine::instance() {
    static Engine inst;
    return inst;
}

void Engine::read_tables_folder() {
    std::filesystem::path path{ "tables" };
    std::filesystem::directory_iterator dir{path};
    for (auto &i : dir) {
        std::cout << i.path().filename().string() << std::endl;
        tables[i.path().filename().string()] = parse_table_metadata(i);
    }
}

void Engine::write_table_metadata(const Table& table) {
    std::filesystem::path path{ "tables/" + table.name };
    if (!std::filesystem::is_directory(path)) std::filesystem::create_directory(path);
    std::filesystem::path metadata_path{ path.string() + "/metadata.data" };
    std::ofstream output_file{ metadata_path };
    output_file << table.columns.size() << '\n';
    std::vector<std::string> indexes;
    for (const Column& col : table.columns) {
        output_file << data_to_char[(size_t)col.type] << ' ' << col.name << '\n';
        if (col.is_index) indexes.push_back(col.name);
    }
    output_file << indexes.size() << '\n';
    for (auto &i : indexes) output_file << i << '\n';
    output_file << table.pages << '\n';
}

Table Engine::parse_table_metadata(const std::filesystem::directory_entry &dir)
{
    Table table{ dir.path().filename().string() };
    std::filesystem::path metadata_path{ dir.path().string() + "/metadata.data" };
    std::ifstream input_file{ metadata_path };
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

void Engine::run(Query& query) {
    if (query.empty()) return;
    try {
        auto first_token = query[0];
        switch (first_token.type) {
            case TokenType::SELECT:
                run_select(query);
                break;
            case TokenType::CREATE:
                run_create(query);
                break;
            case TokenType::INSERT:
                //run_insert(query);
                break;
            case TokenType::DELETE:
                //run_delete(query);
                break;    
            default:
                throw SyntaxError("No handler for first token with type: " + std::to_string((int)first_token.type));
        }
    } catch (SyntaxError error) {
        std::cerr << "Syntax error: " << error.what() << std::endl;
        return;
    } catch (ReferenceError error) {
        std::cerr << "Reference error: " << error.what() << std::endl;
        return;
    }

};

QueryResult Engine::run_select(Query& query) {
    bool all_cols = false;
    std::vector<std::string> columns;
    int i = 2;

    if (query.size() >= 2 && query[1].type == TokenType::ASTERISK) {
        all_cols = true;
    }
    if (!all_cols) {
        if (query.size() < 2 || query[1].type != TokenType::IDENTIFIER) throw SyntaxError("No columns provided");
        columns = {std::get<std::string>(query[1].data)};
        for (; i < query.size(); i++) {
            if (i % 2 == 0) {
                if (query[i].type == TokenType::FROM) break;
                if (query[i].type != TokenType::COMMA) throw SyntaxError("Bad columns");
            } else {
                if (query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Bad columns");
                columns.push_back(std::get<std::string>(query[i].data));
            }
        }
    }
    if (i >= query.size() || query[i].type != TokenType::FROM) throw SyntaxError("Table not specified");
    i++;
    if (i >= query.size() || query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Table not specified");
    std::string &table_name = std::get<std::string>(query[i].data);
    auto it = tables.find(table_name);
    if (it == tables.end()) throw ReferenceError("Table " + table_name + " does not exist");
    Table &table = it->second;
    if (all_cols) {
        for (auto &col : table.columns) columns.push_back(col.name);
    } else {
        for (auto &col : columns) {
            if (!table.has_column(col)) throw ReferenceError("Column " + col + " does not exist in table " + table.name);
        }
    }
    i++;
    if (i < query.size()) {
        if (query[i++].type != TokenType::WHERE) throw SyntaxError("Expected where clause");
        if (i >= query.size()) throw SyntaxError("Expected identifier");
        Token &arg1 = query[i++];
        if (i >= query.size()) throw SyntaxError("Expected operator");
        Token &op = query[i++];
        if (i >= query.size()) throw SyntaxError("Expected literal");
        Token& arg2 = query[i++];
        if (i < query.size() && query[i].type != TokenType::SEMICOLON) throw SyntaxError("Expected semicolon");
        if (++i < query.size()) throw SyntaxError("Expected termination");
        auto predicate = [&]() -> std::variant<Predicate<int>, Predicate<std::string>, Predicate<bool>, Predicate<double>> {
            switch (arg2.data.index()) {
                case 1:
                    return Predicate<int>(arg1, op, arg2);
                case 2:
                    return Predicate<std::string>(arg1, op, arg2);
                case 3:
                    return Predicate<bool>(arg1, op, arg2);
                case 4:
                    return Predicate<double>(arg1, op, arg2);
                default:
                    return Predicate<int>(arg1, op, arg2);
            }
        }();



    } else {

    }

    return QueryResult();

}

void Engine::run_create(Query &query) {
    int i = 1;
    if (i >= query.size() || query[i++].type != TokenType::TABLE) throw SyntaxError("Expected TABLE");
    if (i >= query.size() || query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected table name");
    const std::string &table_name = std::get<std::string>(query[i++].data);
    if (tables.count(table_name)) throw ReferenceError("Table " + table_name + " already exists");
    Table new_table{table_name};
    if (i >= query.size() || query[i++].type != TokenType::OPEN_PAREN) throw SyntaxError("Expected open parentheses");
    if (i >= query.size() || query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected column name");
    std::string &primary_key_name = std::get<std::string>(query[i++].data);
    if (i >= query.size() || !is_type(query[i].type)) throw SyntaxError("Expected type");
    const TokenType primary_key_type = query[i++].type;
    if (i >= query.size()) throw SyntaxError("Expected closing parantheses");
    std::string cur_name = std::move(primary_key_name); DataType cur_type = token_to_type(primary_key_type);
    for (; i < query.size(); i++) {
        if (i % 3 == 0) {
            new_table.add_column(cur_type, std::move(cur_name), i == 6, i == 6);
            if (query[i].type == TokenType::CLOSE_PAREN) {
                i++; 
                break;
            }
            if (query[i].type != TokenType::COMMA) throw SyntaxError("Bad columns");
        } else if (i % 3 == 1) {
            if (query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected column name");
            cur_name = std::move(std::get<std::string>(query[i].data));
        } else {
            if (!is_type(query[i].type)) throw SyntaxError("Expected type");
            cur_type = token_to_type(query[i].type);
        }
    }
    if (query[i-1].type != TokenType::CLOSE_PAREN) throw SyntaxError("Bad columns");
    if (i < query.size() && query[i++].type != TokenType::SEMICOLON) throw SyntaxError("Expected termination");
    if (i < query.size()) throw SyntaxError("Expected termination");
    tables.insert({new_table.name, new_table});
    write_table_metadata(new_table);
};
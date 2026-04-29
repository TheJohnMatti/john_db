#include <iostream>
#include <fstream>
#include <algorithm>
#include "engine.hpp"
#include "error.hpp"
#include "predicate.hpp"

Engine::Engine() {};

void Engine::init() {
    read_tables_folder();
    for (auto &i : tables) i.second.write_table_metadata();
};

Engine& Engine::instance() {
    static Engine inst;
    return inst;
}

void Engine::read_tables_folder() {
    if (!std::filesystem::is_directory("tables")) std::filesystem::create_directory("tables");
    std::filesystem::path path{ "tables" };
    std::filesystem::directory_iterator dir{ path };
    for (auto &i : dir) {
        std::cout << i.path().filename().string() << std::endl;
        tables[i.path().filename().string()] = Table::parse_table_metadata(i);
    }
}

void Engine::run(Query& query) {
    if (query.empty()) return;
    try {
        auto &first_token = query[0];
        switch (first_token.type) {
            case TokenType::SELECT:
                run_select(query);
                break;
            case TokenType::CREATE:
                run_create(query);
                break;
            case TokenType::INSERT:
                run_insert(query);
                break;
            case TokenType::DELETE:
                //run_delete(query);
                break;    
            default:
                throw SyntaxError("No handler for first token with type: " + std::to_string((int)first_token.type));
        }
    } catch (const SyntaxError &error) {
        std::cerr << "Syntax error: " << error.what() << std::endl;
        return;
    } catch (const ReferenceError &error) {
        std::cerr << "Reference error: " << error.what() << std::endl;
        return;
    } catch (const LogicError &error) {
        std::cerr << "Logic error: " << error.what() << std::endl;
        return;
    } catch (const TypeError &error) {
        std::cerr << "Type error: " << error.what() << std::endl;
        return;
    }

};

void Engine::run_select(Query& query) {
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
    if (i < query.size() && query[i].type != TokenType::SEMICOLON) {
        if (query[i++].type != TokenType::WHERE) throw SyntaxError("Expected where clause");
        if (i >= query.size() || !is_identifier(query[i].type)) throw SyntaxError("Expected identifier");
        Token &arg1 = query[i++];
        if (i >= query.size() || !is_operator(query[i].type)) throw SyntaxError("Expected operator");
        Token &op = query[i++];
        if (i >= query.size() || !is_literal(query[i].type)) throw SyntaxError("Expected literal");
        Token& arg2 = query[i++];
        if (i < query.size() && query[i].type != TokenType::SEMICOLON) throw SyntaxError("Expected semicolon");
        if (++i < query.size()) throw SyntaxError("Expected termination");
        auto predicate = [&]() -> VariablePredicate {
            switch (arg2.data.index()) {
                case 1:
                    return Predicate<std::string>(arg1, op, arg2);
                case 2:
                    return Predicate<int>(arg1, op, arg2);
                case 3:
                    return Predicate<bool>(arg1, op, arg2);
                case 4:
                    return Predicate<double>(arg1, op, arg2);
                default:
                    return Predicate<int>(arg1, op, arg2);
            }
        }();
        memory_layer.select(table, columns, predicate);
    } else {
        if (i < query.size() && query[i].type != TokenType::SEMICOLON) throw SyntaxError("Expected semicolon");
        if (++i < query.size()) throw SyntaxError("Expected termination");
        VariablePredicate predicate = NullPredicate{};
        memory_layer.select(table, columns, predicate);
    }

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
    tables.insert({ new_table.name, std::move(new_table) });
    new_table.write_table_metadata();
};

void Engine::run_insert(Query &query) {
    int i = 1;
    if (i >= query.size() || query[i++].type != TokenType::INTO) throw SyntaxError("Expected 'INTO'");
    if (i >= query.size() || query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected table name");
    std::string &table_name = std::get<std::string>(query[i].data);
    if (!tables.count(table_name)) throw ReferenceError("Table " + table_name + " does not exist");
    Table &target_table = tables[table_name];
    std::vector<bool> active_index(target_table.columns.size());
    std::vector<int> index_order;
    i++;
    if (i >= query.size() || query[i++].type != TokenType::OPEN_PAREN) throw SyntaxError("Expected opening parentheses");
    if (i >= query.size() || query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected column name");
    std::string &first_column = std::get<std::string>(query[i].data);
    if (!target_table.column_index.count(first_column)) throw ReferenceError("Column " + first_column + " does not exist in table " + table_name);
    int idx = target_table.column_index[first_column];
    active_index[idx] = true;
    index_order.push_back(idx);
    i++;
    int parity = i % 2;
    for (; i < query.size(); i++) {
        if (i % 2 == parity) {
            if (query[i].type == TokenType::CLOSE_PAREN) break;
            if (query[i].type != TokenType::COMMA) throw SyntaxError("Expected comma or closing parentheses");
        } else {
            if (query[i].type != TokenType::IDENTIFIER) throw SyntaxError("Expected column name");
            std::string &column_name = std::get<std::string>(query[i].data);
            if (!target_table.column_index.count(column_name)) throw ReferenceError("Column " + first_column + " does not exist in table " + table_name);
            int idx = target_table.column_index[column_name];
            if (active_index[idx]) throw LogicError("Cannot duplicate column " + column_name);
            active_index[idx] = true;
            index_order.push_back(idx);
        }
    }
    if (i++ == query.size()) throw SyntaxError("Expected closing parentheses");
    // TODO: remove when null enabled
    for (auto i : active_index) if (i == false) throw LogicError("All columns must be specified");
    if (i >= query.size() || query[i++].type != TokenType::VALUES) throw SyntaxError("Expected 'VALUES'");
    if (i >= query.size() || query[i++].type != TokenType::OPEN_PAREN) throw SyntaxError("Expected opening parentheses");
    std::vector<Data> values;
    if (i >= query.size() || !is_literal(query[i].type)) throw SyntaxError("Expected literal");
    if (data_index_to_data_type(query[i].data) != target_table.columns[index_order[0]].type) throw TypeError("Wrong type for value at index 0");
    values.push_back(query[i].data);
    i++;
    int starting_index = i-1;
    parity = i % 2;
    for (; i < query.size(); i++) {
        if (i % 2 == parity) {
            if (query[i].type == TokenType::CLOSE_PAREN) break;
            if (query[i].type != TokenType::COMMA) throw SyntaxError("Expected comma or closing parentheses");
        } else {
            if (!is_literal(query[i].type)) throw SyntaxError("Expected literal");
            int idx = (i-starting_index)/2;
            if (data_index_to_data_type(query[i].data) != target_table.columns[index_order[idx]].type) throw TypeError("Wrong type for value at index " + std::to_string(idx));
            values.push_back(query[i].data);
        }
    }
    if (i >= query.size() || query[i++].type != TokenType::CLOSE_PAREN) throw SyntaxError("Expected closing parentheses");
    if (i < query.size() && query[i++].type != TokenType::SEMICOLON) throw SyntaxError("Expected semicolon");
    if (i < query.size()) throw SyntaxError("Expected termination");
    std::vector<Data> ordered_columns(active_index.size());
    for (int i{}; i < active_index.size(); i++) {
        ordered_columns[index_order[i]] = values[i];
    }
    try {
        memory_layer.insert(target_table, ordered_columns);
    } catch (const std::runtime_error& e) {
        std::cerr << "Insert failed: " << e.what() << std::endl;
    }
}
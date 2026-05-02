#include "memory_layer.hpp"
#include "table.hpp"
#include "data_type.hpp"
#include "predicate.hpp"
#include "token.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

int main() {
    std::filesystem::remove_all("test_data");

    MemoryLayer memory_layer("test_data");

    // Create table
    Table table("users");
    table.add_column(DataType::INT, "id", true, true); // pk
    table.add_column(DataType::VARCHAR_16, "name", false, false);
    table.add_column(DataType::BOOL, "active", false, false);
    memory_layer.create_table(table);

    // Test INSERT
    std::vector<Data> row1 = {Data(1), Data("Alice"), Data(true)};
    memory_layer.insert(table, row1);

    std::vector<Data> row2 = {Data(2), Data("Bob"), Data(false)};
    memory_layer.insert(table, row2);

    // Test SELECT all
    std::vector<std::string> cols = {"id", "name", "active"};
    VariablePredicate pred = NullPredicate{};
    memory_layer.select(table, cols, pred);

    // Test SELECT with WHERE id = 1
    Token arg1(TokenType::IDENTIFIER, "id");
    Token op(TokenType::EQUAL, "=");
    Token arg2(TokenType::INT_LITERAL, 1);
    Predicate<int> p(arg1, op, arg2);
    VariablePredicate pred_where = p;
    memory_layer.select(table, cols, pred_where);

    // Test DELETE WHERE id = 2
    Token arg1_del(TokenType::IDENTIFIER, "id");
    Token op_del(TokenType::EQUAL, "=");
    Token arg2_del(TokenType::INT_LITERAL, 2);
    Predicate<int> p_del(arg1_del, op_del, arg2_del);
    VariablePredicate pred_del = p_del;
    size_t deleted = memory_layer.remove(table, pred_del);
    assert(deleted == 1);

    // Test SELECT after delete
    pred = NullPredicate{};
    memory_layer.select(table, cols, pred);

    std::cout << "memory_layer.test.cpp: All tests passed" << std::endl;
    return 0;
}
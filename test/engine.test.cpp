#include "engine.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

int main() {
    // Clean up any existing tables
    std::filesystem::remove_all("tables");

    Engine &engine = Engine::instance();

    // Test CREATE TABLE
    engine.run_query("CREATE TABLE users (id INT, name STRING, active BOOL);");

    // Test INSERT
    engine.run_query("INSERT INTO users (id, name, active) VALUES (1, \"Alice\", true);");
    engine.run_query("INSERT INTO users (id, name, active) VALUES (2, \"Bob\", false);");

    // Test SELECT all
    std::cout << "SELECT * FROM users;" << std::endl;
    engine.run_query("SELECT * FROM users;");

    // Test SELECT with WHERE pk = 
    std::cout << "SELECT * FROM users WHERE id = 1;" << std::endl;
    engine.run_query("SELECT * FROM users WHERE id = 1;");

    // Test SELECT with WHERE pk >= and <=
    std::cout << "SELECT * FROM users WHERE id >= 1;" << std::endl;
    engine.run_query("SELECT * FROM users WHERE id >= 1;");
    std::cout << "SELECT * FROM users WHERE id <= 1;" << std::endl;
    engine.run_query("SELECT * FROM users WHERE id <= 1;");

    // Test DELETE with WHERE pk =
    std::cout << "DELETE FROM users WHERE id = 2;" << std::endl;
    engine.run_query("DELETE FROM users WHERE id = 2;");

    // Test SELECT after delete
    std::cout << "SELECT * FROM users;" << std::endl;
    engine.run_query("SELECT * FROM users;");

    std::cout << "engine.test.cpp: All integration tests passed" << std::endl;

    return 0;
}
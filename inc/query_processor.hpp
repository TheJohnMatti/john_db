#pragma once

#include <iostream>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <utility>
#include <bitset>
#include "engine.hpp"
#include "tokens.hpp"

using QueryString = std::vector<std::string_view>;
using Query = std::vector<Token>;
class ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class TokenizeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class SyntaxError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

enum HandlerEnum {
    SELECT = 0,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    ALTER,
    DROP,
};

const std::unordered_map<TokenType, HandlerEnum> first_token_to_handler = {
    {TokenType::SELECT, HandlerEnum::SELECT},
    {TokenType::INSERT, HandlerEnum::INSERT},
    {TokenType::UPDATE, HandlerEnum::UPDATE},
    {TokenType::DELETE, HandlerEnum::DELETE},
    {TokenType::CREATE, HandlerEnum::CREATE},
    {TokenType::ALTER, HandlerEnum::ALTER},
    {TokenType::DROP, HandlerEnum::DROP},
};

static constexpr std::array special = {'(', ')', ',', '=', '>', '<', ';'};
constexpr auto special_characters = []{
    std::bitset<256> b{};
    for (auto c : special) b[c] = 1;
    return b;
}();

class QueryProcessor {

    private:
    void select_handler(Query&), insert_handler(Query&), update_handler(Query&), delete_handler(Query&),
    create_handler(Query&), alter_handler(Query&), drop_handler(Query&), unknown_handler(Query&);
    Engine &engine = Engine::instance();
    QueryString get_token_strings(std::string_view);
    Query get_tokens(QueryString);
    Token to_token(std::string_view);

    public:
    QueryProcessor();
    void process(std::string_view);
    static bool is_string(std::string_view);
    static bool is_int(std::string_view);
    static bool is_float(std::string_view);
    static bool is_identifier(std::string_view);


};
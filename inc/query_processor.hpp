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
#include "token.hpp"
#include "query_result.hpp"

using QueryString = std::vector<std::string_view>;
using Query = std::vector<Token>;

static constexpr std::array special = {'(', ')', ',', '=', '>', '<', ';'};
constexpr auto special_characters = []{
    std::bitset<256> b{};
    for (auto c : special) b[c] = 1;
    return b;
}();

class QueryProcessor {

    private:
    Engine &engine = Engine::instance();
    QueryString get_token_strings(std::string_view);
    Query get_tokens(QueryString);
    Token to_token(std::string_view);

    public:
    QueryProcessor();
    Query process(std::string_view);
    static bool is_string(std::string_view);
    static bool is_int(std::string_view);
    static bool is_float(std::string_view);
    static bool is_identifier(std::string_view);


};
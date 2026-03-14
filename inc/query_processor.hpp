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

using Query = std::vector<std::string_view>;

enum HandlerEnum {
    SELECT = 0,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    ALTER,
    DROP,
};

const std::unordered_map<std::string, HandlerEnum> FIRST_TO_HANDLER = {
    {"SELECT", SELECT},
    {"INSERT", INSERT},
    {"UPDATE", UPDATE},
    {"DELETE", DELETE},
    {"CREATE", CREATE},
    {"ALTER", ALTER},
    {"DROP", DROP}
};

class QueryProcessor {

    private:
    Query get_tokens(std::string_view);
    void select_handler(Query), insert_handler(Query), update_handler(Query), delete_handler(Query),
    create_handler(Query), alter_handler(Query), drop_handler(Query), unknown_handler(Query);
    Engine &engine = Engine::instance();
    std::bitset<256> special_characters;

    public:
    QueryProcessor();
    void process(std::string_view query);


};
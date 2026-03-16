#pragma once

#include <string>
#include <unordered_map>
#include "data_type.hpp"

enum class TokenType {
    UNDEFINED = 0,
    SELECT, // keywords start
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    ALTER,
    TABLE,
    DROP,
    FROM,
    INTO, 
    AS,
    ORDER,
    BY,
    DESC,
    ASC,
    HAVING,
    COUNT,
    SUM,
    AVG,
    MAX,
    MIN,
    WHERE,
    STRING, // literals start
    INT,
    FLOAT,
    COLON, // special characters start
    SEMICOLON,
    COMMA, 
    GREATERTHAN,
    EQUAL,
    LESSTHAN,
    ASTERISK,
    IDENTIFIER, // identifiers start
};

struct Token {
    TokenType type;
    Data data;
    Token() : type{TokenType::UNDEFINED}, data{std::monostate()} {}
    Token(TokenType type) : type{type}, data{std::monostate()} {}
    Token(TokenType type, Data data) : type{type}, data{data} {}
};

const std::unordered_map<std::string, TokenType> keyword_to_token = {
    {"SELECT", TokenType::SELECT},
    {"INSERT", TokenType::INSERT},
    {"UPDATE", TokenType::UPDATE},
    {"DELETE", TokenType::DELETE},
    {"CREATE", TokenType::CREATE},
    {"ALTER", TokenType::ALTER},
    {"TABLE", TokenType::TABLE},
    {"DROP", TokenType::DROP},
    {"FROM", TokenType::FROM},
    {"INTO", TokenType::INTO},
    {"AS", TokenType::AS},
    {"ORDER", TokenType::ORDER},
    {"BY", TokenType::BY},
    {"DESC", TokenType::DESC},
    {"ASC", TokenType::ASC},
    {"HAVING", TokenType::HAVING},
    {"COUNT", TokenType::COUNT},
    {"SUM", TokenType::SUM},
    {"AVG", TokenType::AVG},
    {"MAX", TokenType::MAX},
    {"MIN", TokenType::MIN},
    {"WHERE", TokenType::WHERE},
};

// TODO: refactor to 256 long array with constexpr lambda
const std::unordered_map<char, TokenType> special_char_to_token = {
    {':', TokenType::COLON},
    {';', TokenType::SEMICOLON},
    {',', TokenType::COMMA},
    {'>', TokenType::GREATERTHAN},
    {'=', TokenType::EQUAL},
    {'<', TokenType::LESSTHAN},
    {'*', TokenType::ASTERISK},
};

static const std::unordered_set<TokenType> operators {
    TokenType::LESSTHAN,
    TokenType::EQUAL,
    TokenType::GREATERTHAN,
};

static const std::unordered_set<TokenType> literals {
    TokenType::STRING,
    TokenType::INT,
    TokenType::FLOAT,
};

inline bool is_operator(TokenType token_type) {
    return operators.count(token_type);
}

inline bool is_literal(TokenType token_type) {
    return literals.count(token_type);
}

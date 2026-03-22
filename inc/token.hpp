#pragma once

#include <string>
#include <unordered_map>
#include "data_type.hpp"
#include "error.hpp"

enum class TokenType {
    UNDEFINED = 0,

    // keywords start
    SELECT, 
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
    STRING,
    INT,
    FLOAT,

    // literals start
    STRING_LITERAL,
    INT_LITERAL,
    FLOAT_LITERAL,
    
    // special characters start
    COLON,
    SEMICOLON,
    COMMA, 
    GREATERTHAN,
    EQUAL,
    LESSTHAN,
    ASTERISK,
    OPEN_PAREN,
    CLOSE_PAREN,

    // identifiers start
    IDENTIFIER, 
};

struct Token {
    TokenType type;
    Data data;
    Token() : type{TokenType::UNDEFINED}, data{std::monostate{}} {}
    Token(TokenType type) : type{type}, data{std::monostate{}} {}
    Token(TokenType type, Data data) : type{type}, data{data} {}
};

inline const std::unordered_map<std::string, TokenType> keyword_to_token = {
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
    {"STRING", TokenType::STRING},
    {"INT", TokenType::INT},
    {"FLOAT", TokenType::FLOAT},
};

// TODO: refactor to 256 long array with constexpr lambda
inline const std::unordered_map<char, TokenType> special_char_to_token = {
    {':', TokenType::COLON},
    {';', TokenType::SEMICOLON},
    {',', TokenType::COMMA},
    {'>', TokenType::GREATERTHAN},
    {'=', TokenType::EQUAL},
    {'<', TokenType::LESSTHAN},
    {'*', TokenType::ASTERISK},
    {'(', TokenType::OPEN_PAREN},
    {')', TokenType::CLOSE_PAREN},
};

static constexpr std::array special = {'(', ')', ',', '=', '>', '<', ';', '*', ':'};
inline constexpr auto special_characters = []{
    std::bitset<256> b{};
    for (auto c : special) b[c] = 1;
    return b;
}();

static const std::unordered_set<TokenType> operators {
    TokenType::LESSTHAN,
    TokenType::EQUAL,
    TokenType::GREATERTHAN,
};

inline bool is_operator(TokenType token_type) {
    return operators.count(token_type);
}

static const std::unordered_set<TokenType> literals {
    TokenType::STRING_LITERAL,
    TokenType::INT_LITERAL,
    TokenType::FLOAT_LITERAL,
};

inline bool is_literal(TokenType token_type) {
    return literals.count(token_type);
}

static const std::unordered_set<TokenType> types {
    TokenType::STRING,
    TokenType::INT,
    TokenType::FLOAT,
};

inline bool is_type(TokenType token_type) {
    return types.count(token_type);
}


static const std::unordered_map<TokenType, DataType> __token_to_type = {
    {TokenType::STRING, DataType::VARCHAR_16},
    {TokenType::INT, DataType::INT},
    {TokenType::FLOAT, DataType::FLOAT},
};

inline DataType token_to_type(TokenType token_type) {
    auto it = __token_to_type.find(token_type);
    if (it == __token_to_type.end()) 
        throw ConversionError("Cannot convert token "
            + std::to_string((int)token_type) 
            + " to data type");
    return it->second;
}

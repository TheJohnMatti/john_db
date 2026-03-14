#include <string>
#include <variant>
#include <unordered_map>

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
    STRING, // literals start
    INT,
    FLOAT,
    COLON, // special characters start
    SEMICOLON,
    COMMA, 
    GREATERTHAN,
    EQUAL,
    LESSTHAN,
    IDENTIFIER, // identifiers start
};

using Data = std::variant<std::monostate, std::string, int, double>;

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
};

// TODO: refactor to 256 long array with constexpr lambda
const std::unordered_map<char, TokenType> special_char_to_token = {
    {':', TokenType::COLON},
    {';', TokenType::SEMICOLON},
    {',', TokenType::COMMA},
    {'>', TokenType::GREATERTHAN},
    {'=', TokenType::EQUAL},
    {'<', TokenType::LESSTHAN},
};
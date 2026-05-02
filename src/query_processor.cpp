#include "query_processor.hpp"
#include <utility>
#include <array>
#include <algorithm>
#include <cctype>
#include "error.hpp"

QueryProcessor::QueryProcessor() {};

QueryString QueryProcessor::get_token_strings(std::string_view query) {
    QueryString res;
    for (int i{}; i < query.size(); ) {
        if (query[i] == ' ') {
            i++; 
            continue;
        }
        if (query[i] == '"') {
            int j;
            for (j = i + 1; j < query.size() && query[j] != '"'; j++);
            if (j == query.size()) {
                throw ParseError("Unterminated string: " + (std::string)query.substr(i));
            }
            res.push_back(query.substr(i, j-i+1));
            i = j + 1;
            continue;
        }
        if (query[i] == '<' || query[i] == '>') {
            if (i + 1 < query.size() && query[i + 1] == '=') {
                res.push_back(query.substr(i, 2));
                i += 2;
                continue;
            }
        }
        if (special_characters[query[i]]) {
            res.push_back(query.substr(i++, 1));
            continue;
        }
        int j;
        for (j = i; j < query.size() && query[j] != ' ' && !special_characters[query[j]]; j++){}
        res.push_back(query.substr(i, j-i));
        i = (j < query.size() && special_characters[query[j]]) ? j : j+1;
    }
    return res;
}

Query QueryProcessor::get_tokens(QueryString token_strings) {
    Query tokens(token_strings.size());
    for (int i{}; i < token_strings.size(); i++) {
        tokens[i] = to_token(token_strings[i]);
    }
    return tokens;
}

Token QueryProcessor::to_token(std::string_view token_string) {
    if (token_string.size() == 1) {
        auto it = special_char_to_token.find(token_string[0]);
        if (it != special_char_to_token.end()) return Token(it->second);
    }
    std::string uppercase_token_string(token_string.begin(), token_string.end());
    std::transform(token_string.begin(), token_string.end(), uppercase_token_string.begin(), ::toupper);
    if (uppercase_token_string == "TRUE") return Token(TokenType::BOOL_LITERAL, true);
    if (uppercase_token_string == "FALSE") return Token(TokenType::BOOL_LITERAL, false);
    if (uppercase_token_string == ">=") return Token(TokenType::GREATERTHANOREQUAL);
    if (uppercase_token_string == "<=") return Token(TokenType::LESSTHANOREQUAL);
    auto it = keyword_to_token.find(uppercase_token_string);
    if (it != keyword_to_token.end()) 
        return Token(it->second);
    if (is_string(token_string)) {
        if (token_string.size() > MAX_STRING_SIZE + 2) throw TokenizeError("Strings longer than " + std::to_string(MAX_STRING_SIZE) + " are not supported.");
        return Token(TokenType::STRING_LITERAL, std::string(token_string.begin()+1, token_string.end()-1));
    }
    if (is_int(token_string)) 
        return Token(TokenType::INT_LITERAL, std::stoi(std::string(token_string)));
    if (is_float(token_string)) 
        return Token(TokenType::FLOAT_LITERAL, std::stod(std::string(token_string)));
    if (is_identifier(token_string))
        return Token(TokenType::IDENTIFIER, std::string(token_string));
    throw TokenizeError("Invalid identifier: " + std::string(token_string));
}

bool QueryProcessor::is_string(std::string_view token) {
    return token[0] == '"' && token.back() == '"';
}

bool QueryProcessor::is_int(std::string_view token) {
    size_t start_pos = (token[0] == '-' || token[0] == '+'); // skip leading sign
    for (auto i = start_pos; i < token.size(); i++) {
        if (!std::isdigit(token[i])) return false;
    }
    return token.size() > start_pos;
}

bool QueryProcessor::is_float(std::string_view token) {
    if (token.size() < 2) return false;
    size_t start_pos = (token[0] == '-' || token[0] == '+'); // skip leading sign
    auto i = start_pos;
    for (; i < token.size(); i++) {
        if (!std::isdigit(token[i])) {
            if (token[i++] == '.') break;
            else return false;
        }
    }
    for (; i < token.size(); i++) {
        if (!std::isdigit(token[i])) return false;
    }
    return token.size() > start_pos;
}

bool QueryProcessor::is_identifier(std::string_view token) {
    if (token.empty()) return false;
    if (!std::isalpha(token[0]) && token[0] != '_')
        return false;
    for (size_t i = 1; i < token.size(); ++i) {
        char c = token[i];
        if (!std::isalnum(c) && c != '_')
            return false;
    }
    return true;
}

Query QueryProcessor::process(std::string_view query) {
    QueryString token_strings;
    Query tokens;

    try {
        token_strings = get_token_strings(query);
    } catch (const ParseError &error) {
        std::cerr << error.what() << std::endl;
        return Query();
    }
    if (!token_strings.size()) return Query();

    try {
        tokens = get_tokens(token_strings);
    } catch (const TokenizeError &error) {
        std::cerr << error.what() << std::endl;
        return Query();
    }
    return tokens;
}
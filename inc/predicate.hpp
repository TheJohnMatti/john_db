#pragma once

#include "token.hpp"
#include "error.hpp"
#include <unordered_set>
#include <functional>

using PredicateFunc = std::function<bool(Data &el1, Data &el2)>;

template<typename T>
static PredicateFunc less_than = [](Data &el1, Data &el2) {
    return std::get<T>(el1) < std::get<T>(el2);
};

template<typename T>
static PredicateFunc less_than_or_equal = [](Data &el1, Data &el2) {
    return std::get<T>(el1) <= std::get<T>(el2);
};

template<typename T> 
static PredicateFunc greater_than = [](Data &el1, Data &el2) {
    return std::get<T>(el1) > std::get<T>(el2);
};

template<typename T>
static PredicateFunc greater_than_or_equal = [](Data &el1, Data &el2) {
    return std::get<T>(el1) >= std::get<T>(el2);
};

template<typename T>
static PredicateFunc equal_to = [](Data &el1, Data &el2) {
    return std::get<T>(el1) == std::get<T>(el2);
};

template<typename T>
struct Predicate {

    Token arg1, op, arg2;
    std::string target_column;
    PredicateFunc func;

    PredicateFunc get_predicate_func() {
        switch (op.type) {
            case TokenType::LESSTHAN:
                return less_than<T>;
            case TokenType::LESSTHANOREQUAL:
                return less_than_or_equal<T>;
            case TokenType::GREATERTHAN:
                return greater_than<T>;
            case TokenType::GREATERTHANOREQUAL:
                return greater_than_or_equal<T>;
            case TokenType::EQUAL:
                return equal_to<T>;
            default:
                return equal_to<T>;
        }
    }

    Predicate(Token &arg1, Token &op, Token &arg2) {
        if (arg1.type != TokenType::IDENTIFIER) throw SyntaxError("Arg 1 of predicate must be an identifier");
        if (!is_operator(op.type)) throw SyntaxError("Invalid operator");
        if (!is_literal(arg2.type)) throw SyntaxError("Arg 2 of predicate must be a literal");
        this->arg1 = arg1;
        this->op = op;
        this->arg2 = arg2;
        target_column = std::get<std::string>(arg1.data);
        func = get_predicate_func();
    }

    bool eval(Data &element) {
        return func(element, arg2.data);
    }

};

struct NullPredicate {
    bool eval(Data &) {
        return true;
    }
};

using VariablePredicate = std::variant<Predicate<int>, Predicate<std::string>, Predicate<bool>, Predicate<double>, NullPredicate>;
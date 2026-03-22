#pragma once

#include <iostream>

class ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class TokenizeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class SyntaxError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class ReferenceError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ConversionError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class LogicError : public std::runtime_error {
    using std::runtime_error::runtime_error;
}
#pragma once

#include "TokenType.hpp"
#include <any>
#include <string>
#include <variant>

using namespace std;

using Literal = variant<monostate, string, double>;

struct Token {
    TokenType type;
    string lexeme;
    Literal literal;
    int line;

    Token(TokenType type, string lexeme, Literal literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}
};

void printToken(const Token &token);

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

    string toString() const {
        string name(tokenTypeName(type));
        switch (type) {
        case TokenType::ident:
            return name + "(" + lexeme + ")";
        case TokenType::string:
            return name + "('" + (holds_alternative<string>(literal)
                                      ? get<string>(literal)
                                      : lexeme) +
                   "')";
        case TokenType::charcon:
            return name + "('" + (holds_alternative<string>(literal)
                                      ? get<string>(literal)
                                      : lexeme) +
                   "')";
        case TokenType::intcon:
            if (holds_alternative<double>(literal))
                return name + "(" +
                       std::to_string(
                           static_cast<long long>(get<double>(literal))) +
                       ")";
            return name + "(" + lexeme + ")";
        case TokenType::realcon:
            return name + "(" + lexeme + ")";
        case TokenType::unknown:
            return name + "(" + lexeme + ")";
        default:
            return name;
        }
    }
};

void printToken(const Token &token);

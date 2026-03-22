#pragma once

#include "Token.hpp"
#include "SourceBuffer.hpp"
#include <string>
#include <vector>

class Scanner {
    public:
        explicit Scanner(const std::string &path);
        std::vector<Token> scanTokens();

    private:
        SourceBuffer source;
        size_t start = 0;
        size_t current = 0;
        int line = 1;
        std::vector<Token> tokens;

        void scanToken();
        bool isAtEnd() const;
        char advance();
        char peek() const;
        char peekNext() const;
        bool match(char expected);

        void addToken(TokenType type, Literal literal = Literal{});
        void identifier();
        void number();
        void stringLiteral();

        static bool isAlpha(char c);
        static bool isAlphaNumeric(char c);
        static std::string toLower(std::string s);
};

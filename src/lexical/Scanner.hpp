#pragma once

#include "SourceBuffer.hpp"
#include "Token.hpp"
#include <string>
#include <vector>

struct PendingError {
    int line;
    std::string message;
};

class Scanner {
  public:
    explicit Scanner(const std::string &path);
    std::vector<Token> scanTokens();
    void printAllTokens();

  private:
    SourceBuffer source;
    size_t start = 0;
    size_t current = 0;
    int line = 1;
    std::vector<Token> tokens;
    std::vector<PendingError> errors;

    void scanToken();
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    void addToken(TokenType type, Literal literal = Literal{});
    void addError(int line, const std::string &message);
    void identifier();
    void number();
    void stringLiteral();

    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
    static std::string toLower(std::string s);
};

#include "Token.hpp"
#include "TokenType.hpp"

void printToken(const Token &token) {
    if (token.type == TokenType::eof_token) {
        return;
    }

    std::cout << tokenTypeName(token.type);

    if (token.type == TokenType::ident) {
        std::cout << " (" << token.lexeme << ")";
    } else if (token.type == TokenType::string ||
               token.type == TokenType::charcon) {
        if (std::holds_alternative<std::string>(token.literal)) {
            std::cout << " (" << std::get<std::string>(token.literal) << ")";
        }
    } else if (token.type == TokenType::intcon ||
               token.type == TokenType::realcon) {
        if (std::holds_alternative<double>(token.literal)) {
            const double value = std::get<double>(token.literal);
            if (token.type == TokenType::intcon) {
                std::cout << " (" << static_cast<long long>(value) << ")";
            } else {
                std::cout << " (" << value << ")";
            }
        }
    }

    std::cout << '\n';
}

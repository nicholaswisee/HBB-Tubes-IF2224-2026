#include "Scanner.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <unordered_map>

using namespace std;

Scanner::Scanner(const string &path) : source(path) {}

vector<Token> Scanner::scanTokens() {
    tokens.clear();
    start = 0;
    current = 0;
    line = 1;
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(Token{TokenType::eof_token, "", Literal{}, line});
    return tokens;
}

void Scanner::scanToken() {
    char c = advance();
    switch (c) {
    case '\0':
        return;
    case ' ':
    case '\r':
    case '\t':
        return;
    case '\n':
        line++;
        return;
    case '(':
        addToken(TokenType::lparent);
        return;
    case ')':
        addToken(TokenType::rparent);
        return;
    case '[':
        addToken(TokenType::lbrack);
        return;
    case ']':
        addToken(TokenType::rbrack);
        return;
    case ',':
        addToken(TokenType::comma);
        return;
    case ';':
        addToken(TokenType::semicolon);
        return;
    case '+':
        addToken(TokenType::plus);
        return;
    case '-':
        addToken(TokenType::minus);
        return;
    case '*':
        addToken(TokenType::times);
        return;
    case '.':
        addToken(TokenType::period);
        return;
    case ':':
        if (match('='))
            addToken(TokenType::becomes);
        else
            addToken(TokenType::colon);
        return;
    case '=':
        addToken(TokenType::eql);
        return;
    case '<':
        if (match('='))
            addToken(TokenType::leq);
        else if (match('>'))
            addToken(TokenType::neq);
        else
            addToken(TokenType::lss);
        return;
    case '>':
        if (match('='))
            addToken(TokenType::geq);
        else
            addToken(TokenType::gtr);
        return;
    case '"':
        stringLiteral();
        return;
    default:
        if (isAlpha(c)) {
            identifier();
            return;
        } else if (isdigit(static_cast<unsigned char>(c))) {
            number();
            return;
        } else {
            // Unknown char
            return;
        }
    }
}

bool Scanner::isAtEnd() const { return peek() == '\0'; }

char Scanner::advance() { return source.at(current++); }

char Scanner::peek() const { return source.at(current); }

char Scanner::peekNext() const { return source.at(current + 1); }

bool Scanner::match(char expected) {
    if (isAtEnd())
        return false;
    if (source.at(current) != expected)
        return false;
    current++;
    return true;
}

void Scanner::addToken(TokenType type, Literal literal) {
    string lex = source.substring(start, current - start);
    tokens.emplace_back(Token{type, lex, literal, line});
}

void Scanner::identifier() {
    while (isAlphaNumeric(peek()))
        advance();

    string text = source.substring(start, current - start);
    string lower = toLower(text);

    static const unordered_map<string, TokenType> keywords = {
        {"const", TokenType::constsy},
        {"type", TokenType::typesy},
        {"var", TokenType::varsy},
        {"function", TokenType::functionsy},
        {"procedure", TokenType::proceduresy},
        {"array", TokenType::arraysy},
        {"record", TokenType::recordsy},
        {"program", TokenType::programsy},
        {"begin", TokenType::beginsy},
        {"end", TokenType::endsy},
        {"if", TokenType::ifsy},
        {"then", TokenType::thensy},
        {"else", TokenType::elsesy},
        {"case", TokenType::casesy},
        {"of", TokenType::ofsy},
        {"repeat", TokenType::repeatsy},
        {"until", TokenType::untilsy},
        {"while", TokenType::whilesy},
        {"do", TokenType::dosy},
        {"for", TokenType::forsy},
        {"to", TokenType::tosy},
        {"downto", TokenType::downtosy},
        {"not", TokenType::notsy},
        {"and", TokenType::andsy},
        {"or", TokenType::orsy},
        {"div", TokenType::idiv},
        {"mod", TokenType::imod},
    };

    auto it = keywords.find(lower);
    if (it != keywords.end()) {
        addToken(it->second);
    } else {
        addToken(TokenType::ident, Literal{text});
    }
}

void Scanner::stringLiteral() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd()) {
        string val = source.substring(start + 1, current - start - 1);
        addToken(TokenType::string, Literal{val});
        return;
    }

    advance();
    string val = source.substring(start + 1, current - start - 2 + 1);
    addToken(TokenType::string, Literal{val});
}

bool Scanner::isAlpha(char c) {
    return isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || isdigit(static_cast<unsigned char>(c));
}

string Scanner::toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return tolower(ch); });
    return s;
}

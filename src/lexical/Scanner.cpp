#include "Scanner.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cctype>
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

void Scanner::printAllTokens() {
    const vector<Token> scannedTokens = scanTokens();
    for (const Token &token : scannedTokens) {
        printToken(token);
    }
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
    case '{':
        while (peek() != '}' && !isAtEnd()) {
            if (peek() == '\n')
                line++;
            advance();
        }
        if (!isAtEnd()) {
            advance();
        } else {
            Logger::error(line, "Unterminated comment.");
        }
        return;
    case '(':
        if (match('*')) {
            while (!(peek() == '*' && peekNext() == ')') && !isAtEnd()) {
                if (peek() == '\n')
                    line++;
                advance();
            }

            if (!isAtEnd()) {
                advance(); // consume '*'
                advance(); // consume ')'
            } else {
                Logger::error(line, "Unterminated comment.");
            }
        } else {
            addToken(TokenType::lparent);
        }
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
    case '/':
        addToken(TokenType::rdiv);
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
        if (match('='))
            addToken(TokenType::eql);
        else
            Logger::error(line, "Unexpected character: =");
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
    case '\'':
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
            Logger::error(line, std::string("Unexpected character: ") +
                                    std::string(1, c));
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
    while (peek() != '\'' && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd()) {
        Logger::error(line, "Unterminated string or character literal.");
        return;
    }

    advance(); // consume closing quote
    string val = source.substring(start + 1, current - start - 2);

    if (val.length() == 1) {
        addToken(TokenType::charcon, Literal{val});
    } else {
        addToken(TokenType::string, Literal{val});
    }
}

bool Scanner::isAlpha(char c) {
    return isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || isdigit(static_cast<unsigned char>(c));
}

string Scanner::toLower(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char ch) { return tolower(ch); });
    return s;
}

void Scanner::number() {
    while (isdigit(static_cast<unsigned char>(peek())))
        advance();

    if (peek() == '.' && isdigit(static_cast<unsigned char>(peekNext()))) {
        advance(); // consume '.'
        while (isdigit(static_cast<unsigned char>(peek())))
            advance();
        addToken(TokenType::realcon,
                 stod(source.substring(start, current - start)));
    } else {
        addToken(TokenType::intcon, static_cast<double>(stoi(source.substring(
                                        start, current - start))));
    }
}

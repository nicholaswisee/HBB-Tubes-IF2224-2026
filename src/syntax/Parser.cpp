#include "Parser.hpp"

#include <iostream>
#include <sstream>

using namespace std;


Parser::Parser(const vector<Token> &tokens) : tokens(tokens), pos(0) {}
const Token &Parser::peek() const { return tokens[pos]; }
const Token &Parser::previous() const { return tokens[pos - 1]; }
const Token &Parser::advance() {
    if (!isAtEnd())
        pos++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd())
        return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token &Parser::expect(TokenType type, const string &errMsg) {
    if (check(type))
        return advance();
    error(errMsg);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::eof_token;
}

shared_ptr<ParseTreeNode>
Parser::consumeTerminal(TokenType type, const string &errMsg) {
    const Token &tok = expect(type, errMsg);
    return makeNode(tok.toString(), tok.line);
}

[[noreturn]] void Parser::error(const string &msg) {
    const Token &tok = peek();
    ostringstream oss;
    oss << "Syntax error on line " << tok.line << ": " << msg
        << " (got " << tok.toString() << ")";
    throw SyntaxError(tok.line, oss.str());
}

shared_ptr<ParseTreeNode> Parser::parse() { return parseProgram(); }
shared_ptr<ParseTreeNode> Parser::parseProgram() {
    auto node = makeNode("<program>");
    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(consumeTerminal(TokenType::period,
                                   "Expected '.' at end of program"));
    return node;
}

// program-header → programsy ident semicolon
shared_ptr<ParseTreeNode> Parser::parseProgramHeader() {
    auto node = makeNode("<program-header>");
    node->addChild(
        consumeTerminal(TokenType::programsy, "Expected 'program'"));
    node->addChild(consumeTerminal(TokenType::ident,
                                   "Expected program name (identifier)"));
    node->addChild(
        consumeTerminal(TokenType::semicolon, "Expected ';' after program name"));
    return node;
}

// declaration-part → (const-declaration)* (type-declaration)*
//                     (var-declaration)* (subprogram-declaration)*
shared_ptr<ParseTreeNode> Parser::parseDeclarationPart() {
    auto node = makeNode("<declaration-part>");

    while (check(TokenType::constsy)) {
        node->addChild(parseConstDeclaration());
    }
    while (check(TokenType::typesy)) {
        node->addChild(parseTypeDeclaration());
    }
    while (check(TokenType::varsy)) {
        node->addChild(parseVarDeclaration());
    }
    while (check(TokenType::proceduresy) || check(TokenType::functionsy)) {
        node->addChild(parseSubprogramDeclaration());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseConstDeclaration() {
    auto node = makeNode("<const-declaration>");
    node->addChild(consumeTerminal(TokenType::constsy, "Expected 'const'"));

    do {
        node->addChild(consumeTerminal(TokenType::ident,
                                       "Expected identifier in const declaration"));
        node->addChild(
            consumeTerminal(TokenType::eql, "Expected '==' in const declaration"));
        node->addChild(parseConstant());
        node->addChild(consumeTerminal(TokenType::semicolon,
                                       "Expected ';' after constant"));
    } while (check(TokenType::ident));

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseConstant() {
    auto node = makeNode("<constant>");

    if (check(TokenType::charcon)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else if (check(TokenType::string)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else {
        if (check(TokenType::plus) || check(TokenType::minus)) {
            const Token& tok = advance();
            node->addChild(makeNode(tok.toString(), tok.line));
        }
        if (check(TokenType::ident) || check(TokenType::intcon) ||
            check(TokenType::realcon)) {
            const Token& tok = advance();
            node->addChild(makeNode(tok.toString(), tok.line));
        } else {
            error("Expected constant value");
        }
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseTypeDeclaration() {
    auto node = makeNode("<type-declaration>");
    node->addChild(consumeTerminal(TokenType::typesy, "Expected 'type'"));

    do {
        node->addChild(consumeTerminal(TokenType::ident,
                                       "Expected identifier in type declaration"));
        node->addChild(
            consumeTerminal(TokenType::eql, "Expected '==' in type declaration"));
        node->addChild(parseType());
        node->addChild(
            consumeTerminal(TokenType::semicolon, "Expected ';' after type definition"));
    } while (check(TokenType::ident));

    return node;
}


shared_ptr<ParseTreeNode> Parser::parseType() {
    auto node = makeNode("<type>");

    if (check(TokenType::arraysy)) {
        node->addChild(parseArrayType());
    } else if (check(TokenType::recordsy)) {
        node->addChild(parseRecordType());
    } else if (check(TokenType::lparent)) {
        node->addChild(parseEnumerated());
    } else if (check(TokenType::ident)) {
        size_t saved = pos;
        const Token& identTok = advance();
        auto identNode = makeNode(identTok.toString(), identTok.line);

        if (check(TokenType::period)) {
            size_t periodPos = pos;
            advance(); // first period
            if (check(TokenType::period)) {
                pos = saved;
                node->addChild(parseRange());
            } else {
                pos = periodPos;
                node->addChild(identNode);
            }
        } else {
            node->addChild(identNode);
        }
    } else if (check(TokenType::intcon) || check(TokenType::realcon) ||
               check(TokenType::charcon) || check(TokenType::plus) ||
               check(TokenType::minus)) {
        node->addChild(parseRange());
    } else {
        error("Expected type");
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseArrayType() {
    auto node = makeNode("<array-type>");
    node->addChild(consumeTerminal(TokenType::arraysy, "Expected 'array'"));
    node->addChild(consumeTerminal(TokenType::lbrack, "Expected '['"));

    if (check(TokenType::ident)) {
        size_t saved = pos;
        const Token& identTok = advance();
        if (check(TokenType::period)) {
            pos = saved;
            node->addChild(parseRange());
        } else {
            node->addChild(makeNode(identTok.toString(), identTok.line));
        }
    } else {
        node->addChild(parseRange());
    }

    node->addChild(consumeTerminal(TokenType::rbrack, "Expected ']'"));
    node->addChild(consumeTerminal(TokenType::ofsy, "Expected 'of'"));
    node->addChild(parseType());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseRange() {
    auto node = makeNode("<range>");
    node->addChild(parseConstant());
    node->addChild(
        consumeTerminal(TokenType::period, "Expected '..' in range (first '.')"));
    node->addChild(
        consumeTerminal(TokenType::period, "Expected '..' in range (second '.')"));
    node->addChild(parseConstant());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseEnumerated() {
    auto node = makeNode("<enumerated>");
    node->addChild(consumeTerminal(TokenType::lparent, "Expected '('"));
    node->addChild(
        consumeTerminal(TokenType::ident, "Expected identifier in enumeration"));

    while (match(TokenType::comma)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(consumeTerminal(TokenType::ident,
                                       "Expected identifier after ','"));
    }

    node->addChild(consumeTerminal(TokenType::rparent, "Expected ')'"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseRecordType() {
    auto node = makeNode("<record-type>");
    node->addChild(consumeTerminal(TokenType::recordsy, "Expected 'record'"));
    node->addChild(parseFieldList());
    node->addChild(consumeTerminal(TokenType::endsy, "Expected 'end'"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseFieldList() {
    auto node = makeNode("<field-list>");
    node->addChild(parseFieldPart());

    while (match(TokenType::semicolon)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        if (check(TokenType::ident)) {
            node->addChild(parseFieldPart());
        }
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseFieldPart() {
    auto node = makeNode("<field-part>");
    node->addChild(parseIdentifierList());
    node->addChild(consumeTerminal(TokenType::colon, "Expected ':'"));
    node->addChild(parseType());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseVarDeclaration() {
    auto node = makeNode("<var-declaration>");
    node->addChild(consumeTerminal(TokenType::varsy, "Expected 'var'"));

    do {
        node->addChild(parseIdentifierList());
        node->addChild(consumeTerminal(TokenType::colon,
                                       "Expected ':' in var declaration"));
        node->addChild(parseType());
        node->addChild(consumeTerminal(TokenType::semicolon,
                                       "Expected ';' after var declaration"));
    } while (check(TokenType::ident));

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseIdentifierList() {
    auto node = makeNode("<identifier-list>");
    node->addChild(
        consumeTerminal(TokenType::ident, "Expected identifier"));

    while (match(TokenType::comma)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(
            consumeTerminal(TokenType::ident, "Expected identifier after ','"));
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseSubprogramDeclaration() {
    if (check(TokenType::proceduresy)) {
        return parseProcedureDeclaration();
    } else {
        return parseFunctionDeclaration();
    }
}
shared_ptr<ParseTreeNode> Parser::parseProcedureDeclaration() {
    auto node = makeNode("<procedure-declaration>");
    node->addChild(
        consumeTerminal(TokenType::proceduresy, "Expected 'procedure'"));
    node->addChild(consumeTerminal(TokenType::ident,
                                   "Expected procedure name"));
    if (check(TokenType::lparent)) {
        node->addChild(parseFormalParameterList());
    }
    node->addChild(consumeTerminal(TokenType::semicolon,
                                   "Expected ';' after procedure header"));
    node->addChild(parseBlock());
    node->addChild(consumeTerminal(TokenType::semicolon,
                                   "Expected ';' after procedure body"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseFunctionDeclaration() {
    auto node = makeNode("<function-declaration>");
    node->addChild(
        consumeTerminal(TokenType::functionsy, "Expected 'function'"));
    node->addChild(
        consumeTerminal(TokenType::ident, "Expected function name"));

    if (check(TokenType::lparent)) {
        node->addChild(parseFormalParameterList());
    }

    node->addChild(
        consumeTerminal(TokenType::colon, "Expected ':' before return type"));
    node->addChild(consumeTerminal(TokenType::ident,
                                   "Expected return type identifier"));
    node->addChild(consumeTerminal(TokenType::semicolon,
                                   "Expected ';' after function header"));
    node->addChild(parseBlock());
    node->addChild(consumeTerminal(TokenType::semicolon,
                                   "Expected ';' after function body"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseBlock() {
    auto node = makeNode("<block>");
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseFormalParameterList() {
    auto node = makeNode("<formal-parameter-list>");
    node->addChild(consumeTerminal(TokenType::lparent, "Expected '('"));
    node->addChild(parseParameterGroup());

    while (match(TokenType::semicolon)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseParameterGroup());
    }

    node->addChild(consumeTerminal(TokenType::rparent, "Expected ')'"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseParameterGroup() {
    auto node = makeNode("<parameter-group>");
    node->addChild(parseIdentifierList());
    node->addChild(consumeTerminal(TokenType::colon, "Expected ':'"));

    if (check(TokenType::arraysy)) {
        node->addChild(parseArrayType());
    } else {
        node->addChild(
            consumeTerminal(TokenType::ident, "Expected parameter type"));
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseCompoundStatement() {
    auto node = makeNode("<compound-statement>");
    node->addChild(consumeTerminal(TokenType::beginsy, "Expected 'begin'"));
    node->addChild(parseStatementList());
    node->addChild(consumeTerminal(TokenType::endsy, "Expected 'end'"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseStatementList() {
    auto node = makeNode("<statement-list>");
    node->addChild(parseStatement());

    while (match(TokenType::semicolon)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseStatement());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseStatement() {
    auto node = makeNode("<statement>");

    if (check(TokenType::ident)) {
        size_t saved = pos;
        parseVariable(); 
        bool isAssign = check(TokenType::becomes);
        pos = saved; // backtrack
        if (isAssign) {
            node->addChild(parseAssignmentStatement());
        } else {
            node->addChild(parseProcFuncCall());
        }
    } else if (check(TokenType::ifsy)) {
        node->addChild(parseIfStatement());
    } else if (check(TokenType::casesy)) {
        node->addChild(parseCaseStatement());
    } else if (check(TokenType::whilesy)) {
        node->addChild(parseWhileStatement());
    } else if (check(TokenType::repeatsy)) {
        node->addChild(parseRepeatStatement());
    } else if (check(TokenType::forsy)) {
        node->addChild(parseForStatement());
    } else if (check(TokenType::beginsy)) {
        node->addChild(parseCompoundStatement());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseAssignmentStatement() {
    auto node = makeNode("<assignment-statement>");
    node->addChild(parseVariable());
    node->addChild(consumeTerminal(TokenType::becomes, "Expected ':='"));
    node->addChild(parseExpression());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseIfStatement() {
    auto node = makeNode("<if-statement>");
    node->addChild(consumeTerminal(TokenType::ifsy, "Expected 'if'"));
    node->addChild(parseExpression());
    node->addChild(consumeTerminal(TokenType::thensy, "Expected 'then'"));
    node->addChild(parseStatement());

    if (match(TokenType::elsesy)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseStatement());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseCaseStatement() {
    auto node = makeNode("<case-statement>");
    node->addChild(consumeTerminal(TokenType::casesy, "Expected 'case'"));
    node->addChild(parseExpression());
    node->addChild(consumeTerminal(TokenType::ofsy, "Expected 'of'"));
    node->addChild(parseCaseBlock());
    node->addChild(consumeTerminal(TokenType::endsy, "Expected 'end'"));
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseCaseBlock() {
    auto node = makeNode("<case-block>");

    node->addChild(parseConstant());
    while (match(TokenType::comma)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseConstant());
    }

    node->addChild(consumeTerminal(TokenType::colon, "Expected ':'"));
    node->addChild(parseStatement());

    while (match(TokenType::semicolon)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        if (!check(TokenType::endsy)) {
            node->addChild(parseCaseBlock());
        }
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseWhileStatement() {
    auto node = makeNode("<while-statement>");
    node->addChild(consumeTerminal(TokenType::whilesy, "Expected 'while'"));
    node->addChild(parseExpression());
    node->addChild(consumeTerminal(TokenType::dosy, "Expected 'do'"));
    node->addChild(parseStatement());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseRepeatStatement() {
    auto node = makeNode("<repeat-statement>");
    node->addChild(consumeTerminal(TokenType::repeatsy, "Expected 'repeat'"));
    node->addChild(parseStatementList());
    node->addChild(consumeTerminal(TokenType::untilsy, "Expected 'until'"));
    node->addChild(parseExpression());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseForStatement() {
    auto node = makeNode("<for-statement>");
    node->addChild(consumeTerminal(TokenType::forsy, "Expected 'for'"));
    node->addChild(
        consumeTerminal(TokenType::ident, "Expected loop variable"));
    node->addChild(consumeTerminal(TokenType::becomes, "Expected ':='"));
    node->addChild(parseExpression());

    if (check(TokenType::tosy)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else if (check(TokenType::downtosy)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else {
        error("Expected 'to' or 'downto'");
    }

    node->addChild(parseExpression());
    node->addChild(consumeTerminal(TokenType::dosy, "Expected 'do'"));
    node->addChild(parseStatement());
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseProcFuncCall() {
    auto node = makeNode("<procedure/function-call>");
    node->addChild(consumeTerminal(TokenType::ident,
                                   "Expected procedure/function name"));

    if (match(TokenType::lparent)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        if (!check(TokenType::rparent)) {
            node->addChild(parseParameterList());
        }
        node->addChild(consumeTerminal(TokenType::rparent, "Expected ')'"));
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseParameterList() {
    auto node = makeNode("<parameter-list>");
    node->addChild(parseExpression());

    while (match(TokenType::comma)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseExpression());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseExpression() {
    auto node = makeNode("<expression>");
    node->addChild(parseSimpleExpression());

    if (check(TokenType::eql) || check(TokenType::neq) ||
        check(TokenType::gtr) || check(TokenType::geq) ||
        check(TokenType::lss) || check(TokenType::leq)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
        node->addChild(parseSimpleExpression());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseSimpleExpression() {
    auto node = makeNode("<simple-expression>");

    if (check(TokenType::plus) || check(TokenType::minus)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    }

    node->addChild(parseTerm());

    while (check(TokenType::plus) || check(TokenType::minus) ||
           check(TokenType::orsy)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
        node->addChild(parseTerm());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseTerm() {
    auto node = makeNode("<term>");
    node->addChild(parseFactor());

    while (check(TokenType::times) || check(TokenType::rdiv) ||
           check(TokenType::idiv) || check(TokenType::imod) ||
           check(TokenType::andsy)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
        node->addChild(parseFactor());
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseFactor() {
    auto node = makeNode("<factor>");

    if (check(TokenType::intcon) || check(TokenType::realcon) ||
        check(TokenType::charcon) || check(TokenType::string)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else if (check(TokenType::ident)) {
        size_t saved = pos;
        advance();
        if (check(TokenType::lparent)) {
            pos = saved;
            node->addChild(parseProcFuncCall());
        } else {
            pos = saved;
            node->addChild(parseVariable());
        }
    } else if (match(TokenType::lparent)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseExpression());
        node->addChild(
            consumeTerminal(TokenType::rparent, "Expected ')'"));
    } else if (match(TokenType::notsy)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseFactor());
    } else {
        error("Expected factor (value, identifier, or expression)");
    }

    return node;
}

shared_ptr<ParseTreeNode> Parser::parseVariable() {
    auto node = makeNode("<variable>");
    node->addChild(consumeTerminal(TokenType::ident,
                                   "Expected identifier"));
    while (check(TokenType::lbrack) || check(TokenType::period)) {
        node->addChild(parseComponentVariable());
    }
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseComponentVariable() {
    auto node = makeNode("<component-variable>");
    if (match(TokenType::lbrack)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(parseIndexList());
        node->addChild(
            consumeTerminal(TokenType::rbrack, "Expected ']'"));
    } else if (match(TokenType::period)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        node->addChild(
            consumeTerminal(TokenType::ident, "Expected field name"));
    } else {
        error("Expected '[' or '.'");
    }
    return node;
}

shared_ptr<ParseTreeNode> Parser::parseIndexList() {
    auto node = makeNode("<index-list>");
    if (check(TokenType::intcon) || check(TokenType::charcon) ||
        check(TokenType::ident)) {
        const Token& tok = advance();
        node->addChild(makeNode(tok.toString(), tok.line));
    } else {
        error("Expected index (integer, char, or identifier)");
    }
    while (match(TokenType::comma)) {
        node->addChild(makeNode(previous().toString(), previous().line));
        if (check(TokenType::intcon) || check(TokenType::charcon) ||
            check(TokenType::ident)) {
            const Token& tok = advance();
            node->addChild(makeNode(tok.toString(), tok.line));
        } else {
            error("Expected index after ','");
        }
    }
    return node;
}

#pragma once

#include "../lexical/Token.hpp"
#include "ParseTree.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct SyntaxError : public std::runtime_error {
    int line;
    SyntaxError(int line, const std::string &msg): std::runtime_error(msg), line(line) {}
};

class Parser {
  public:
    explicit Parser(const std::vector<Token> &tokens);
    std::shared_ptr<ParseTreeNode> parse();

  private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token &peek() const;
    const Token &previous() const;
    const Token &advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token &expect(TokenType type, const std::string &errMsg);
    bool isAtEnd() const;
    std::shared_ptr<ParseTreeNode> consumeTerminal(TokenType type, const std::string &errMsg);
    [[noreturn]] void error(const std::string &msg);

    std::shared_ptr<ParseTreeNode> parseProgram();
    std::shared_ptr<ParseTreeNode> parseProgramHeader();
    std::shared_ptr<ParseTreeNode> parseDeclarationPart();
    std::shared_ptr<ParseTreeNode> parseConstDeclaration();
    std::shared_ptr<ParseTreeNode> parseConstant();
    std::shared_ptr<ParseTreeNode> parseTypeDeclaration();
    std::shared_ptr<ParseTreeNode> parseType();
    std::shared_ptr<ParseTreeNode> parseArrayType();
    std::shared_ptr<ParseTreeNode> parseRange();
    std::shared_ptr<ParseTreeNode> parseEnumerated();
    std::shared_ptr<ParseTreeNode> parseRecordType();
    std::shared_ptr<ParseTreeNode> parseFieldList();
    std::shared_ptr<ParseTreeNode> parseFieldPart();
    std::shared_ptr<ParseTreeNode> parseVarDeclaration();
    std::shared_ptr<ParseTreeNode> parseIdentifierList();
    std::shared_ptr<ParseTreeNode> parseSubprogramDeclaration();
    std::shared_ptr<ParseTreeNode> parseProcedureDeclaration();
    std::shared_ptr<ParseTreeNode> parseFunctionDeclaration();
    std::shared_ptr<ParseTreeNode> parseBlock();
    std::shared_ptr<ParseTreeNode> parseFormalParameterList();
    std::shared_ptr<ParseTreeNode> parseParameterGroup();
    std::shared_ptr<ParseTreeNode> parseCompoundStatement();
    std::shared_ptr<ParseTreeNode> parseStatementList();
    std::shared_ptr<ParseTreeNode> parseStatement();
    std::shared_ptr<ParseTreeNode> parseAssignmentStatement();
    std::shared_ptr<ParseTreeNode> parseIfStatement();
    std::shared_ptr<ParseTreeNode> parseCaseStatement();
    std::shared_ptr<ParseTreeNode> parseCaseBlock();
    std::shared_ptr<ParseTreeNode> parseWhileStatement();
    std::shared_ptr<ParseTreeNode> parseRepeatStatement();
    std::shared_ptr<ParseTreeNode> parseForStatement();
    std::shared_ptr<ParseTreeNode> parseProcFuncCall();
    std::shared_ptr<ParseTreeNode> parseParameterList();
    std::shared_ptr<ParseTreeNode> parseExpression();
    std::shared_ptr<ParseTreeNode> parseSimpleExpression();
    std::shared_ptr<ParseTreeNode> parseTerm();
    std::shared_ptr<ParseTreeNode> parseFactor();
    std::shared_ptr<ParseTreeNode> parseVariable();
    std::shared_ptr<ParseTreeNode> parseComponentVariable();
    std::shared_ptr<ParseTreeNode> parseIndexList();
};

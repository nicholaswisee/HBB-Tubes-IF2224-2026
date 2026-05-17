#pragma once

#include "syntax/ParseTree.hpp"
#include "ASTNode.hpp"
#include <memory>

class ParseTreeToAST {
public:
    std::shared_ptr<ASTNode> convert(std::shared_ptr<ParseTreeNode> root);

private:
    // Extract helpers
    static std::string extractIdent(const std::string& label);
    static std::string extractLiteralValue(const std::string& label);
    static std::string tokenLabelToOp(const std::string& label);
    static bool isTerminalOperator(const std::string& label);

    // Conversion helpers for each non-terminal
    std::shared_ptr<ASTNode> convertProgram(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::shared_ptr<ASTNode>> convertDeclarationPart(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::shared_ptr<ASTNode>> convertConstDecl(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::shared_ptr<ASTNode>> convertVarDecl(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::shared_ptr<ASTNode>> convertTypeDecl(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertSubprogramDecl(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertBlock(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertType(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertArrayType(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertRecordType(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertRange(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::shared_ptr<ParamNode>> convertFormalParams(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertStatement(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertStatementList(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertExpression(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertSimpleExpression(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertTerm(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertFactor(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertVariable(std::shared_ptr<ParseTreeNode> node);
    std::shared_ptr<ASTNode> convertConstant(std::shared_ptr<ParseTreeNode> node);
    std::vector<std::string> convertIdentifierList(std::shared_ptr<ParseTreeNode> node);
};

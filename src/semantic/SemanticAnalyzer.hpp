#pragma once

#include "ASTNode.hpp"
#include "ASTVisitor.hpp"
#include "SymbolTableManager.hpp"
#include "TypeSystem.hpp"

#include <vector>
#include <string>
#include <memory>
#include <iostream>

struct SemanticError {
    int line;
    std::string message;
    SemanticError(int line, const std::string& msg) : line(line), message(msg) {}
};

class SemanticAnalyzer : public ASTVisitor {
public:
    explicit SemanticAnalyzer(SymbolTableManager& symTable);

    // Entry point
    void analyze(std::shared_ptr<ASTNode> ast);

    // Visitor implementations
    void visit(ProgramNode& node) override;
    void visit(VarDeclNode& node) override;
    void visit(ConstDeclNode& node) override;
    void visit(TypeDeclNode& node) override;
    void visit(ProcDeclNode& node) override;
    void visit(FuncDeclNode& node) override;
    void visit(AssignNode& node) override;
    void visit(IfNode& node) override;
    void visit(WhileNode& node) override;
    void visit(ForNode& node) override;
    void visit(RepeatNode& node) override;
    void visit(CaseNode& node) override;
    void visit(CaseBranchNode& node) override;
    void visit(CompoundNode& node) override;
    void visit(ProcCallNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(UnaryOpNode& node) override;
    void visit(VariableNode& node) override;
    void visit(LiteralNode& node) override;
    void visit(ArrayAccessNode& node) override;
    void visit(FieldAccessNode& node) override;
    void visit(RangeNode& node) override;
    void visit(ArrayTypeNode& node) override;
    void visit(RecordTypeNode& node) override;
    void visit(ParamNode& node) override;

    // Error handling
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<SemanticError>& getErrors() const { return errors; }
    void printErrors() const;

    // Type checking helpers
    bool checkTypeCompatibility(int type1, int type2);
    bool checkAssignmentCompatibility(int targetType, int valueType);
    int inferBinaryOpType(const std::string& op, int leftType, int rightType);
    int inferUnaryOpType(const std::string& op, int operandType);

private:
    SymbolTableManager& symTable;
    std::vector<SemanticError> errors;
    int currentLine = 0;

    void reportError(const std::string& message);
    void reportError(int line, const std::string& message);
    void checkConditionType(std::shared_ptr<ASTNode> condition, const std::string& context);

    // Helper for resolving type names
    int resolveTypeName(const std::string& typeName);
    int resolveNodeType(std::shared_ptr<ASTNode> typeNode);

    // Helpers for procedure/function calls
    void checkProcedureArgs(int procIdx, const std::vector<std::shared_ptr<ASTNode>>& args, int line);

    // Track initialized variables (optional)
    std::vector<bool> initialized;
};

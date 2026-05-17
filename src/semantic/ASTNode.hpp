#pragma once

#include "ASTVisitor.hpp"
#include <memory>
#include <string>
#include <vector>

class ASTNode {
public:
    int type = 0;      
    int tabIndex = -1; 
    int line = 0;      
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

class ProgramNode : public ASTNode {
public:
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> declarations;
    std::shared_ptr<ASTNode> body;
    explicit ProgramNode(const std::string& name) : name(name) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VarDeclNode : public ASTNode {
public:
    std::string name;
    std::string typeName;
    std::shared_ptr<ASTNode> typeNode; 
    VarDeclNode(const std::string& name, const std::string& typeName)
        : name(name), typeName(typeName) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ConstDeclNode : public ASTNode {
public:
    std::string name;
    std::shared_ptr<ASTNode> value;
    ConstDeclNode(const std::string& name, std::shared_ptr<ASTNode> value)
        : name(name), value(value) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class TypeDeclNode : public ASTNode {
public:
    std::string name;
    std::shared_ptr<ASTNode> typeNode;
    TypeDeclNode(const std::string& name, std::shared_ptr<ASTNode> typeNode)
        : name(name), typeNode(typeNode) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ParamNode : public ASTNode {
public:
    std::string name;
    std::string typeName;
    bool isVar = false;
    ParamNode(const std::string& name, const std::string& typeName, bool isVar = false)
        : name(name), typeName(typeName), isVar(isVar) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ProcDeclNode : public ASTNode {
public:
    std::string name;
    std::vector<std::shared_ptr<ParamNode>> params;
    std::vector<std::shared_ptr<ASTNode>> localDeclarations;
    std::shared_ptr<ASTNode> body;
    explicit ProcDeclNode(const std::string& name) : name(name) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FuncDeclNode : public ASTNode {
public:
    std::string name;
    std::string returnTypeName;
    std::vector<std::shared_ptr<ParamNode>> params;
    std::vector<std::shared_ptr<ASTNode>> localDeclarations;
    std::shared_ptr<ASTNode> body;
    FuncDeclNode(const std::string& name, const std::string& returnTypeName)
        : name(name), returnTypeName(returnTypeName) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class AssignNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> target;
    std::shared_ptr<ASTNode> value;
    AssignNode(std::shared_ptr<ASTNode> target, std::shared_ptr<ASTNode> value)
        : target(target), value(value) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class IfNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> thenBranch;
    std::shared_ptr<ASTNode> elseBranch;
    IfNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> thenB,
           std::shared_ptr<ASTNode> elseB = nullptr)
        : condition(cond), thenBranch(thenB), elseBranch(elseB) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class WhileNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> body;
    WhileNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> body)
        : condition(cond), body(body) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ForNode : public ASTNode {
public:
    std::string varName;
    std::shared_ptr<ASTNode> initExpr;
    std::string direction; 
    std::shared_ptr<ASTNode> finalExpr;
    std::shared_ptr<ASTNode> body;
    ForNode(const std::string& varName, std::shared_ptr<ASTNode> init,
            const std::string& dir, std::shared_ptr<ASTNode> final,
            std::shared_ptr<ASTNode> body)
        : varName(varName), initExpr(init), direction(dir), finalExpr(final), body(body) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class RepeatNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;
    std::shared_ptr<ASTNode> condition;
    RepeatNode(std::vector<std::shared_ptr<ASTNode>> stmts, std::shared_ptr<ASTNode> cond)
        : statements(stmts), condition(cond) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class CaseBranchNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> constants;
    std::shared_ptr<ASTNode> statement;
    CaseBranchNode(std::vector<std::shared_ptr<ASTNode>> consts, std::shared_ptr<ASTNode> stmt)
        : constants(consts), statement(stmt) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class CaseNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> expression;
    std::vector<std::shared_ptr<ASTNode>> branches;
    CaseNode(std::shared_ptr<ASTNode> expr, std::vector<std::shared_ptr<ASTNode>> branches)
        : expression(expr), branches(branches) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class CompoundNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;
    CompoundNode(std::vector<std::shared_ptr<ASTNode>> stmts) : statements(stmts) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ProcCallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> arguments;
    ProcCallNode(const std::string& name, std::vector<std::shared_ptr<ASTNode>> args)
        : name(name), arguments(args) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BinaryOpNode : public ASTNode {
public:
    std::string op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    BinaryOpNode(const std::string& op, std::shared_ptr<ASTNode> left,
                 std::shared_ptr<ASTNode> right)
        : op(op), left(left), right(right) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class UnaryOpNode : public ASTNode {
public:
    std::string op;
    std::shared_ptr<ASTNode> operand;
    UnaryOpNode(const std::string& op, std::shared_ptr<ASTNode> operand)
        : op(op), operand(operand) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VariableNode : public ASTNode {
public:
    std::string name;
    explicit VariableNode(const std::string& name) : name(name) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class LiteralNode : public ASTNode {
public:
    std::string value;
    explicit LiteralNode(const std::string& value) : value(value) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayAccessNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> arrayExpr;
    std::vector<std::shared_ptr<ASTNode>> indices;
    ArrayAccessNode(std::shared_ptr<ASTNode> arr, std::vector<std::shared_ptr<ASTNode>> idx)
        : arrayExpr(arr), indices(idx) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FieldAccessNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> recordExpr;
    std::string fieldName;
    FieldAccessNode(std::shared_ptr<ASTNode> rec, const std::string& field)
        : recordExpr(rec), fieldName(field) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class RangeNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> low;
    std::shared_ptr<ASTNode> high;
    RangeNode(std::shared_ptr<ASTNode> low, std::shared_ptr<ASTNode> high)
        : low(low), high(high) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayTypeNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> indexType;   
    std::shared_ptr<ASTNode> elementType; 
    ArrayTypeNode(std::shared_ptr<ASTNode> idx, std::shared_ptr<ASTNode> elem)
        : indexType(idx), elementType(elem) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class RecordTypeNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> fields; 
    RecordTypeNode(std::vector<std::shared_ptr<ASTNode>> fields) : fields(fields) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

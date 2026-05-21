#pragma once

#include "ASTVisitor.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class ASTNode {
  public:
    int type = 0;
    int tabIndex = -1;
    int level = -1; // Lexical level annotation (e.g. 0 = global)
    int line = 0;
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
    virtual std::string toString() const = 0;
    virtual std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const = 0;

    void print(std::ostream &os, const std::string &prefix = "",
               bool isLast = true, const std::string &label = "") const {
        os << prefix << (prefix.empty() ? "" : (isLast ? "+-- " : "|-- "));
        if (!label.empty()) {
            os << label << ": ";
        }
        os << toString() << "\n";

        std::string newPrefix = prefix + (isLast ? "    " : "|   ");
        auto children = getChildren();
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
            activeChildren;
        for (const auto &child : children) {
            if (child.second) {
                activeChildren.push_back(child);
            }
        }
        for (size_t i = 0; i < activeChildren.size(); ++i) {
            activeChildren[i].second->print(os, newPrefix,
                                            i == activeChildren.size() - 1,
                                            activeChildren[i].first);
        }
    }
};

class ProgramNode : public ASTNode {
  public:
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> declarations;
    std::shared_ptr<ASTNode> body;
    explicit ProgramNode(const std::string &name) : name(name) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ProgramNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < declarations.size(); ++i) {
            children.push_back(
                {"declaration[" + std::to_string(i) + "]", declarations[i]});
        }
        children.push_back({"body", body});
        return children;
    }
};

class VarDeclNode : public ASTNode {
  public:
    std::string name;
    std::string typeName;
    std::shared_ptr<ASTNode> typeNode;
    VarDeclNode(const std::string &name, const std::string &typeName)
        : name(name), typeName(typeName) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "VarDeclNode(" + name + " : " + typeName + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        if (typeNode) {
            children.push_back({"typeNode", typeNode});
        }
        return children;
    }
};

class ConstDeclNode : public ASTNode {
  public:
    std::string name;
    std::shared_ptr<ASTNode> value;
    ConstDeclNode(const std::string &name, std::shared_ptr<ASTNode> value)
        : name(name), value(value) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ConstDeclNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        children.push_back({"value", value});
        return children;
    }
};

class TypeDeclNode : public ASTNode {
  public:
    std::string name;
    std::shared_ptr<ASTNode> typeNode;
    TypeDeclNode(const std::string &name, std::shared_ptr<ASTNode> typeNode)
        : name(name), typeNode(typeNode) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "TypeDeclNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        children.push_back({"typeNode", typeNode});
        return children;
    }
};

class ParamNode : public ASTNode {
  public:
    std::string name;
    std::string typeName;
    bool isVar = false;
    ParamNode(const std::string &name, const std::string &typeName,
              bool isVar = false)
        : name(name), typeName(typeName), isVar(isVar) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ParamNode(" + name + " : " + typeName + (isVar ? ", var" : "") +
               ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {};
    }
};

class ProcDeclNode : public ASTNode {
  public:
    std::string name;
    std::vector<std::shared_ptr<ParamNode>> params;
    std::vector<std::shared_ptr<ASTNode>> localDeclarations;
    std::shared_ptr<ASTNode> body;
    explicit ProcDeclNode(const std::string &name) : name(name) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ProcDeclNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < params.size(); ++i) {
            children.push_back({"param[" + std::to_string(i) + "]", params[i]});
        }
        for (size_t i = 0; i < localDeclarations.size(); ++i) {
            children.push_back(
                {"localDecl[" + std::to_string(i) + "]", localDeclarations[i]});
        }
        children.push_back({"body", body});
        return children;
    }
};

class FuncDeclNode : public ASTNode {
  public:
    std::string name;
    std::string returnTypeName;
    std::vector<std::shared_ptr<ParamNode>> params;
    std::vector<std::shared_ptr<ASTNode>> localDeclarations;
    std::shared_ptr<ASTNode> body;
    FuncDeclNode(const std::string &name, const std::string &returnTypeName)
        : name(name), returnTypeName(returnTypeName) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "FuncDeclNode(" + name + " : " + returnTypeName + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < params.size(); ++i) {
            children.push_back({"param[" + std::to_string(i) + "]", params[i]});
        }
        for (size_t i = 0; i < localDeclarations.size(); ++i) {
            children.push_back(
                {"localDecl[" + std::to_string(i) + "]", localDeclarations[i]});
        }
        children.push_back({"body", body});
        return children;
    }
};

class AssignNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> target;
    std::shared_ptr<ASTNode> value;
    AssignNode(std::shared_ptr<ASTNode> target, std::shared_ptr<ASTNode> value)
        : target(target), value(value) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "AssignNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"target", target}, {"value", value}};
    }
};

class IfNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> thenBranch;
    std::shared_ptr<ASTNode> elseBranch;
    IfNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> thenB,
           std::shared_ptr<ASTNode> elseB = nullptr)
        : condition(cond), thenBranch(thenB), elseBranch(elseB) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "IfNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"condition", condition},
                {"thenBranch", thenBranch},
                {"elseBranch", elseBranch}};
    }
};

class WhileNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> body;
    WhileNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> body)
        : condition(cond), body(body) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "WhileNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"condition", condition}, {"body", body}};
    }
};

class ForNode : public ASTNode {
  public:
    std::string varName;
    std::shared_ptr<ASTNode> initExpr;
    std::string direction;
    std::shared_ptr<ASTNode> finalExpr;
    std::shared_ptr<ASTNode> body;
    ForNode(const std::string &varName, std::shared_ptr<ASTNode> init,
            const std::string &dir, std::shared_ptr<ASTNode> final,
            std::shared_ptr<ASTNode> body)
        : varName(varName), initExpr(init), direction(dir), finalExpr(final),
          body(body) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ForNode(" + varName + " " + direction + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {
            {"initExpr", initExpr}, {"finalExpr", finalExpr}, {"body", body}};
    }
};

class RepeatNode : public ASTNode {
  public:
    std::vector<std::shared_ptr<ASTNode>> statements;
    std::shared_ptr<ASTNode> condition;
    RepeatNode(std::vector<std::shared_ptr<ASTNode>> stmts,
               std::shared_ptr<ASTNode> cond)
        : statements(stmts), condition(cond) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "RepeatNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < statements.size(); ++i) {
            children.push_back(
                {"statement[" + std::to_string(i) + "]", statements[i]});
        }
        children.push_back({"condition", condition});
        return children;
    }
};

class CaseBranchNode : public ASTNode {
  public:
    std::vector<std::shared_ptr<ASTNode>> constants;
    std::shared_ptr<ASTNode> statement;
    CaseBranchNode(std::vector<std::shared_ptr<ASTNode>> consts,
                   std::shared_ptr<ASTNode> stmt)
        : constants(consts), statement(stmt) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "CaseBranchNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < constants.size(); ++i) {
            children.push_back(
                {"constant[" + std::to_string(i) + "]", constants[i]});
        }
        children.push_back({"statement", statement});
        return children;
    }
};

class CaseNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> expression;
    std::vector<std::shared_ptr<ASTNode>> branches;
    CaseNode(std::shared_ptr<ASTNode> expr,
             std::vector<std::shared_ptr<ASTNode>> branches)
        : expression(expr), branches(branches) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "CaseNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        children.push_back({"expression", expression});
        for (size_t i = 0; i < branches.size(); ++i) {
            children.push_back(
                {"branch[" + std::to_string(i) + "]", branches[i]});
        }
        return children;
    }
};

class CompoundNode : public ASTNode {
  public:
    std::vector<std::shared_ptr<ASTNode>> statements;
    CompoundNode(std::vector<std::shared_ptr<ASTNode>> stmts)
        : statements(stmts) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "CompoundNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < statements.size(); ++i) {
            children.push_back(
                {"statement[" + std::to_string(i) + "]", statements[i]});
        }
        return children;
    }
};

class ProcCallNode : public ASTNode {
  public:
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> arguments;
    ProcCallNode(const std::string &name,
                 std::vector<std::shared_ptr<ASTNode>> args)
        : name(name), arguments(args) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "ProcCallNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < arguments.size(); ++i) {
            children.push_back(
                {"argument[" + std::to_string(i) + "]", arguments[i]});
        }
        return children;
    }
};

class BinaryOpNode : public ASTNode {
  public:
    std::string op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    BinaryOpNode(const std::string &op, std::shared_ptr<ASTNode> left,
                 std::shared_ptr<ASTNode> right)
        : op(op), left(left), right(right) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "BinaryOpNode(" + op + ")"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"left", left}, {"right", right}};
    }
};

class UnaryOpNode : public ASTNode {
  public:
    std::string op;
    std::shared_ptr<ASTNode> operand;
    UnaryOpNode(const std::string &op, std::shared_ptr<ASTNode> operand)
        : op(op), operand(operand) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "UnaryOpNode(" + op + ")"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"operand", operand}};
    }
};

class VariableNode : public ASTNode {
  public:
    std::string name;
    explicit VariableNode(const std::string &name) : name(name) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "VariableNode(" + name + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {};
    }
};

class LiteralNode : public ASTNode {
  public:
    std::string value;
    explicit LiteralNode(const std::string &value) : value(value) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "LiteralNode(" + value + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {};
    }
};

class ArrayAccessNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> arrayExpr;
    std::vector<std::shared_ptr<ASTNode>> indices;
    ArrayAccessNode(std::shared_ptr<ASTNode> arr,
                    std::vector<std::shared_ptr<ASTNode>> idx)
        : arrayExpr(arr), indices(idx) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "ArrayAccessNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        children.push_back({"arrayExpr", arrayExpr});
        for (size_t i = 0; i < indices.size(); ++i) {
            children.push_back(
                {"index[" + std::to_string(i) + "]", indices[i]});
        }
        return children;
    }
};

class FieldAccessNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> recordExpr;
    std::string fieldName;
    FieldAccessNode(std::shared_ptr<ASTNode> rec, const std::string &field)
        : recordExpr(rec), fieldName(field) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override {
        return "FieldAccessNode(" + fieldName + ")";
    }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"recordExpr", recordExpr}};
    }
};

class RangeNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> low;
    std::shared_ptr<ASTNode> high;
    RangeNode(std::shared_ptr<ASTNode> low, std::shared_ptr<ASTNode> high)
        : low(low), high(high) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "RangeNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"low", low}, {"high", high}};
    }
};

class ArrayTypeNode : public ASTNode {
  public:
    std::shared_ptr<ASTNode> indexType;
    std::shared_ptr<ASTNode> elementType;
    ArrayTypeNode(std::shared_ptr<ASTNode> idx, std::shared_ptr<ASTNode> elem)
        : indexType(idx), elementType(elem) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "ArrayTypeNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        return {{"indexType", indexType}, {"elementType", elementType}};
    }
};

class RecordTypeNode : public ASTNode {
  public:
    std::vector<std::shared_ptr<ASTNode>> fields;
    RecordTypeNode(std::vector<std::shared_ptr<ASTNode>> fields)
        : fields(fields) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
    std::string toString() const override { return "RecordTypeNode"; }
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>>
    getChildren() const override {
        std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> children;
        for (size_t i = 0; i < fields.size(); ++i) {
            children.push_back({"field[" + std::to_string(i) + "]", fields[i]});
        }
        return children;
    }
};

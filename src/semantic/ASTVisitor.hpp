#pragma once

#include <memory>
#include <string>
#include <vector>

class ProgramNode;
class VarDeclNode;
class ConstDeclNode;
class TypeDeclNode;
class ProcDeclNode;
class FuncDeclNode;
class AssignNode;
class IfNode;
class WhileNode;
class ForNode;
class RepeatNode;
class CaseNode;
class CaseBranchNode;
class CompoundNode;
class ProcCallNode;
class BinaryOpNode;
class UnaryOpNode;
class VariableNode;
class LiteralNode;
class ArrayAccessNode;
class FieldAccessNode;
class RangeNode;
class ArrayTypeNode;
class RecordTypeNode;
class ParamNode;

class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode &node) = 0;
    virtual void visit(VarDeclNode &node) = 0;
    virtual void visit(ConstDeclNode &node) = 0;
    virtual void visit(TypeDeclNode &node) = 0;
    virtual void visit(ProcDeclNode &node) = 0;
    virtual void visit(FuncDeclNode &node) = 0;
    virtual void visit(AssignNode &node) = 0;
    virtual void visit(IfNode &node) = 0;
    virtual void visit(WhileNode &node) = 0;
    virtual void visit(ForNode &node) = 0;
    virtual void visit(RepeatNode &node) = 0;
    virtual void visit(CaseNode &node) = 0;
    virtual void visit(CaseBranchNode &node) = 0;
    virtual void visit(CompoundNode &node) = 0;
    virtual void visit(ProcCallNode &node) = 0;
    virtual void visit(BinaryOpNode &node) = 0;
    virtual void visit(UnaryOpNode &node) = 0;
    virtual void visit(VariableNode &node) = 0;
    virtual void visit(LiteralNode &node) = 0;
    virtual void visit(ArrayAccessNode &node) = 0;
    virtual void visit(FieldAccessNode &node) = 0;
    virtual void visit(RangeNode &node) = 0;
    virtual void visit(ArrayTypeNode &node) = 0;
    virtual void visit(RecordTypeNode &node) = 0;
    virtual void visit(ParamNode &node) = 0;
};

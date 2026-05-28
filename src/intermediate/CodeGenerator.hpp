#pragma once

#include "../intermediate/Instruction.hpp"
#include "../semantic/ASTNode.hpp"
#include "../semantic/ASTVisitor.hpp"
#include "../semantic/SymbolTableManager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Intermediate {

class CodeGenerator : public ASTVisitor {
  public:
    explicit CodeGenerator(SymbolTableManager &symTable);

    // Entry point: generate intermediate code from decorated AST
    std::vector<Instruction> generate(std::shared_ptr<ASTNode> ast);

    // Get generated instructions
    const std::vector<Instruction> &getInstructions() const;

    // Visitor implementations (expression & statement)
    void visit(LiteralNode &node) override;
    void visit(VariableNode &node) override;
    void visit(BinaryOpNode &node) override;
    void visit(UnaryOpNode &node) override;
    void visit(AssignNode &node) override;
    void visit(CompoundNode &node) override;
    void visit(ProcCallNode &node) override;

    // Visitor stubs (implemented by Role 2)
    void visit(ProgramNode &node) override;
    void visit(VarDeclNode &node) override;
    void visit(ConstDeclNode &node) override;
    void visit(TypeDeclNode &node) override;
    void visit(ProcDeclNode &node) override;
    void visit(FuncDeclNode &node) override;
    void visit(IfNode &node) override;
    void visit(WhileNode &node) override;
    void visit(ForNode &node) override;
    void visit(RepeatNode &node) override;
    void visit(CaseNode &node) override;
    void visit(CaseBranchNode &node) override;
    void visit(ParamNode &node) override;
    void visit(ArrayAccessNode &node) override;
    void visit(FieldAccessNode &node) override;
    void visit(RangeNode &node) override;
    void visit(ArrayTypeNode &node) override;
    void visit(RecordTypeNode &node) override;

    // Emit an instruction (auto-assigns line number)
    void emit(Opcode op, int level, int operand = 0);

    // Label management
    int newLabel();                      // allocate a new label ID
    void placeLabel(int labelId);        // record current line as label target
    int getLabelLine(int labelId) const; // get resolved line for a label
    void backpatch(int instIndex, int line); // fill in operand of JMP/JPC

  private:
    SymbolTableManager &symTable;
    std::vector<Instruction> instructions;
    int nextLine = 0;
    int labelCounter = 0;
    std::vector<int> labelLines; // labelId → resolved instruction line

    // Map binary operator string to OPR operation number
    int opToCode(const std::string &op) const;
};

} // namespace Intermediate

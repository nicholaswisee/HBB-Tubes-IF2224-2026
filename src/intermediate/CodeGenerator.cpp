#include "../intermediate/CodeGenerator.hpp"
#include "../semantic/ASTNode.hpp"
#include "../semantic/TypeSystem.hpp"
#include <iostream>
#include <stdexcept>

namespace Intermediate {

CodeGenerator::CodeGenerator(SymbolTableManager &symTable)
    : symTable(symTable) {}

std::vector<Instruction> CodeGenerator::generate(std::shared_ptr<ASTNode> ast) {
    instructions.clear();
    nextLine = 0;
    if (ast) {
        ast->accept(*this);
    }
    return instructions;
}

const std::vector<Instruction> &CodeGenerator::getInstructions() const {
    return instructions;
}

void CodeGenerator::emit(Opcode op, int level, int operand) {
    instructions.emplace_back(nextLine, op, level, operand);
    nextLine++;
}

int CodeGenerator::newLabel() {
    int id = labelCounter++;
    if ((int)labelLines.size() <= id) {
        labelLines.resize(id + 1, -1);
    }
    return id;
}

void CodeGenerator::placeLabel(int labelId) {
    if (labelId >= 0 && labelId < (int)labelLines.size()) {
        labelLines[labelId] = nextLine;
    }
}

int CodeGenerator::getLabelLine(int labelId) const {
    if (labelId >= 0 && labelId < (int)labelLines.size()) {
        return labelLines[labelId];
    }
    return -1;
}

void CodeGenerator::backpatch(int instIndex, int line) {
    if (instIndex >= 0 && instIndex < (int)instructions.size()) {
        instructions[instIndex].operand = line;
    }
}

int CodeGenerator::opToCode(const std::string &op) const {
    if (op == "+")
        return 2; // ADD
    if (op == "-")
        return 3; // SUB
    if (op == "*")
        return 4; // MUL
    if (op == "/")
        return 5; // DIV
    if (op == "div")
        return 5; // DIV
    if (op == "mod")
        return 6; // MOD
    if (op == "==")
        return 7; // EQL
    if (op == "<>")
        return 8; // NEQ
    if (op == "<")
        return 9; // LSS
    if (op == ">=")
        return 10; // GEQ
    if (op == ">")
        return 11; // GTR
    if (op == "<=")
        return 12; // LEQ
    return 0;      // unknown
}

// Visit a literal: push its value onto the stack
void CodeGenerator::visit(LiteralNode &node) {
    int value = 0;
    try {
        value = std::stoi(node.value);
    } catch (...) {
        if (node.value == "true") {
            value = 1;
        } else if (node.value == "false") {
            value = 0;
        }
    }
    emit(Opcode::LIT, 0, value);
}

// Visit a variable: load its value from memory
void CodeGenerator::visit(VariableNode &node) {
    if (node.tabIndex < 0)
        return;
    const TabEntry &entry = symTable.getTab(node.tabIndex);
    emit(Opcode::LOD, entry.lev, entry.adr);
}

// Visit a binary operation: evaluate left, right, then apply operator
void CodeGenerator::visit(BinaryOpNode &node) {
    if (node.left)
        node.left->accept(*this);
    if (node.right)
        node.right->accept(*this);
    int opNum = opToCode(node.op);
    if (opNum > 0) {
        emit(Opcode::OPR, 0, opNum);
    }
}

// Visit a unary operation: evaluate operand, then apply operator
void CodeGenerator::visit(UnaryOpNode &node) {
    if (node.operand)
        node.operand->accept(*this);
    if (node.op == "-" || node.op == "not") {
        emit(Opcode::OPR, 0, 1); // NEG
    }
}

// Visit an assignment: evaluate value, then store to target
void CodeGenerator::visit(AssignNode &node) {
    if (node.value)
        node.value->accept(*this);
    if (node.target && node.target->tabIndex >= 0) {
        const TabEntry &entry = symTable.getTab(node.target->tabIndex);
        emit(Opcode::STO, entry.lev, entry.adr);
    }
}

// Visit a compound statement: evaluate each statement in order
void CodeGenerator::visit(CompoundNode &node) {
    for (auto &stmt : node.statements) {
        if (stmt)
            stmt->accept(*this);
    }
}

// Visit a procedure call: handle I/O and user-defined calls
void CodeGenerator::visit(ProcCallNode &node) {
    if (node.name == "writeln" || node.name == "write") {
        for (auto &arg : node.arguments) {
            if (arg)
                arg->accept(*this);
        }
        int opNum = (node.name == "writeln") ? 14 : 13;
        emit(Opcode::OPR, 0, opNum);
    } else if (node.name == "readln" || node.name == "read") {
        for (auto &arg : node.arguments) {
            if (arg && arg->tabIndex >= 0) {
                const TabEntry &entry = symTable.getTab(arg->tabIndex);
                emit(Opcode::OPR, 0, 15); // READ (bonus)
                emit(Opcode::STO, entry.lev, entry.adr);
            }
        }
    }
}

// Stubs for Role 2 implementation
void CodeGenerator::visit(ProgramNode &node) {
    if (node.body)
        node.body->accept(*this);
}

void CodeGenerator::visit(VarDeclNode &node) {
    (void)node; // no-op for expression codegen
}

void CodeGenerator::visit(ConstDeclNode &node) { (void)node; }

void CodeGenerator::visit(TypeDeclNode &node) { (void)node; }

void CodeGenerator::visit(ProcDeclNode &node) { (void)node; }

void CodeGenerator::visit(FuncDeclNode &node) { (void)node; }

void CodeGenerator::visit(IfNode &node) {
    if (node.condition)
        node.condition->accept(*this);
    if (node.thenBranch)
        node.thenBranch->accept(*this);
    if (node.elseBranch)
        node.elseBranch->accept(*this);
}

void CodeGenerator::visit(WhileNode &node) {
    if (node.condition)
        node.condition->accept(*this);
    if (node.body)
        node.body->accept(*this);
}

void CodeGenerator::visit(ForNode &node) {
    if (node.initExpr)
        node.initExpr->accept(*this);
    if (node.body)
        node.body->accept(*this);
}

void CodeGenerator::visit(RepeatNode &node) {
    for (auto &stmt : node.statements) {
        if (stmt)
            stmt->accept(*this);
    }
    if (node.condition)
        node.condition->accept(*this);
}

void CodeGenerator::visit(CaseNode &node) {
    if (node.expression)
        node.expression->accept(*this);
    for (auto &branch : node.branches) {
        if (branch)
            branch->accept(*this);
    }
}

void CodeGenerator::visit(CaseBranchNode &node) {
    for (auto &c : node.constants) {
        if (c)
            c->accept(*this);
    }
    if (node.statement)
        node.statement->accept(*this);
}

void CodeGenerator::visit(ParamNode &node) { (void)node; }

void CodeGenerator::visit(ArrayAccessNode &node) {
    if (node.arrayExpr)
        node.arrayExpr->accept(*this);
    for (auto &idx : node.indices) {
        if (idx)
            idx->accept(*this);
    }
}

void CodeGenerator::visit(FieldAccessNode &node) {
    if (node.recordExpr)
        node.recordExpr->accept(*this);
}

void CodeGenerator::visit(RangeNode &node) {
    if (node.low)
        node.low->accept(*this);
    if (node.high)
        node.high->accept(*this);
}

void CodeGenerator::visit(ArrayTypeNode &node) { (void)node; }

void CodeGenerator::visit(RecordTypeNode &node) { (void)node; }

} // namespace Intermediate

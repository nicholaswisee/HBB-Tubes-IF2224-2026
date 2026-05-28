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

} // namespace Intermediate

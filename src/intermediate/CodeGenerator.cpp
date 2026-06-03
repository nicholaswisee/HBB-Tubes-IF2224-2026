#include "../intermediate/CodeGenerator.hpp"
#include "../semantic/ASTNode.hpp"
#include "../semantic/TypeSystem.hpp"
#include <algorithm>
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
    if (node.op == "-") {
        emit(Opcode::OPR, 0, 1); // NEG
    } else if (node.op == "not") {
        emit(Opcode::OPR, 0, 16); // NOT
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
                emit(Opcode::OPR, 0, 15); // READ
                emit(Opcode::STO, entry.lev, entry.adr);
            }
        }
    } else {
        // User-defined procedure/function call
        int calleeIdx = symTable.lookup(node.name);
        if (calleeIdx >= 0) {
            const TabEntry &entry = symTable.getTab(calleeIdx);
            for (auto &arg : node.arguments) {
                if (arg) arg->accept(*this);
            }
            emit(Opcode::CAL, entry.lev, entry.adr);
        }
    }
}

// Stubs for Role 2 implementation
void CodeGenerator::assignAddresses() {
    for (int b = 0; b < symTable.btabSize(); b++) {
        BTabEntry &block = symTable.getBTab(b);

        // Collect all entries belonging to this block via linked list
        std::vector<int> indices;
        int idx = block.last;
        while (idx > 0) {
            const TabEntry &entry = symTable.getTab(idx);
            indices.push_back(idx);
            idx = entry.link;
        }
        // Reverse to get declaration order
        std::reverse(indices.begin(), indices.end());

        // Parameters are pushed by the caller before CAL and sit below
        // the new frame (negative offsets). Locals are inside the frame.
        int paramAddr = -block.psze;
        int varAddr = 3;

        for (int tabIdx : indices) {
            TabEntry &entry = symTable.getTab(tabIdx);
            if (entry.obj == TypeSystem::OBJ_VARIABLE) {
                if (entry.nrm == 0) {
                    entry.adr = paramAddr++;
                } else {
                    entry.adr = varAddr++;
                }
            }
        }

        int computedVsze = varAddr - (3 + block.psze);
        if (computedVsze > block.vsze) {
            block.vsze = computedVsze;
        }
    }
}

void CodeGenerator::visit(ProgramNode &node) {
    // Ensure all variables have addresses before generating code
    assignAddresses();

    // Hitung frame size dari btab level 0
    int blockIdx = symTable.getDisplay(0);
    BTabEntry &block = symTable.getBTab(blockIdx);
    int frameSize = 3 + block.psze + block.vsze;  // 3 untuk SL/DL/RA + parameter + variabel

    emit(Opcode::INT, 0, frameSize);

    // Proses deklarasi
    for (auto &decl : node.declarations) {
        if (decl) decl->accept(*this);
    }

    // Proses body
    if (node.body) node.body->accept(*this);

    emit(Opcode::RET, 0, 0);
}

void CodeGenerator::visit(VarDeclNode &node) {
    (void)node; // no-op for expression codegen
}

void CodeGenerator::visit(ConstDeclNode &node) { (void)node; }

void CodeGenerator::visit(TypeDeclNode &node) { (void)node; }

void CodeGenerator::visit(ProcDeclNode &node) {
    // Procedure entry point — catat line number untuk CAL
    TabEntry &entry = symTable.getTab(symTable.lookup(node.name));
    entry.adr = instructions.size();

    // Frame initialization using correct lexical level
    int blockIdx = symTable.getDisplay(entry.lev);
    BTabEntry &block = symTable.getBTab(blockIdx);
    // Parameters are pre-pushed by the caller; INT only allocates SL/DL/RA + locals
    emit(Opcode::INT, entry.lev, 3 + block.vsze);

    // Local declarations
    for (auto &decl : node.localDeclarations) {
        if (decl) decl->accept(*this);
    }

    // Body
    if (node.body) node.body->accept(*this);

    emit(Opcode::RET, 0, 0);
}

void CodeGenerator::visit(FuncDeclNode &node) {
    // Function entry point
    TabEntry &entry = symTable.getTab(symTable.lookup(node.name));
    entry.adr = instructions.size();

    // Frame initialization using correct lexical level
    int blockIdx = symTable.getDisplay(entry.lev);
    BTabEntry &block = symTable.getBTab(blockIdx);
    // Parameters are pre-pushed by the caller; INT only allocates SL/DL/RA + locals
    emit(Opcode::INT, entry.lev, 3 + block.vsze);

    // Local declarations
    for (auto &decl : node.localDeclarations) {
        if (decl) decl->accept(*this);
    }

    // Body
    if (node.body) node.body->accept(*this);

    // (Note: push return value ke stack sebelum RET di-handle pada return statement di AST, atau disini)

    emit(Opcode::RET, 0, 0);
}

void CodeGenerator::visit(IfNode &node) {
    int elseLabel = newLabel();
    int endLabel = newLabel();

    // Condition
    if (node.condition) node.condition->accept(*this);

    // Conditional jump to else
    int jpcIdx = instructions.size();
    emit(Opcode::JPC, 0, 0);  // backpatched to elseLabel

    // Then branch
    if (node.thenBranch) node.thenBranch->accept(*this);

    // Jump ke end (skip else)
    int jmpIdx = -1;
    if (node.elseBranch) {
        jmpIdx = instructions.size();
        emit(Opcode::JMP, 0, 0);  // backpatched to endLabel
    }

    // Else label
    placeLabel(elseLabel);
    backpatch(jpcIdx, getLabelLine(elseLabel));

    // Else branch
    if (node.elseBranch) node.elseBranch->accept(*this);

    // End label
    placeLabel(endLabel);
    if (jmpIdx >= 0) backpatch(jmpIdx, getLabelLine(endLabel));
}

void CodeGenerator::visit(WhileNode &node) {
    int startLabel = newLabel();
    int endLabel = newLabel();

    // Start label
    placeLabel(startLabel);

    // Condition
    if (node.condition) node.condition->accept(*this);

    // Conditional jump to end
    int jpcIdx = instructions.size();
    emit(Opcode::JPC, 0, 0);  // backpatched to endLabel

    // Body
    if (node.body) node.body->accept(*this);

    // Jump back to start
    emit(Opcode::JMP, 0, getLabelLine(startLabel));

    // End label
    placeLabel(endLabel);
    backpatch(jpcIdx, getLabelLine(endLabel));
}

void CodeGenerator::visit(ForNode &node) {
    int startLabel = newLabel();
    int endLabel = newLabel();

    // Init: assign initial value to loop variable
    if (node.initExpr) node.initExpr->accept(*this);
    int varIdx = symTable.lookup(node.varName);
    if (varIdx < 0) return; // variable not found
    TabEntry &varEntry = symTable.getTab(varIdx);
    emit(Opcode::STO, varEntry.lev, varEntry.adr);

    // Start label
    placeLabel(startLabel);

    // Check condition
    emit(Opcode::LOD, varEntry.lev, varEntry.adr);
    if (node.finalExpr) node.finalExpr->accept(*this);

    if (node.direction == "to") {
        emit(Opcode::OPR, 0, 12); // LEQ
    } else {
        emit(Opcode::OPR, 0, 10); // GEQ
    }
    int jpcIdx = instructions.size();
    emit(Opcode::JPC, 0, 0);  // backpatched to endLabel

    // Body
    if (node.body) node.body->accept(*this);

    // Increment/decrement
    emit(Opcode::LOD, varEntry.lev, varEntry.adr);
    emit(Opcode::LIT, 0, 1);
    if (node.direction == "to") {
        emit(Opcode::OPR, 0, 2);  // ADD
    } else {
        emit(Opcode::OPR, 0, 3);  // SUB
    }
    emit(Opcode::STO, varEntry.lev, varEntry.adr);

    // Jump back to condition check
    emit(Opcode::JMP, 0, getLabelLine(startLabel));

    // End label
    placeLabel(endLabel);
    backpatch(jpcIdx, getLabelLine(endLabel));
}

void CodeGenerator::visit(RepeatNode &node) {
    int startLabel = newLabel();
    placeLabel(startLabel);
    int startLine = getLabelLine(startLabel);

    // Statements
    for (auto &stmt : node.statements) {
        if (stmt) stmt->accept(*this);
    }

    // Condition
    if (node.condition) node.condition->accept(*this);

    // If false, jump back to start
    emit(Opcode::JPC, 0, startLine);
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

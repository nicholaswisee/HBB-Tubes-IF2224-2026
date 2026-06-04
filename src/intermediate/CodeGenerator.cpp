#include "../intermediate/CodeGenerator.hpp"
#include "../semantic/ASTNode.hpp"
#include "../semantic/TypeSystem.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

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
    // Constants (true, false, enum values, user-defined consts) are inlined
    // as literals; their 'adr' field stores the constant value.
    if (entry.obj == TypeSystem::OBJ_CONSTANT) {
        emit(Opcode::LIT, 0, entry.adr);
    } else {
        emit(Opcode::LOD, entry.lev, entry.adr);
    }
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

void CodeGenerator::visit(AssignNode &node) {
    // For array/record LHS, we must emit the address offset BEFORE the value,
    // because STOA pops value first (top of stack), then offset (below it).
    // Stack layout expected by STOA: bottom→top = [..., offset, value]
    if (node.target) {
        if (auto arrAcc = std::dynamic_pointer_cast<ArrayAccessNode>(node.target)) {
            // 1. Compute and push array offset
            int baseIdx = emitArrayAccess(arrAcc);
            // 2. Compute and push the RHS value
            if (node.value) node.value->accept(*this);
            // 3. Store: STOA pops value then offset
            if (baseIdx >= 0) {
                const TabEntry &entry = symTable.getTab(baseIdx);
                emit(Opcode::STOA, entry.lev, entry.adr);
            }
            return;
        } else if (auto fieldAcc = std::dynamic_pointer_cast<FieldAccessNode>(node.target)) {
            if (auto var = std::dynamic_pointer_cast<VariableNode>(fieldAcc->recordExpr)) {
                if (var->tabIndex >= 0) {
                    const TabEntry &recEntry = symTable.getTab(var->tabIndex);
                    int recBlock = recEntry.ref;
                    int fieldIdx = symTable.lookupLocal(fieldAcc->fieldName, recBlock);
                    if (fieldIdx >= 0) {
                        const TabEntry &fieldEntry = symTable.getTab(fieldIdx);
                        // 1. Push field offset
                        emit(Opcode::LIT, 0, fieldEntry.adr);
                        // 2. Push RHS value
                        if (node.value) node.value->accept(*this);
                        // 3. Store
                        emit(Opcode::STOA, recEntry.lev, recEntry.adr);
                        return;
                    }
                }
            }
        }
    }

    // Simple assignment (scalar variable or function return value)
    if (node.value) node.value->accept(*this);

    if (node.target && node.target->tabIndex >= 0) {
        const TabEntry &entry = symTable.getTab(node.target->tabIndex);
        if (entry.obj == TypeSystem::OBJ_FUNCTION) {
            int blockIdx = entry.ref;
            if (blockIdx <= 0 || blockIdx >= symTable.btabSize()) {
                blockIdx = symTable.getDisplay(entry.lev);
            }
            BTabEntry &block = symTable.getBTab(blockIdx);
            int level = entry.lev;
            if (block.lpar > 0) {
                level = symTable.getTab(block.lpar).lev;
            } else if (level == 0) {
                level = 1;
            }
            emit(Opcode::STO, level, 3);
        } else {
            emit(Opcode::STO, entry.lev, entry.adr);
        }
    }
}

void CodeGenerator::visit(CompoundNode &node) {
    for (auto &stmt : node.statements) {
        if (stmt)
            stmt->accept(*this);
    }
}

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

void CodeGenerator::assignAddresses() {
    for (int b = 0; b < symTable.btabSize(); b++) {
        BTabEntry &block = symTable.getBTab(b);

        std::unordered_set<int> paramIndices;
        int pIdx = block.lpar;
        int paramCount = 0;
        while (pIdx > 0) {
            paramIndices.insert(pIdx);
            paramCount++;
            const TabEntry &e = symTable.getTab(pIdx);
            pIdx = e.link;
        }
        block.psze = paramCount;

        // Collect all entries belonging to this block via linked list
        std::vector<int> indices;
        int idx = block.last;
        while (idx > 0) {
            const TabEntry &entry = symTable.getTab(idx);
            indices.push_back(idx);
            idx = entry.link;
        }
        std::reverse(indices.begin(), indices.end());

        bool hasProcOrFunc = false;
        bool isFunction = false;
        for (int i = 1; i < symTable.tabSize(); i++) {
            const TabEntry &e = symTable.getTab(i);
            if (e.ref == b && (e.obj == TypeSystem::OBJ_PROCEDURE || e.obj == TypeSystem::OBJ_FUNCTION)) {
                hasProcOrFunc = true;
                if (e.obj == TypeSystem::OBJ_FUNCTION) isFunction = true;
            }
        }

        bool isRecordBlock = false;
        if (!hasProcOrFunc && b > 0 && !indices.empty()) {
            bool allVars = true;
            for (int tabIdx : indices) {
                if (symTable.getTab(tabIdx).obj != TypeSystem::OBJ_VARIABLE) {
                    allVars = false;
                    break;
                }
            }
            if (allVars && paramIndices.empty()) {
                isRecordBlock = true;
            }
        }

        if (isRecordBlock) {
            int fieldAddr = 0;
            for (int tabIdx : indices) {
                TabEntry &entry = symTable.getTab(tabIdx);
                entry.adr = fieldAddr++;
            }
            block.vsze = fieldAddr;
            continue;
        }

        int paramAddr = -block.psze;
        int startVarAddr = isFunction ? 4 : 3;
        int varAddr = startVarAddr;

        for (int tabIdx : indices) {
            TabEntry &entry = symTable.getTab(tabIdx);
            if (entry.obj == TypeSystem::OBJ_VARIABLE) {
                if (paramIndices.count(tabIdx) || entry.nrm == 0) {
                    entry.nrm = 0;
                    entry.adr = paramAddr++;
                } else {
                    entry.adr = varAddr;
                    int size = 1; // default: 1 slot for scalar
                    if (entry.type == TypeSystem::TYPE_ARRAY && entry.ref >= 0 && entry.ref < symTable.atabSize()) {
                        size = symTable.getATab(entry.ref).size;
                        if (size <= 0) size = 1;
                    } else if (entry.type == TypeSystem::TYPE_RECORD && entry.ref >= 0 && entry.ref < symTable.btabSize()) {
                        // Count fields in the record block; each scalar field = 1 slot.
                        // Start at 0 (not 1!) to get the exact count.
                        int fieldCount = 0;
                        int fieldIdx = symTable.getBTab(entry.ref).last;
                        while (fieldIdx > 0) {
                            if (symTable.getTab(fieldIdx).obj == TypeSystem::OBJ_VARIABLE) {
                                fieldCount++;
                            }
                            fieldIdx = symTable.getTab(fieldIdx).link;
                        }
                        size = (fieldCount > 0) ? fieldCount : 1;
                    }
                    varAddr += size;
                }
            }
        }

        int computedVsze = varAddr - startVarAddr;
        block.vsze = computedVsze;
    }
}

void CodeGenerator::visit(ProgramNode &node) {
    assignAddresses();

    int blockIdx = symTable.getDisplay(0);
    BTabEntry &block = symTable.getBTab(blockIdx);
    int frameSize = 3 + block.psze + block.vsze;  

    emit(Opcode::INT, 0, frameSize);

    int mainLabel = newLabel();
    int jmpIdx = instructions.size();
    emit(Opcode::JMP, 0, 0); 

    for (auto &decl : node.declarations) {
        if (decl) decl->accept(*this);
    }

    placeLabel(mainLabel);
    backpatch(jmpIdx, getLabelLine(mainLabel));

    if (node.body) node.body->accept(*this);

    emit(Opcode::RET, 0, 0);
}

void CodeGenerator::visit(VarDeclNode &node) {
    (void)node; 
}

void CodeGenerator::visit(ConstDeclNode &node) { (void)node; }

void CodeGenerator::visit(TypeDeclNode &node) { (void)node; }

void CodeGenerator::visit(ProcDeclNode &node) {
    TabEntry &entry = symTable.getTab(symTable.lookup(node.name));
    entry.adr = instructions.size();

    int blockIdx = entry.ref;
    if (blockIdx <= 0 || blockIdx >= symTable.btabSize()) {
        blockIdx = symTable.getDisplay(entry.lev);
    }
    BTabEntry &block = symTable.getBTab(blockIdx);
    int level = entry.lev;
    if (block.lpar > 0) {
        level = symTable.getTab(block.lpar).lev;
    } else if (level == 0) {
        level = 1;
    }
    emit(Opcode::INT, level, 3 + block.vsze);

    for (auto &decl : node.localDeclarations) {
        if (decl) decl->accept(*this);
    }

    
    if (node.body) node.body->accept(*this);

    emit(Opcode::RET, block.psze, 0);
}

void CodeGenerator::visit(FuncDeclNode &node) {
    TabEntry &entry = symTable.getTab(symTable.lookup(node.name));
    entry.adr = instructions.size();

    int blockIdx = entry.ref;
    if (blockIdx <= 0 || blockIdx >= symTable.btabSize()) {
        blockIdx = symTable.getDisplay(entry.lev);
    }
    BTabEntry &block = symTable.getBTab(blockIdx);
    int level = entry.lev;
    if (block.lpar > 0) {
        level = symTable.getTab(block.lpar).lev;
    } else if (level == 0) {
        level = 1;
    }
    emit(Opcode::INT, level, 4 + block.vsze); // 3 (sys) + 1 (ret) + vsze

    for (auto &decl : node.localDeclarations) {
        if (decl) decl->accept(*this);
    }

    // Body
    if (node.body) node.body->accept(*this);

    // Push return value to stack
    emit(Opcode::LOD, level, 3);

    emit(Opcode::RET, block.psze, 1);
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
    int endLabel = newLabel();
    std::vector<int> bodyJmps;
    std::vector<std::pair<int, int>> jpcPatches; // (instIndex, targetLabelId)

    std::vector<std::shared_ptr<CaseBranchNode>> validBranches;
    std::vector<int> branchLabels;

    for (auto &b : node.branches) {
        auto cb = std::dynamic_pointer_cast<CaseBranchNode>(b);
        if (!cb) continue;
        validBranches.push_back(cb);
        branchLabels.push_back(newLabel());
    }

    int elseLabel = newLabel();

    for (size_t i = 0; i < validBranches.size(); ++i) {
        auto &cb = validBranches[i];
        placeLabel(branchLabels[i]);

        int nextLabel = (i + 1 < validBranches.size()) ? branchLabels[i + 1] : elseLabel;

        for (auto &c : cb->constants) {
            if (node.expression) node.expression->accept(*this);
            if (c) c->accept(*this);
            emit(Opcode::OPR, 0, 7); // EQL
            int jpcIdx = instructions.size();
            emit(Opcode::JPC, 0, 0); // if false, goto next
            jpcPatches.push_back({jpcIdx, nextLabel});
        }

        if (cb->statement) cb->statement->accept(*this);
        int jmpIdx = instructions.size();
        emit(Opcode::JMP, 0, 0);
        bodyJmps.push_back(jmpIdx);
    }

    placeLabel(elseLabel);

    placeLabel(endLabel);

    for (auto &p : jpcPatches) {
        backpatch(p.first, getLabelLine(p.second));
    }
    for (int idx : bodyJmps) {
        backpatch(idx, getLabelLine(endLabel));
    }
}

void CodeGenerator::visit(CaseBranchNode &node) {
    // Handled in visit(CaseNode)
    (void)node;
}

void CodeGenerator::visit(ParamNode &node) { (void)node; }

int CodeGenerator::emitAddressOffset(std::shared_ptr<ASTNode> target) {
    if (auto var = std::dynamic_pointer_cast<VariableNode>(target)) {
        emit(Opcode::LIT, 0, 0); // initial offset 0
        return var->tabIndex;
    } else if (auto arrAcc = std::dynamic_pointer_cast<ArrayAccessNode>(target)) {
        int baseIdx = emitAddressOffset(arrAcc->arrayExpr);
        if (baseIdx < 0) return -1;
        
        const TabEntry &baseEntry = symTable.getTab(baseIdx);
        int ref = baseEntry.ref;
        
        // Advance ref to current depth?
        // Wait, emitAddressOffset is recursive. 
        // We should instead collect all indices iteratively to avoid deep ATabEntry tracking issues!
        // Actually, if we just use a helper that collects all indices from the outer target, it's easier.
        return -1; // This branch won't be hit if we rewrite properly.
    }
    return -1;
}

int CodeGenerator::emitArrayAccess(std::shared_ptr<ASTNode> target) {
    // Peel off ArrayAccessNode layers to collect indices and find the base variable.
    std::vector<std::shared_ptr<ASTNode>> allIndices;
    std::shared_ptr<ASTNode> curr = target;
    while (auto arrAcc = std::dynamic_pointer_cast<ArrayAccessNode>(curr)) {
        allIndices.insert(allIndices.begin(), arrAcc->indices.begin(), arrAcc->indices.end());
        curr = arrAcc->arrayExpr;
    }

    auto varNode = std::dynamic_pointer_cast<VariableNode>(curr);
    if (!varNode || varNode->tabIndex < 0) {
        emit(Opcode::LIT, 0, 0); // push dummy offset so STOA/LODA always has something
        return -1;
    }

    int baseIdx = varNode->tabIndex;
    const TabEntry &baseEntry = symTable.getTab(baseIdx);
    int ref = baseEntry.ref;

    // Compute flat offset: for each dimension, offset = offset * dimSize + (index - low)
    // For the very first dimension, offset starts at 0, so the MUL is a no-op (0*anything=0).
    // We skip the initial LIT 0 + MUL and directly compute (index - low) for the first dim,
    // then accumulate for subsequent dims.
    bool first = true;
    for (auto &idxExpr : allIndices) {
        if (ref < 0 || ref >= symTable.atabSize()) break;
        const ATabEntry &ate = symTable.getATab(ref);

        if (!first) {
            // For multi-dim: multiply accumulated offset by size of this dimension
            int dimSize = ate.high - ate.low + 1;
            emit(Opcode::LIT, 0, dimSize);
            emit(Opcode::OPR, 0, 4); // MUL: offset *= dimSize
        }

        // Push index and bounds-check it
        idxExpr->accept(*this);                      // push raw index
        emit(Opcode::LIT, 0, ate.low);               // push low bound
        emit(Opcode::LIT, 0, ate.high);              // push high bound
        emit(Opcode::OPR, 0, 17);                    // BOUNDS_CHECK: pops high,low,idx; pushes idx back

        // Subtract lower bound to get 0-based offset
        emit(Opcode::LIT, 0, ate.low);
        emit(Opcode::OPR, 0, 3); // SUB: (index - low)

        if (!first) {
            emit(Opcode::OPR, 0, 2); // ADD: offset = offset*dimSize + (index-low)
        }

        // Scale by element size
        if (ate.elsz > 1) {
            emit(Opcode::LIT, 0, ate.elsz);
            emit(Opcode::OPR, 0, 4); // MUL
        }

        first = false;
        ref = ate.eref;
    }

    // If no indices were processed (shouldn't happen), push 0 as fallback offset
    if (first) {
        emit(Opcode::LIT, 0, 0);
    }

    return baseIdx;
}

void CodeGenerator::visit(ArrayAccessNode &node) {
    int baseIdx = emitArrayAccess(std::make_shared<ArrayAccessNode>(node));
    if (baseIdx >= 0) {
        const TabEntry &entry = symTable.getTab(baseIdx);
        emit(Opcode::LODA, entry.lev, entry.adr);
    }
}

void CodeGenerator::visit(FieldAccessNode &node) {
    if (!node.recordExpr) return;

    if (auto var = std::dynamic_pointer_cast<VariableNode>(node.recordExpr)) {
        if (var->tabIndex >= 0) {
            const TabEntry &recEntry = symTable.getTab(var->tabIndex);
            int recBlock = recEntry.ref;
            int fieldIdx = symTable.lookupLocal(node.fieldName, recBlock);
            if (fieldIdx >= 0) {
                const TabEntry &fieldEntry = symTable.getTab(fieldIdx);
                emit(Opcode::LIT, 0, fieldEntry.adr);           // push field offset
                emit(Opcode::LODA, recEntry.lev, recEntry.adr); // load from base+offset
                return;
            }
        }
    }
    // fallback
    if (node.recordExpr) node.recordExpr->accept(*this);
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

#include "Interpreter.hpp"
#include <climits>
#include <limits>

using Intermediate::Opcode;
using Intermediate::Instruction;

Interpreter::Interpreter(StackMachine &stack)
    : stack(stack), ip(0), currentInstructions(nullptr) {}

void Interpreter::execute(const std::vector<Instruction> &instructions) {
    ip = 0;
    errors.clear();
    stack.clear();
    currentInstructions = &instructions;

    while (ip >= 0 && ip < (int)instructions.size()) {
        const Instruction &inst = instructions[ip];
        try {
            switch (inst.opcode) {
                case Opcode::INT: handleINT(inst); break;
                case Opcode::LIT: handleLIT(inst); break;
                case Opcode::LOD: handleLOD(inst); break;
                case Opcode::STO: handleSTO(inst); break;
                case Opcode::CAL: handleCAL(inst); break;
                case Opcode::JMP: handleJMP(inst); break;
                case Opcode::JPC: handleJPC(inst); break;
                case Opcode::OPR: handleOPR(inst); break;
                case Opcode::RET: handleRET(inst); break;
            }
        } catch (const RuntimeError &e) {
            errors.push_back(e);
            return; 
        }

        ip++;  
    }
}

bool Interpreter::hasErrors() const {
    return !errors.empty();
}

const std::vector<RuntimeError>& Interpreter::getErrors() const {
    return errors;
}

void Interpreter::handleINT(const Instruction &inst) {
    int frameSize = inst.operand;
    int staticLink = (inst.level == 0) ? 0 : stack.getDisplay(inst.level - 1);
    int returnAddr;
    if (!returnAddrStack.empty()) {
        returnAddr = returnAddrStack.back();
        returnAddrStack.pop_back();
    } else {
        returnAddr = (int)currentInstructions->size();
    }
    stack.pushFrame(frameSize, staticLink, returnAddr);
    stack.setDisplay(inst.level, stack.currentFrameBase());
}

void Interpreter::handleLIT(const Instruction &inst) {
    stack.push(inst.operand);
}

void Interpreter::handleLOD(const Instruction &inst) {
    int value = stack.load(inst.level, inst.operand);
    stack.push(value);
}

void Interpreter::handleSTO(const Instruction &inst) {
    int value = stack.pop();
    stack.store(inst.level, inst.operand, value);
}

void Interpreter::handleJMP(const Instruction &inst) {
    if (!currentInstructions || inst.operand < 0 || inst.operand >= (int)currentInstructions->size()) {
        throw RuntimeError(RuntimeError::INVALID_JUMP,
            "Invalid jump target: line " + std::to_string(inst.operand), ip);
    }
    ip = inst.operand - 1;  // -1 because main loop will ip++
}

void Interpreter::handleJPC(const Instruction &inst) {
    if (!currentInstructions || inst.operand < 0 || inst.operand >= (int)currentInstructions->size()) {
        throw RuntimeError(RuntimeError::INVALID_JUMP,
            "Invalid jump target: line " + std::to_string(inst.operand), ip);
    }
    int cond = stack.pop();
    if (cond == 0) {
        ip = inst.operand - 1;  // jump
    }
}

void Interpreter::handleRET(const Instruction &inst) {
    (void)inst; // unused
    int frameBase = stack.currentFrameBase();
    const auto &stk = stack.getStack();
    if (frameBase < 0 || frameBase + 2 >= (int)stk.size()) {
        throw RuntimeError(RuntimeError::STACK_CORRUPTION,
            "RET: invalid frame base or stack too small", ip);
    }
    int returnAddr = stk[frameBase + 2];  
    stack.popFrame();
    ip = returnAddr - 1;  
}

void Interpreter::handleCAL(const Instruction &inst) {
    int targetLine = inst.operand;
    if (!currentInstructions || targetLine < 0 || targetLine >= (int)currentInstructions->size()) {
        throw RuntimeError(RuntimeError::INVALID_JUMP,
            "Invalid call target: line " + std::to_string(targetLine), ip);
    }
    returnAddrStack.push_back(ip + 1);
    ip = targetLine - 1;  
}

void Interpreter::handleOPR(const Instruction &inst) {
    switch (inst.operand) {
        case 1: { // NEG
            int a = stack.pop();
            stack.push(-a);
            break;
        }
        case 2: { // ADD
            int b = stack.pop();
            int a = stack.pop();
            if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
                throw RuntimeError(RuntimeError::NUMERIC_OVERFLOW,
                    "Integer overflow in addition", ip);
            }
            stack.push(a + b);
            break;
        }
        case 3: { // SUB
            int b = stack.pop();
            int a = stack.pop();
            if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b)) {
                throw RuntimeError(RuntimeError::NUMERIC_OVERFLOW,
                    "Integer overflow in subtraction", ip);
            }
            stack.push(a - b);
            break;
        }
        case 4: { // MUL
            int b = stack.pop();
            int a = stack.pop();
            if (a != 0 && b != 0) {
                if ((a > 0 && b > 0 && a > INT_MAX / b) ||
                    (a < 0 && b < 0 && a < INT_MAX / b) ||
                    (a > 0 && b < 0 && b < INT_MIN / a) ||
                    (a < 0 && b > 0 && a < INT_MIN / b)) {
                    throw RuntimeError(RuntimeError::NUMERIC_OVERFLOW,
                        "Integer overflow in multiplication", ip);
                }
            }
            stack.push(a * b);
            break;
        }
        case 5: { // DIV
            int b = stack.pop();
            int a = stack.pop();
            if (b == 0) {
                throw RuntimeError(RuntimeError::DIVISION_BY_ZERO,
                    "Division by zero", ip);
            }
            stack.push(a / b);
            break;
        }
        case 6: { // MOD
            int b = stack.pop();
            int a = stack.pop();
            if (b == 0) {
                throw RuntimeError(RuntimeError::DIVISION_BY_ZERO,
                    "Modulo by zero", ip);
            }
            stack.push(a % b);
            break;
        }
        case 7: { // EQL
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a == b ? 1 : 0);
            break;
        }
        case 8: { // NEQ
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a != b ? 1 : 0);
            break;
        }
        case 9: { // LSS
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a < b ? 1 : 0);
            break;
        }
        case 10: { // GEQ
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a >= b ? 1 : 0);
            break;
        }
        case 11: { // GTR
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a > b ? 1 : 0);
            break;
        }
        case 12: { // LEQ
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a <= b ? 1 : 0);
            break;
        }
        case 13: { // WRT
            int a = stack.pop();
            std::cout << a;
            break;
        }
        case 14: { // WRTLN
            int a = stack.pop();
            std::cout << a << std::endl;
            break;
        }
        case 15: { // READ
            int val = 0;
            if (!(std::cin >> val)) {
                val = 0;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            stack.push(val);
            break;
        }
        case 16: { // NOT
            int a = stack.pop();
            stack.push(a == 0 ? 1 : 0);
            break;
        }
        default:
            throw RuntimeError(RuntimeError::UNKNOWN_INSTRUCTION,
                "Unknown OPR operation: " + std::to_string(inst.operand), ip);
    }
}

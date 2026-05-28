#pragma once

#include <string>

namespace Intermediate {

enum class Opcode {
    INT,    // Initiate Memory: allocate frame of size operand
    LIT,    // Load Literal: push operand onto stack
    LOD,    // Load Value: push value from address (level, operand) onto stack
    STO,    // Store Value: pop value and store to address (level, operand)
    CAL,    // Call: jump to procedure at instruction line operand
    JMP,    // Unconditional Jump: set IP to operand
    JPC,    // Conditional Jump: pop value; if 0, set IP to operand
    OPR,    // Operation: execute operation number operand
    RET     // Return: exit current procedure
};

std::string opcodeToString(Opcode op);

struct Instruction {
    int line;       // instruction index (auto-assigned by emit)
    Opcode opcode;
    int level;      // static nesting level
    int operand;    // literal value, address, operation number, or target line

    Instruction(int ln, Opcode op, int lv, int opr)
        : line(ln), opcode(op), level(lv), operand(opr) {}

    std::string toString() const;
};

} // namespace Intermediate

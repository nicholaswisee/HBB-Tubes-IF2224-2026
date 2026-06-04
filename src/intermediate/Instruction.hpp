#pragma once

#include <string>

namespace Intermediate {

enum class Opcode {
    INT,    
    LIT,  
    LOD,    
    STO,   
    CAL,    
    JMP,   
    JPC,    
    OPR,    
    RET,   
    LODA,   
    STOA    
};

std::string opcodeToString(Opcode op);

struct Instruction {
    int line;      
    Opcode opcode;
    int level;      
    int operand;   

    Instruction(int ln, Opcode op, int lv, int opr)
        : line(ln), opcode(op), level(lv), operand(opr) {}

    std::string toString() const;
};

} // namespace Intermediate

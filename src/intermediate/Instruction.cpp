#include "Instruction.hpp"
#include <sstream>

namespace Intermediate {

std::string opcodeToString(Opcode op) {
    switch (op) {
    case Opcode::INT:
        return "INT";
    case Opcode::LIT:
        return "LIT";
    case Opcode::LOD:
        return "LOD";
    case Opcode::STO:
        return "STO";
    case Opcode::CAL:
        return "CAL";
    case Opcode::JMP:
        return "JMP";
    case Opcode::JPC:
        return "JPC";
    case Opcode::OPR:
        return "OPR";
    case Opcode::RET:
        return "RET";
    }
    return "???";
}

std::string Instruction::toString() const {
    std::ostringstream oss;
    oss << line << " " << opcodeToString(opcode) << " " << level << " "
        << operand;
    return oss.str();
}

} // namespace Intermediate

#include "semantic/TypeSystem.hpp"

namespace TypeSystem {

bool isCompatible(int t1, int t2) {
    if (t1 == t2) return true;
    if ((t1 == TYPE_INTEGER && t2 == TYPE_SUBRANGE) ||
        (t1 == TYPE_SUBRANGE && t2 == TYPE_INTEGER))
        return true;
    return false;
}

bool isAssignmentCompatible(int target, int value) {
    if (target == value) return true;
    if (target == TYPE_REAL && value == TYPE_INTEGER) return true;
    if (target == TYPE_REAL && value == TYPE_SUBRANGE) return true;
    if (isCompatible(target, value)) return true;
    return false;
}

int typeNameToCode(const std::string& name) {
    if (name == "integer" || name == "Integer") return TYPE_INTEGER;
    if (name == "real" || name == "Real") return TYPE_REAL;
    if (name == "char" || name == "Char") return TYPE_CHAR;
    if (name == "boolean" || name == "Boolean") return TYPE_BOOLEAN;
    if (name == "string" || name == "String") return TYPE_STRING;
    return TYPE_UNKNOWN;
}

std::string codeToTypeName(int code) {
    switch (code) {
        case TYPE_INTEGER: return "Integer";
        case TYPE_REAL: return "Real";
        case TYPE_CHAR: return "Char";
        case TYPE_BOOLEAN: return "Boolean";
        case TYPE_STRING: return "String";
        case TYPE_ARRAY: return "Array";
        case TYPE_RECORD: return "Record";
        case TYPE_SUBRANGE: return "Subrange";
        case TYPE_ENUM: return "Enumerated";
        default: return "Unknown";
    }
}

} 

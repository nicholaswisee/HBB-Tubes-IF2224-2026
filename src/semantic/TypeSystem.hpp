#pragma once

#include <string>
#include <vector>

namespace Semantic {

// Type codes for basic types
// Index 0 is reserved (undefined/error)
// Index 1-32 are reserved for predefined types and keywords
constexpr int TYPE_UNDEFINED = 0;
constexpr int TYPE_INTEGER   = 1;
constexpr int TYPE_REAL      = 2;
constexpr int TYPE_CHAR      = 3;
constexpr int TYPE_BOOLEAN   = 4;
constexpr int TYPE_STRING    = 5;

// Object classes (obj field in tab)
constexpr int OBJ_CONSTANT   = 0;
constexpr int OBJ_VARIABLE   = 1;
constexpr int OBJ_TYPE       = 2;
constexpr int OBJ_PROCEDURE  = 3;
constexpr int OBJ_FUNCTION   = 4;
constexpr int OBJ_PROGRAM    = 5;
constexpr int OBJ_PARAMETER  = 6;  // Formal parameter
constexpr int OBJ_FIELD      = 7;  // Record field

// Standard identifiers start at index 33 (0-32 reserved)
constexpr int RESERVED_WORDS_COUNT = 33;

// Convert type code to string name
inline std::string typeToString(int type) {
    switch (type) {
        case TYPE_INTEGER: return "Integer";
        case TYPE_REAL:    return "Real";
        case TYPE_CHAR:    return "Char";
        case TYPE_BOOLEAN: return "Boolean";
        case TYPE_STRING:  return "String";
        case TYPE_UNDEFINED: return "Undefined";
        default:           return "Custom(" + std::to_string(type) + ")";
    }
}

// Convert object class to string
inline std::string objToString(int obj) {
    switch (obj) {
        case OBJ_CONSTANT:  return "Constant";
        case OBJ_VARIABLE:  return "Variable";
        case OBJ_TYPE:      return "Type";
        case OBJ_PROCEDURE: return "Procedure";
        case OBJ_FUNCTION:  return "Function";
        case OBJ_PROGRAM:   return "Program";
        case OBJ_PARAMETER: return "Parameter";
        case OBJ_FIELD:     return "Field";
        default:            return "Unknown(" + std::to_string(obj) + ")";
    }
}

// Type compatibility checking
class TypeSystem {
public:
    // Check if two types are compatible (for comparison, etc.)
    static bool isCompatible(int type1, int type2);
    
    // Check if valueType can be assigned to targetType
    static bool isAssignmentCompatible(int targetType, int valueType);
    
    // Check if type is numeric (Integer or Real)
    static bool isNumeric(int type);
    
    // Check if type is ordinal (Integer, Char, Boolean, or subrange)
    static bool isOrdinal(int type);
    
    // Check if type is simple (not array or record)
    static bool isSimple(int type);
    
    // Get result type of binary operation
    // Returns TYPE_UNDEFINED if operation is invalid
    static int getBinaryOpResultType(const std::string& op, int leftType, int rightType);
    
    // Get result type of unary operation
    static int getUnaryOpResultType(const std::string& op, int operandType);
    
    // Check if operator is arithmetic (+, -, *, /, div, mod)
    static bool isArithmeticOp(const std::string& op);
    
    // Check if operator is relational (=, <>, <, <=, >, >=)
    static bool isRelationalOp(const std::string& op);
    
    // Check if operator is logical (and, or)
    static bool isLogicalOp(const std::string& op);
};

} // namespace Semantic

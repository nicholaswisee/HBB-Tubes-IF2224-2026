#include "TypeSystem.hpp"

namespace Semantic {

bool TypeSystem::isCompatible(int type1, int type2) {
    // Rule 1: Same type is always compatible
    if (type1 == type2) {
        return true;
    }
    
    // Note: Integer and Real are NOT considered "compatible" in the general sense
    // They are comparable for relational operations (handled separately)
    // but not compatible for general use
    
    // Rule 2: Subrange compatibility (to be implemented when subrange is added)
    // A subrange is compatible with its base type and other subranges of the same base
    
    return false;
}

bool TypeSystem::isAssignmentCompatible(int targetType, int valueType) {
    // Rule i: Real := Integer is allowed
    if (targetType == TYPE_REAL && valueType == TYPE_INTEGER) {
        return true;
    }
    
    // Rule ii: Same types are always assignment compatible
    if (targetType == valueType) {
        return true;
    }
    
    // Integer := Real is NOT allowed (opposite of Rule i)
    // This must be explicitly rejected
    if (targetType == TYPE_INTEGER && valueType == TYPE_REAL) {
        return false;
    }
    
    // Rule iii: Subrange assignments (value must be within subrange bounds)
    // This requires additional checks during semantic analysis
    
    return false;
}

bool TypeSystem::isNumeric(int type) {
    return type == TYPE_INTEGER || type == TYPE_REAL;
}

bool TypeSystem::isOrdinal(int type) {
    return type == TYPE_INTEGER || type == TYPE_CHAR || type == TYPE_BOOLEAN;
}

bool TypeSystem::isSimple(int type) {
    // For now, basic types are simple
    // Arrays and records will be handled separately
    return type == TYPE_INTEGER || type == TYPE_REAL || 
           type == TYPE_CHAR || type == TYPE_BOOLEAN || type == TYPE_STRING;
}

int TypeSystem::getBinaryOpResultType(const std::string& op, int leftType, int rightType) {
    // Arithmetic operators: +, -, *, /, div, mod
    if (isArithmeticOp(op)) {
        if (!isNumeric(leftType) || !isNumeric(rightType)) {
            return TYPE_UNDEFINED;  // Error: operands must be numeric
        }
        
        // Division (/) always returns Real
        if (op == "/") {
            return TYPE_REAL;
        }
        
        // Integer division (div) and modulus (mod) return Integer
        if (op == "div" || op == "mod") {
            if (leftType == TYPE_INTEGER && rightType == TYPE_INTEGER) {
                return TYPE_INTEGER;
            }
            return TYPE_UNDEFINED;  // Error: div/mod require integer operands
        }
        
        // If either operand is Real, result is Real
        if (leftType == TYPE_REAL || rightType == TYPE_REAL) {
            return TYPE_REAL;
        }
        
        // Otherwise result is Integer
        return TYPE_INTEGER;
    }
    
    // Relational operators: =, <>, <, <=, >, >=
    if (isRelationalOp(op)) {
        // For relational operators, numeric types can be compared
        bool bothNumeric = isNumeric(leftType) && isNumeric(rightType);
        bool sameType = (leftType == rightType);
        bool comparable = bothNumeric || sameType;
        
        if (!comparable) {
            return TYPE_UNDEFINED;  // Error: incompatible types for comparison
        }
        // Relational operators always return Boolean
        return TYPE_BOOLEAN;
    }
    
    // Logical operators: and, or
    if (isLogicalOp(op)) {
        if (leftType != TYPE_BOOLEAN || rightType != TYPE_BOOLEAN) {
            return TYPE_UNDEFINED;  // Error: logical operators require Boolean operands
        }
        return TYPE_BOOLEAN;
    }
    
    return TYPE_UNDEFINED;  // Unknown operator
}

int TypeSystem::getUnaryOpResultType(const std::string& op, int operandType) {
    // Unary plus and minus
    if (op == "+" || op == "-") {
        if (!isNumeric(operandType)) {
            return TYPE_UNDEFINED;  // Error: unary +/- requires numeric operand
        }
        return operandType;  // Same type as operand
    }
    
    // Logical not
    if (op == "not") {
        if (operandType != TYPE_BOOLEAN) {
            return TYPE_UNDEFINED;  // Error: not requires Boolean operand
        }
        return TYPE_BOOLEAN;
    }
    
    return TYPE_UNDEFINED;  // Unknown operator
}

bool TypeSystem::isArithmeticOp(const std::string& op) {
    return op == "+" || op == "-" || op == "*" || op == "/" || 
           op == "div" || op == "mod";
}

bool TypeSystem::isRelationalOp(const std::string& op) {
    return op == "=" || op == "<>" || op == "<" || op == "<=" || 
           op == ">" || op == ">=";
}

bool TypeSystem::isLogicalOp(const std::string& op) {
    return op == "and" || op == "or";
}

} // namespace Semantic

#pragma once

#include <string>

namespace TypeSystem {
constexpr int TYPE_UNKNOWN = 0;
constexpr int TYPE_INTEGER = 1;
constexpr int TYPE_REAL = 2;
constexpr int TYPE_CHAR = 3;
constexpr int TYPE_BOOLEAN = 4;
constexpr int TYPE_STRING = 5;
constexpr int TYPE_ARRAY = 6;
constexpr int TYPE_RECORD = 7;
constexpr int TYPE_SUBRANGE = 8;
constexpr int TYPE_ENUM = 9;

constexpr int OBJ_UNKNOWN = 0;
constexpr int OBJ_CONSTANT = 1;
constexpr int OBJ_VARIABLE = 2;
constexpr int OBJ_TYPE = 3;
constexpr int OBJ_PROCEDURE = 4;
constexpr int OBJ_FUNCTION = 5;

inline bool isNumeric(int type) {
    return type == TYPE_INTEGER || type == TYPE_REAL || type == TYPE_SUBRANGE;
}

inline bool isSimple(int type) {
    return type == TYPE_INTEGER || type == TYPE_REAL || type == TYPE_CHAR ||
           type == TYPE_BOOLEAN || type == TYPE_STRING ||
           type == TYPE_SUBRANGE || type == TYPE_ENUM;
}

inline bool isComparable(int type) {
    return isSimple(type) && type != TYPE_ARRAY && type != TYPE_RECORD;
}

bool isCompatible(int t1, int t2);
bool isAssignmentCompatible(int target, int value);
int typeNameToCode(const std::string &name);
std::string codeToTypeName(int code);
} // namespace TypeSystem

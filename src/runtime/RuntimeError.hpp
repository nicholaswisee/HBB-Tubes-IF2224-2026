#pragma once

#include <string>

struct RuntimeError {
    enum Type {
        STACK_OVERFLOW,
        STACK_UNDERFLOW,
        DIVISION_BY_ZERO,
        INVALID_JUMP,
        INDEX_OUT_OF_BOUNDS,
        NUMERIC_OVERFLOW,
        TYPE_MISMATCH,
        UNKNOWN_INSTRUCTION,
        STACK_CORRUPTION
    };

    Type type;
    std::string message;
    int line;  

    RuntimeError(Type type, const std::string &msg, int line = 0)
        : type(type), message(msg), line(line) {}
};

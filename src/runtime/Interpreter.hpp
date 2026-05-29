#pragma once

#include "../intermediate/Instruction.hpp"
#include "StackMachine.hpp"
#include "RuntimeError.hpp"
#include <vector>
#include <iostream>

class Interpreter {
public:
    Interpreter(StackMachine &stack);

    // Execute a list of instructions
    void execute(const std::vector<Intermediate::Instruction> &instructions);

    // Get any runtime errors
    bool hasErrors() const;
    const std::vector<RuntimeError>& getErrors() const;

private:
    StackMachine &stack;
    int ip;  // Instruction Pointer
    std::vector<RuntimeError> errors;
    const std::vector<Intermediate::Instruction>* currentInstructions;
    std::vector<int> returnAddrStack; // for CAL/INT coordination

    // Instruction handlers
    void handleINT(const Intermediate::Instruction &inst);
    void handleLIT(const Intermediate::Instruction &inst);
    void handleLOD(const Intermediate::Instruction &inst);
    void handleSTO(const Intermediate::Instruction &inst);
    void handleCAL(const Intermediate::Instruction &inst);
    void handleJMP(const Intermediate::Instruction &inst);
    void handleJPC(const Intermediate::Instruction &inst);
    void handleOPR(const Intermediate::Instruction &inst);   // → dispatch to Role 4
    void handleRET(const Intermediate::Instruction &inst);
};

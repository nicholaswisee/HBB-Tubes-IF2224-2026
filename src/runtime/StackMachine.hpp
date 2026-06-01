#pragma once

#include "../intermediate/Instruction.hpp"
#include "RuntimeError.hpp"
#include <vector>
#include <string>
#include <stdexcept>

class StackMachine {
public:
    StackMachine(int maxFrames = 1000);

    // Stack operations
    void push(int value);
    int pop();
    int top() const;
    bool isEmpty() const;

    // Frame operations
    void pushFrame(int frameSize, int staticLink, int returnAddr);
    void popFrame();
    int currentFrameBase() const;
    int getStaticLink(int level) const;

    // Memory access (via static link chain)
    void store(int level, int addr, int value);
    int  load(int level, int addr);

    // Display
    void setDisplay(int level, int basePtr);
    int getDisplay(int level) const;

    // Status
    int frameCount() const;
    void clear();

    const std::vector<int>& getStack() const;

private:
    std::vector<int> stack;       // the main stack memory
    std::vector<int> display;     // display[level] = base pointer
    int bp;                       // base pointer (current frame base)
    int sp;                       // stack pointer (top of stack)
    int maxFrames;                // for stack overflow detection
    int currentFrames;            // current frame count

    // Internal helpers
    int findFrame(int level) const;  // follow static links to find frame at level
};

#include "StackMachine.hpp"

StackMachine::StackMachine(int maxFrames)
    : bp(-1), sp(-1), maxFrames(maxFrames), currentFrames(0) {}

void StackMachine::push(int value) {
    stack.push_back(value);
    sp++;
}

int StackMachine::pop() {
    if (stack.empty()) {
        throw RuntimeError(RuntimeError::STACK_UNDERFLOW,
            "Stack underflow: attempted to pop from empty stack");
    }
    int val = stack.back();
    stack.pop_back();
    sp--;
    return val;
}

int StackMachine::top() const {
    if (stack.empty()) {
        throw RuntimeError(RuntimeError::STACK_UNDERFLOW,
            "Stack underflow: attempted to peek empty stack");
    }
    return stack.back();
}

bool StackMachine::isEmpty() const {
    return stack.empty();
}

void StackMachine::pushFrame(int frameSize, int staticLink, int returnAddr) {
    if (currentFrames >= maxFrames) {
        throw RuntimeError(RuntimeError::STACK_OVERFLOW,
            "Stack overflow: maximum frame depth (" +
            std::to_string(maxFrames) + ") exceeded");
    }
    push(staticLink);
    push(bp);         
    push(returnAddr);
    for (int i = 3; i < frameSize; i++) {
        push(0);
    }
    bp = sp - frameSize + 1;
    currentFrames++;
}

void StackMachine::popFrame() {
    if (currentFrames <= 0) {
        throw RuntimeError(RuntimeError::STACK_CORRUPTION,
            "Stack corruption: no frame to pop");
    }
    int dynLink = stack[bp + 1];
    int frameSize = sp - bp + 1;
    for (int i = 0; i < frameSize; i++) pop();
    bp = dynLink;
    currentFrames--;
}

int StackMachine::currentFrameBase() const {
    return bp;
}

int StackMachine::getStaticLink(int level) const {
    if (level < 0 || level >= (int)display.size()) {
        return 0;
    }
    return display[level];
}

void StackMachine::store(int level, int addr, int value) {
    int frameBP = findFrame(level);
    int absAddr = frameBP + addr;

    if (frameBP == bp && addr >= 0 && addr <= 2) {
        throw RuntimeError(RuntimeError::STACK_CORRUPTION,
            "Stack smashing detected: attempted to overwrite protected frame slot " + std::to_string(addr));
    }

    if (absAddr < 0 || absAddr >= (int)stack.size()) {
        throw RuntimeError(RuntimeError::INDEX_OUT_OF_BOUNDS,
            "Memory access out of bounds: address " + std::to_string(absAddr));
    }
    stack[absAddr] = value;
}

int StackMachine::load(int level, int addr) {
    int frameBP = findFrame(level);
    int absAddr = frameBP + addr;
    if (absAddr < 0 || absAddr >= (int)stack.size()) {
        throw RuntimeError(RuntimeError::INDEX_OUT_OF_BOUNDS,
            "Memory access out of bounds: address " + std::to_string(absAddr));
    }
    return stack[absAddr];
}

void StackMachine::setDisplay(int level, int basePtr) {
    if (level >= (int)display.size()) {
        display.resize(level + 1, 0);
    }
    display[level] = basePtr;
}

int StackMachine::getDisplay(int level) const {
    if (level < 0 || level >= (int)display.size()) {
        return 0;
    }
    return display[level];
}

int StackMachine::frameCount() const {
    return currentFrames;
}

void StackMachine::clear() {
    stack.clear();
    display.clear();
    bp = -1;
    sp = -1;
    currentFrames = 0;
}

const std::vector<int>& StackMachine::getStack() const {
    return stack;
}

int StackMachine::findFrame(int level) const {
    // Determine current level from display size
    int currentLevel = (int)display.size() - 1;
    int bpCursor = bp;
    while (currentLevel > level) {
        if (bpCursor < 0 || bpCursor >= (int)stack.size()) {
            throw RuntimeError(RuntimeError::STACK_CORRUPTION,
                "Static link chain corrupted during frame lookup");
        }
        bpCursor = stack[bpCursor];  
        currentLevel--;
    }
    return bpCursor;
}

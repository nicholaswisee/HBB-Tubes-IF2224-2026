#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include "../src/runtime/StackMachine.hpp"
#include "../src/runtime/Interpreter.hpp"
#include "../src/intermediate/Instruction.hpp"

using Intermediate::Opcode;
using Intermediate::Instruction;

// Helper to build instruction list with auto line numbers
std::vector<Instruction> makeInstructions(std::vector<std::tuple<Opcode, int, int>> ops) {
    std::vector<Instruction> insts;
    int line = 0;
    for (auto &op : ops) {
        insts.emplace_back(line++, std::get<0>(op), std::get<1>(op), std::get<2>(op));
    }
    return insts;
}

// ============================================
// Test 1: StackMachine push/pop
// ============================================
void test_stack_push_pop() {
    StackMachine sm;
    sm.push(10);
    sm.push(20);
    assert(sm.pop() == 20);
    assert(sm.pop() == 10);
    assert(sm.isEmpty());
    std::cout << "[PASS] test_stack_push_pop\n";
}

// ============================================
// Test 2: StackMachine underflow
// ============================================
void test_stack_underflow() {
    StackMachine sm;
    bool caught = false;
    try {
        sm.pop();
    } catch (const RuntimeError &e) {
        caught = true;
        assert(e.type == RuntimeError::STACK_UNDERFLOW);
    }
    assert(caught);
    std::cout << "[PASS] test_stack_underflow\n";
}

// ============================================
// Test 3: Frame push/pop and memory access
// ============================================
void test_frame_and_memory() {
    StackMachine sm;
    // Push frame of size 5 (SL, DL, RA, var1, var2)
    sm.pushFrame(5, 0, 99);
    assert(sm.frameCount() == 1);
    assert(sm.currentFrameBase() == 0);

    // Store to local variables (addr 3 and 4)
    sm.store(0, 3, 42);
    sm.store(0, 4, 77);

    // Load back
    assert(sm.load(0, 3) == 42);
    assert(sm.load(0, 4) == 77);

    // Pop frame
    sm.popFrame();
    assert(sm.frameCount() == 0);
    std::cout << "[PASS] test_frame_and_memory\n";
}

// ============================================
// Test 4: Static link chain / nested frames
// ============================================
void test_static_link_chain() {
    StackMachine sm;
    // Global frame (level 0)
    sm.pushFrame(5, 0, 0);
    sm.setDisplay(0, sm.currentFrameBase());
    sm.store(0, 3, 100); // global var at addr 3
    sm.store(0, 4, 200); // global var at addr 4

    // Procedure frame (level 1), static link = global's base
    int globalBase = sm.currentFrameBase();
    sm.pushFrame(4, globalBase, 10);
    sm.setDisplay(1, sm.currentFrameBase());

    // Access global variables from level 1 through static link
    assert(sm.load(0, 3) == 100);
    assert(sm.load(0, 4) == 200);

    // Access local variable in level 1
    sm.store(1, 3, 55);
    assert(sm.load(1, 3) == 55);

    sm.popFrame();
    sm.popFrame();
    std::cout << "[PASS] test_static_link_chain\n";
}

// ============================================
// Test 5: Interpreter LIT + STO + LOD
// ============================================
void test_interpreter_lit_sto_lod() {
    StackMachine sm;
    Interpreter interp(sm);

    auto prog = makeInstructions({
        {Opcode::INT, 0, 4},  // frame size 4: SL,DL,RA,var
        {Opcode::LIT, 0, 42}, // push 42
        {Opcode::STO, 0, 3},  // store to local var addr 3
        {Opcode::LOD, 0, 3},  // load from local var addr 3
        // no RET: loop ends naturally when ip reaches end
    });

    interp.execute(prog);
    if (interp.hasErrors()) {
        std::cerr << "Error in test_interpreter_lit_sto_lod: " << interp.getErrors()[0].message << "\n";
    }
    assert(!interp.hasErrors());
    // After execution, 42 should be left on stack (from LOD)
    assert(!sm.isEmpty());
    assert(sm.top() == 42);
    std::cout << "[PASS] test_interpreter_lit_sto_lod\n";
}

// ============================================
// Test 6: Interpreter JMP (unconditional jump)
// ============================================
void test_interpreter_jmp() {
    StackMachine sm;
    Interpreter interp(sm);

    auto prog = makeInstructions({
        {Opcode::JMP, 0, 3},  // jump to line 3
        {Opcode::LIT, 0, 1}, // line 1: skipped
        {Opcode::LIT, 0, 2}, // line 2: skipped
        {Opcode::LIT, 0, 3}, // line 3: push 3
        // loop ends naturally
    });

    interp.execute(prog);
    assert(!interp.hasErrors());
    assert(sm.top() == 3);
    std::cout << "[PASS] test_interpreter_jmp\n";
}

// ============================================
// Test 7: Interpreter JPC (conditional jump)
// ============================================
void test_interpreter_jpc() {
    StackMachine sm;
    Interpreter interp(sm);

    // Case: condition 0 (false) → jump to line 5 (past then-branch)
    auto progFalse = makeInstructions({
        {Opcode::INT, 0, 3},  // line 0: main frame
        {Opcode::LIT, 0, 0},  // line 1: push 0
        {Opcode::JPC, 0, 5},  // line 2: if 0 jump to line 5
        {Opcode::LIT, 0, 99}, // line 3: then branch (skipped)
        {Opcode::JMP, 0, 6},  // line 4: jump to end
        {Opcode::LIT, 0, 11}, // line 5: else branch
        {Opcode::RET, 0, 0},  // line 6: return
    });

    interp.execute(progFalse);
    assert(!interp.hasErrors());

    // Case: condition 1 (true) → no jump
    StackMachine sm2;
    Interpreter interp2(sm2);
    auto progTrue = makeInstructions({
        {Opcode::INT, 0, 3},  // line 0: main frame
        {Opcode::LIT, 0, 1},  // line 1: push 1
        {Opcode::JPC, 0, 5},  // line 2: if 0 jump to line 5
        {Opcode::LIT, 0, 99}, // line 3: then branch
        {Opcode::JMP, 0, 6},  // line 4: jump to end
        {Opcode::LIT, 0, 11}, // line 5: else branch (skipped)
        {Opcode::RET, 0, 0},  // line 6: return
    });

    interp2.execute(progTrue);
    assert(!interp2.hasErrors());
    std::cout << "[PASS] test_interpreter_jpc\n";
}

// ============================================
// Test 8: Interpreter CAL + RET
// ============================================
void test_interpreter_cal_ret() {
    StackMachine sm;
    Interpreter interp(sm);

    // Program: main calls proc at line 4, then pushes 7.
    // JMP skips over proc body to main RET.
    auto prog = makeInstructions({
        {Opcode::INT, 0, 3},  // line 0: main frame (SL,DL,RA)
        {Opcode::CAL, 0, 4},  // line 1: call proc at line 4
        {Opcode::LIT, 0, 7},  // line 2: after return, push 7
        {Opcode::JMP, 0, 7},  // line 3: jump to main end (skip proc)
        {Opcode::INT, 0, 3},  // line 4: proc frame
        {Opcode::LIT, 0, 5},  // line 5: proc body push 5
        {Opcode::RET, 0, 0},  // line 6: proc return
        {Opcode::RET, 0, 0},  // line 7: main return
    });

    interp.execute(prog);
    if (interp.hasErrors()) {
        std::cerr << "Error in test_interpreter_cal_ret: " << interp.getErrors()[0].message << "\n";
    }
    assert(!interp.hasErrors());
    std::cout << "[PASS] test_interpreter_cal_ret\n";
}

// ============================================
// Test 9: Invalid jump target
// ============================================
void test_invalid_jump() {
    StackMachine sm;
    Interpreter interp(sm);

    auto prog = makeInstructions({
        {Opcode::JMP, 0, 99}, // invalid target
    });

    interp.execute(prog);
    assert(interp.hasErrors());
    assert(interp.getErrors()[0].type == RuntimeError::INVALID_JUMP);
    std::cout << "[PASS] test_invalid_jump\n";
}

// ============================================
// Test 10: Out-of-bounds access
// ============================================
void test_out_of_bounds() {
    StackMachine sm;
    bool caught = false;
    try {
        sm.store(0, 999, 1); // no frame, invalid address
    } catch (const RuntimeError &e) {
        caught = true;
        assert(e.type == RuntimeError::INDEX_OUT_OF_BOUNDS);
    }
    assert(caught);
    std::cout << "[PASS] test_out_of_bounds\n";
}

// ============================================
// Test 11: Stack overflow (frame count)
// ============================================
void test_stack_overflow() {
    StackMachine sm(2); // max 2 frames
    sm.pushFrame(3, 0, 0);
    sm.pushFrame(3, 0, 0);
    bool caught = false;
    try {
        sm.pushFrame(3, 0, 0); // exceeds maxFrames
    } catch (const RuntimeError &e) {
        caught = true;
        assert(e.type == RuntimeError::STACK_OVERFLOW);
    }
    assert(caught);
    std::cout << "[PASS] test_stack_overflow\n";
}

// ============================================
// Test 12: Display management
// ============================================
void test_display_management() {
    StackMachine sm;
    sm.setDisplay(0, 10);
    sm.setDisplay(1, 20);
    assert(sm.getDisplay(0) == 10);
    assert(sm.getDisplay(1) == 20);
    assert(sm.getDisplay(99) == 0);
    std::cout << "[PASS] test_display_management\n";
}

// ============================================
// Main
// ============================================
int main() {
    std::cout << "=== Role 3 Unit Tests ===\n";
    try {
        test_stack_push_pop();
        test_stack_underflow();
        test_frame_and_memory();
        test_static_link_chain();
        test_interpreter_lit_sto_lod();
        test_interpreter_jmp();
        test_interpreter_jpc();
        test_interpreter_cal_ret();
        test_invalid_jump();
        test_out_of_bounds();
        test_stack_overflow();
        test_display_management();
        std::cout << "\nAll tests passed!\n";
    } catch (const RuntimeError &e) {
        std::cerr << "Uncaught RuntimeError: " << e.message << " (line=" << e.line << ")\n";
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

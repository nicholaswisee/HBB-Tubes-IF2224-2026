#include "lexical/Scanner.hpp"
#include "syntax/Parser.hpp"
#include "semantic/ParseTreeToAST.hpp"
#include "semantic/SymbolTableManager.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "intermediate/CodeGenerator.hpp"
#include "runtime/StackMachine.hpp"
#include "runtime/Interpreter.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./bin/driver_m4 <filename>\n";
        return 1;
    }

    const fs::path inputDir = "test/milestone4/input";
    const fs::path filePath = inputDir / argv[1];

    if (!fs::exists(filePath)) {
        std::cerr << "Input file not found: " << filePath << "\n";
        return 1;
    }

    // Phase 1: Lexical Analysis
    Scanner scanner(filePath.string());
    auto tokens = scanner.scanTokens();

    // Phase 2: Syntax Analysis
    std::shared_ptr<ParseTreeNode> tree;
    try {
        Parser parser(tokens);
        tree = parser.parse();
    } catch (const SyntaxError &e) {
        std::cerr << "Syntax error: " << e.what() << "\n";
        return 1;
    }

    // Phase 3: Semantic Analysis
    ParseTreeToAST converter;
    auto ast = converter.convert(tree);
    if (!ast) {
        std::cerr << "Failed to convert parse tree to AST\n";
        return 1;
    }

    SymbolTableManager symTable;
    SemanticAnalyzer analyzer(symTable);
    analyzer.analyze(ast);

    if (analyzer.hasErrors()) {
        analyzer.printErrors();
        return 1;
    }

    // Phase 4: Intermediate Code Generation
    Intermediate::CodeGenerator codeGen(symTable);
    auto instructions = codeGen.generate(ast);

    std::cout << "=== Intermediate Code ===" << std::endl;
    for (const auto &inst : instructions) {
        std::cout << inst.toString() << "\n";
    }

    // Phase 5: Execution
    std::cout << "\n=== Program Output ===" << std::endl;
    StackMachine stack;
    Interpreter interpreter(stack);

    try {
        interpreter.execute(instructions);
    } catch (const RuntimeError &e) {
        std::cerr << "Runtime error: " << e.message << "\n";
        return 1;
    }

    if (interpreter.hasErrors()) {
        for (const auto &e : interpreter.getErrors()) {
            std::cerr << "Runtime error: " << e.message << "\n";
        }
        return 1;
    }

    std::cout << "\nExecution completed successfully." << std::endl;
    return 0;
}

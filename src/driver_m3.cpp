#include "lexical/Scanner.hpp"
#include "syntax/Parser.hpp"
#include "semantic/ParseTreeToAST.hpp"
#include "semantic/SymbolTableManager.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./bin/driver_m3 <filename>\n";
        return 1;
    }

    const fs::path inputDir = "test/milestone3/input";
    const fs::path fileName = argv[1];
    const fs::path filePath = inputDir / fileName;

    if (!fs::exists(filePath)) {
        std::cerr << "File not found: " << filePath << '\n';
        return 1;
    }

    Scanner scanner(filePath.string());
    std::vector<Token> tokens = scanner.scanTokens();

    std::shared_ptr<ParseTreeNode> tree;
    try {
        Parser parser(tokens);
        tree = parser.parse();
    } catch (const SyntaxError &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    try {
        ParseTreeToAST converter;
        auto ast = converter.convert(tree);

        if (!ast) {
            std::cerr << "Failed to convert parse tree to AST\n";
            return 1;
        }

        std::cout << "=== Abstract Syntax Tree ===\n";
        ast->print(std::cout);
        std::cout << "\n";

        SymbolTableManager symTable;
        SemanticAnalyzer analyzer(symTable);
        analyzer.analyze(ast);

        std::cout << "=== Symbol Tables ===\n";
        symTable.printTab();
        symTable.printBTab();
        if (symTable.atabSize() > 0) {
            symTable.printATab();
        }

        if (analyzer.hasErrors()) {
            analyzer.printErrors();
            return 1;
        }

        std::cout << "\nSemantic analysis completed successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

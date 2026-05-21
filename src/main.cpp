#include "lexical/Scanner.hpp"
#include "syntax/Parser.hpp"
#include "semantic/ParseTreeToAST.hpp"
#include "semantic/SymbolTableManager.hpp"
#include "semantic/SemanticAnalyzer.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    // format: test/milestone[x]/[input or output]/[filename].txt
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: ./bin/compiler <filename-in-test-input> [milestone=3]\n";
        return 1;
    }

    int milestone = 3;
    if (argc == 3) {
        try {
            milestone = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "Invalid milestone number: " << argv[2] << "\n";
            return 1;
        }
    }

    const fs::path inputDir = "test/milestone" + std::to_string(milestone) + "/input";
    const fs::path outputDir = "test/milestone" + std::to_string(milestone) + "/output";

    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        std::cerr << "Input directory not found: " << inputDir << '\n';
        return 1;
    }

    const fs::path fileName = argv[1];
    if (fileName.has_parent_path() || fileName.is_absolute()) {
        std::cerr << "Please pass only a filename, not a path: " << fileName
                  << '\n';
        return 1;
    }

    const fs::path filePath = inputDir / fileName;
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        std::cerr << "Input file not found: " << filePath << '\n';
        return 1;
    }

    std::cout << "=== Lexical Analysis ===\n";
    Scanner scanner(filePath.string());
    std::vector<Token> tokens = scanner.scanTokens();

    for (const auto &tok : tokens) {
        if (tok.type != TokenType::eof_token) {
            std::cout << tok.toString() << "\n";
        }
    }
    std::cout << "\n=== Syntax Analysis (Parse Tree) ===\n";
    std::shared_ptr<ParseTreeNode> tree;
    try {
        Parser parser(tokens);
        tree = parser.parse();
        tree->printToConsole();
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }
        fs::path outFile = outputDir / fileName;
        outFile.replace_extension(".txt");
        tree->saveToFile(outFile.string());
        std::cout << "\nParse tree saved to: " << outFile << "\n";

    } catch (const SyntaxError &e) {
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    }

    std::cout << "\n=== Semantic Analysis ===\n";
    try {
        ParseTreeToAST converter;
        auto ast = converter.convert(tree);

        if (!ast) {
            std::cerr << "Failed to convert parse tree to AST\n";
            return 1;
        }

        std::cout << "\n=== Abstract Syntax Tree (AST) ===\n";
        ast->print(std::cout);
        std::cout << "\n";

        SymbolTableManager symTable;
        SemanticAnalyzer analyzer(symTable);
        analyzer.analyze(ast);

        if (analyzer.hasErrors()) {
            analyzer.printErrors();
        }

        std::cout << "\n=== Symbol Tables ===\n";
        symTable.printTab();
        symTable.printBTab();
        if (symTable.atabSize() > 0) {
            symTable.printATab();
        }

        if (analyzer.hasErrors()) {
            return 1;
        }

        std::cout << "Semantic analysis completed successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Semantic analysis error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

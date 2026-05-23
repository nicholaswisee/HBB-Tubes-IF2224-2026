#include "lexical/Scanner.hpp"
#include "syntax/Parser.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./bin/driver_m2 <filename>\n";
        return 1;
    }

    const fs::path inputDir = "test/milestone2/input";
    const fs::path outputDir = "test/milestone2/output";
    const fs::path fileName = argv[1];
    const fs::path filePath = inputDir / fileName;

    if (!fs::exists(filePath)) {
        std::cerr << "File not found: " << filePath << '\n';
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
    try {
        Parser parser(tokens);
        auto tree = parser.parse();
        tree->printToConsole();

        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }
        fs::path outFile = outputDir / fileName;
        outFile.replace_extension(".txt");
        tree->saveToFile(outFile.string());
        std::cout << "\nParse tree saved to: " << outFile << "\n";
        std::cout << "\nSyntax analysis completed successfully.\n";

    } catch (const SyntaxError &e) {
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}

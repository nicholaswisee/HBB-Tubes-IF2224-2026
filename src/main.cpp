#include "lexical/Scanner.hpp"
#include "syntax/Parser.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    const fs::path inputDir = "test/input";
    const fs::path outputDir = "test/output";

    if (argc != 2) {
        std::cerr << "Usage: ./bin/compiler <filename-in-test-input>\n";
        return 1;
    }

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

    } catch (const SyntaxError &e) {
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}

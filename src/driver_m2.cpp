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

    Scanner scanner(filePath.string());
    std::vector<Token> tokens = scanner.scanTokens();

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

    } catch (const SyntaxError &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}

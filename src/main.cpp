#include "lexical/Scanner.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    const fs::path inputDir = "test/input";

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

    Scanner scanner(filePath.string());
    std::vector<Token> tokens = scanner.scanTokens();

    for (const Token &token : tokens) {
        printToken(token);
    }

    return 0;
}

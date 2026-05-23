#include "lexical/Scanner.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./bin/driver_m1 <filename>\n";
        return 1;
    }

    const fs::path inputDir = "test/milestone1/input";
    const fs::path fileName = argv[1];
    const fs::path filePath = inputDir / fileName;

    if (!fs::exists(filePath)) {
        std::cerr << "File not found: " << filePath << '\n';
        return 1;
    }

    Scanner scanner(filePath.string());
    std::vector<Token> tokens = scanner.scanTokens();

    for (const auto &tok : tokens) {
        if (tok.type != TokenType::eof_token) {
            std::cout << tok.toString() << "\n";
        }
    }
    return 0;
}

#include "lexical/SourceBuffer.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static std::string printableChar(char c) {
    switch (c) {
    case '\n':
        return "\\n";
    case '\t':
        return "\\t";
    case '\r':
        return "\\r";
    case '\0':
        return "\\0";
    default:
        return std::string(1, c);
    }
}

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

    std::cout << "Testing SourceBuffer with " << filePath << "\n\n";

    SourceBuffer source(filePath.string());

    std::cout << filePath.filename().string() << endl;
    std::cout << "length(): " << source.length() << endl;

    for (size_t i = 0; i < source.length(); ++i) {
        std::cout << "at(" << i << "): '" << printableChar(source.at(i))
                  << "'\n";
    }

    std::cout << "at(length()): '" << printableChar(source.at(source.length()))
              << "' (out-of-range sentinel)\n";

    const size_t sampleLen = source.length() < 20 ? source.length() : 20;
    std::cout << "substring(0, " << sampleLen << "): \""
              << source.substring(0, sampleLen) << "\"\n\n";

    return 0;
}

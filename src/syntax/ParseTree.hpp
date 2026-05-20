#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct ParseTreeNode {
    std::string label;
    int line = 0;
    std::vector<std::shared_ptr<ParseTreeNode>> children;

    explicit ParseTreeNode(const std::string &label) : label(label) {}

    void addChild(std::shared_ptr<ParseTreeNode> child) {
        children.push_back(std::move(child));
    }

    // Print buat tree
    void print(std::ostream &os, const std::string &prefix = "",
               bool isLast = true, bool isRoot = true) const {
        if (isRoot) {
            os << label << "\n";
        } else {
            os << prefix;
            os << (isLast ? "+-- " : "|-- ");
            os << label << "\n";
        }

        std::string childPrefix =
            isRoot ? "" : prefix + (isLast ? "    " : "|   ");

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->print(os, childPrefix, i == children.size() - 1,
                               false);
        }
    }

    void printToConsole() const { print(std::cout); }

    void saveToFile(const std::string &path) const {
        std::ofstream file(path);
        if (file.is_open()) {
            print(file);
            file.close();
        } else {
            std::cerr << "Error: Cannot open output file: " << path << "\n";
        }
    }
};
inline std::shared_ptr<ParseTreeNode> makeNode(const std::string &label, int line = 0) {
    auto node = std::make_shared<ParseTreeNode>(label);
    node->line = line;
    return node;
}

#include "SourceBuffer.hpp"
#include <fstream>
#include <sstream>

SourceBuffer::SourceBuffer(const string &path) {
    ifstream file(path);
    stringstream ss;
    ss << file.rdbuf();
    buffer = ss.str();
}

char SourceBuffer::at(size_t pos) const {
    if (pos >= buffer.length())
        return '\0';
    return buffer[pos];
}

string SourceBuffer::substring(size_t start, size_t length) const {
    return buffer.substr(start, length);
}

size_t SourceBuffer::length() const { return buffer.length(); }

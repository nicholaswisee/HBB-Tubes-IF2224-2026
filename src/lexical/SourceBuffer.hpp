#pragma once

#include <cstddef>
#include <string>
using namespace std;

class SourceBuffer {
  public:
    explicit SourceBuffer(const string &path);
    char at(size_t pos) const;
    string substring(size_t start, size_t length) const;
    size_t length() const;

  private:
    string buffer;
};

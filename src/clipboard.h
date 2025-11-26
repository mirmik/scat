#pragma once
#include <string>
#include <sstream>
#include <iostream>

void copy_to_clipboard(const std::string& text);

class CopyGuard {
public:
    explicit CopyGuard(bool enabled);
    ~CopyGuard();
private:
    bool enabled_;
    std::ostringstream buffer_;
    std::streambuf* old_buf_;
};

#pragma once
#include <iostream>
#include <sstream>
#include <string>

void copy_to_clipboard(const std::string &text, bool verbose = false);

class CopyGuard
{
public:
    explicit CopyGuard(bool enabled, bool verbose = false);
    ~CopyGuard();

private:
    bool enabled_;
    bool verbose_;
    std::ostringstream buffer_;
    std::streambuf *old_buf_;
};

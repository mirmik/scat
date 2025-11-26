#pragma once

#include <string>

struct GitInfo {
    std::string commit;
    std::string remote;
    bool has_commit = false;
    bool has_remote = false;
};

// Tries to detect git commit hash and remote origin URL
GitInfo detect_git_info();

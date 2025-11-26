#pragma once

#include <string>

struct GitInfo
{
    std::string commit;
    std::string remote;
    bool has_commit = false;
    bool has_remote = false;
};

struct GitHubInfo
{
    std::string user;
    std::string repo;
    std::string commit;
    bool ok = false;
};

// Tries to detect git commit hash and remote origin URL
GitInfo detect_git_info();
bool parse_github_remote(const std::string &remote,
                         std::string &user,
                         std::string &repo);

GitHubInfo detect_github_info();
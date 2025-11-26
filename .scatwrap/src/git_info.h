<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<pre><code>
#pragma once

#include &lt;string&gt;

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
bool parse_github_remote(const std::string &amp;remote,
                         std::string &amp;user,
                         std::string &amp;repo);

GitHubInfo detect_github_info();
std::string detect_git_dir(); // NEW
</code></pre>
</body>
</html>

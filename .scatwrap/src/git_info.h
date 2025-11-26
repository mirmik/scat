<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
<br>
#include &lt;string&gt;<br>
<br>
struct GitInfo<br>
{<br>
    std::string commit;<br>
    std::string remote;<br>
    bool has_commit = false;<br>
    bool has_remote = false;<br>
};<br>
<br>
struct GitHubInfo<br>
{<br>
    std::string user;<br>
    std::string repo;<br>
    std::string commit;<br>
    bool ok = false;<br>
};<br>
<br>
// Tries to detect git commit hash and remote origin URL<br>
GitInfo detect_git_info();<br>
bool parse_github_remote(const std::string &amp;remote,<br>
                         std::string &amp;user,<br>
                         std::string &amp;repo);<br>
<br>
GitHubInfo detect_github_info();<br>
std::string detect_git_dir(); // NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

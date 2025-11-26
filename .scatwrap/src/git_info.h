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
&#9;std::string commit;<br>
&#9;std::string remote;<br>
&#9;bool has_commit = false;<br>
&#9;bool has_remote = false;<br>
};<br>
<br>
struct GitHubInfo<br>
{<br>
&#9;std::string user;<br>
&#9;std::string repo;<br>
&#9;std::string commit;<br>
&#9;bool ok = false;<br>
};<br>
<br>
// Tries to detect git commit hash and remote origin URL<br>
GitInfo detect_git_info();<br>
bool parse_github_remote(const std::string &amp;remote,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::string &amp;user,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::string &amp;repo);<br>
<br>
GitHubInfo detect_github_info();<br>
std::string detect_git_dir(); // NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

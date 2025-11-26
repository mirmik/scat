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
&emsp;std::string commit;<br>
&emsp;std::string remote;<br>
&emsp;bool has_commit = false;<br>
&emsp;bool has_remote = false;<br>
};<br>
<br>
struct GitHubInfo<br>
{<br>
&emsp;std::string user;<br>
&emsp;std::string repo;<br>
&emsp;std::string commit;<br>
&emsp;bool ok = false;<br>
};<br>
<br>
// Tries to detect git commit hash and remote origin URL<br>
GitInfo detect_git_info();<br>
bool parse_github_remote(const std::string &amp;remote,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::string &amp;user,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::string &amp;repo);<br>
<br>
GitHubInfo detect_github_info();<br>
std::string detect_git_dir(); // NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

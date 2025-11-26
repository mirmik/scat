<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;string&gt;<br>
<br>
struct&nbsp;GitInfo<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;commit;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;remote;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;has_commit&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;has_remote&nbsp;=&nbsp;false;<br>
};<br>
<br>
struct&nbsp;GitHubInfo<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;user;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;repo;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;commit;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;ok&nbsp;=&nbsp;false;<br>
};<br>
<br>
//&nbsp;Tries&nbsp;to&nbsp;detect&nbsp;git&nbsp;commit&nbsp;hash&nbsp;and&nbsp;remote&nbsp;origin&nbsp;URL<br>
GitInfo&nbsp;detect_git_info();<br>
bool&nbsp;parse_github_remote(const&nbsp;std::string&nbsp;&amp;remote,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;&amp;user,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;&amp;repo);<br>
<br>
GitHubInfo&nbsp;detect_github_info();<br>
std::string&nbsp;detect_git_dir();&nbsp;//&nbsp;NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

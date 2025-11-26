<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
<br>
#include &#20;&lt;string&gt;<br>
<br>
struct &#20;GitInfo<br>
{<br>
 &#20; &#20; &#20; &#20;std::string &#20;commit;<br>
 &#20; &#20; &#20; &#20;std::string &#20;remote;<br>
 &#20; &#20; &#20; &#20;bool &#20;has_commit &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;has_remote &#20;= &#20;false;<br>
};<br>
<br>
struct &#20;GitHubInfo<br>
{<br>
 &#20; &#20; &#20; &#20;std::string &#20;user;<br>
 &#20; &#20; &#20; &#20;std::string &#20;repo;<br>
 &#20; &#20; &#20; &#20;std::string &#20;commit;<br>
 &#20; &#20; &#20; &#20;bool &#20;ok &#20;= &#20;false;<br>
};<br>
<br>
// &#20;Tries &#20;to &#20;detect &#20;git &#20;commit &#20;hash &#20;and &#20;remote &#20;origin &#20;URL<br>
GitInfo &#20;detect_git_info();<br>
bool &#20;parse_github_remote(const &#20;std::string &#20;&amp;remote,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;&amp;user,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;&amp;repo);<br>
<br>
GitHubInfo &#20;detect_github_info();<br>
std::string &#20;detect_git_dir(); &#20;// &#20;NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

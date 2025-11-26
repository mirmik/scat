<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
<br>
#include&#32;&lt;string&gt;<br>
<br>
struct&#32;GitInfo<br>
{<br>
&#32;&#32;&#32;&#32;std::string&#32;commit;<br>
&#32;&#32;&#32;&#32;std::string&#32;remote;<br>
&#32;&#32;&#32;&#32;bool&#32;has_commit&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;has_remote&#32;=&#32;false;<br>
};<br>
<br>
struct&#32;GitHubInfo<br>
{<br>
&#32;&#32;&#32;&#32;std::string&#32;user;<br>
&#32;&#32;&#32;&#32;std::string&#32;repo;<br>
&#32;&#32;&#32;&#32;std::string&#32;commit;<br>
&#32;&#32;&#32;&#32;bool&#32;ok&#32;=&#32;false;<br>
};<br>
<br>
//&#32;Tries&#32;to&#32;detect&#32;git&#32;commit&#32;hash&#32;and&#32;remote&#32;origin&#32;URL<br>
GitInfo&#32;detect_git_info();<br>
bool&#32;parse_github_remote(const&#32;std::string&#32;&amp;remote,<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::string&#32;&amp;user,<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::string&#32;&amp;repo);<br>
<br>
GitHubInfo&#32;detect_github_info();<br>
std::string&#32;detect_git_dir();&#32;//&#32;NEW<br>
<!-- END SCAT CODE -->
</body>
</html>

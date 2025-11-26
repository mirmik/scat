<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &#20;&quot;git_info.h&quot;<br>
<br>
#include &#20;&lt;cstdio&gt;<br>
#include &#20;&lt;string&gt;<br>
<br>
// &#20;Runs &#20;a &#20;shell &#20;command &#20;and &#20;captures &#20;its &#20;stdout.<br>
// &#20;Returns &#20;empty &#20;string &#20;on &#20;error &#20;or &#20;if &#20;nothing &#20;was &#20;printed.<br>
static &#20;std::string &#20;run_command_capture(const &#20;char &#20;*cmd)<br>
{<br>
#ifdef &#20;_WIN32<br>
 &#20; &#20; &#20; &#20;FILE &#20;*pipe &#20;= &#20;_popen(cmd, &#20;&quot;r&quot;);<br>
#else<br>
 &#20; &#20; &#20; &#20;FILE &#20;*pipe &#20;= &#20;popen(cmd, &#20;&quot;r&quot;);<br>
#endif<br>
 &#20; &#20; &#20; &#20;if &#20;(!pipe)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;{};<br>
<br>
 &#20; &#20; &#20; &#20;std::string &#20;result;<br>
 &#20; &#20; &#20; &#20;char &#20;buffer[256];<br>
<br>
 &#20; &#20; &#20; &#20;while &#20;(fgets(buffer, &#20;sizeof(buffer), &#20;pipe))<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;result &#20;+= &#20;buffer;<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
#ifdef &#20;_WIN32<br>
 &#20; &#20; &#20; &#20;_pclose(pipe);<br>
#else<br>
 &#20; &#20; &#20; &#20;pclose(pipe);<br>
#endif<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;strip &#20;trailing &#20;newlines<br>
 &#20; &#20; &#20; &#20;while &#20;(!result.empty() &#20;&amp;&amp; &#20;(result.back() &#20;== &#20;'\n' &#20;|| &#20;result.back() &#20;== &#20;'\r'))<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;result.pop_back();<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;return &#20;result;<br>
}<br>
<br>
GitInfo &#20;detect_git_info()<br>
{<br>
 &#20; &#20; &#20; &#20;GitInfo &#20;info;<br>
<br>
#ifdef &#20;_WIN32<br>
 &#20; &#20; &#20; &#20;std::string &#20;commit &#20;= &#20;run_command_capture(&quot;git &#20;rev-parse &#20;HEAD &#20;2&gt;nul&quot;);<br>
 &#20; &#20; &#20; &#20;std::string &#20;remote &#20;=<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;run_command_capture(&quot;git &#20;config &#20;--get &#20;remote.origin.url &#20;2&gt;nul&quot;);<br>
#else<br>
 &#20; &#20; &#20; &#20;std::string &#20;commit &#20;= &#20;run_command_capture(&quot;git &#20;rev-parse &#20;HEAD &#20;2&gt;/dev/null&quot;);<br>
 &#20; &#20; &#20; &#20;std::string &#20;remote &#20;=<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;run_command_capture(&quot;git &#20;config &#20;--get &#20;remote.origin.url &#20;2&gt;/dev/null&quot;);<br>
#endif<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(!commit.empty())<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;info.commit &#20;= &#20;commit;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;info.has_commit &#20;= &#20;true;<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(!remote.empty())<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;info.remote &#20;= &#20;remote;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;info.has_remote &#20;= &#20;true;<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;return &#20;info;<br>
}<br>
<br>
// &#20;Разбор &#20;remote &#20;вроде &#20;git@github.com:user/repo.git &#20;или<br>
// &#20;https://github.com/user/repo(.git)<br>
bool &#20;parse_github_remote(const &#20;std::string &#20;&amp;remote,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;&amp;user,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;&amp;repo)<br>
{<br>
 &#20; &#20; &#20; &#20;const &#20;std::string &#20;host &#20;= &#20;&quot;github.com&quot;;<br>
 &#20; &#20; &#20; &#20;auto &#20;pos &#20;= &#20;remote.find(host);<br>
 &#20; &#20; &#20; &#20;if &#20;(pos &#20;== &#20;std::string::npos)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;pos &#20;+= &#20;host.size();<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;пропускаем &#20;':' &#20;или &#20;'/' &#20;после &#20;github.com<br>
 &#20; &#20; &#20; &#20;while &#20;(pos &#20;&lt; &#20;remote.size() &#20;&amp;&amp; &#20;(remote[pos] &#20;== &#20;':' &#20;|| &#20;remote[pos] &#20;== &#20;'/'))<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;++pos;<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(pos &#20;&gt;= &#20;remote.size())<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;user &#20;/ &#20;repo[.git] &#20;/ &#20;...<br>
 &#20; &#20; &#20; &#20;auto &#20;slash1 &#20;= &#20;remote.find('/', &#20;pos);<br>
 &#20; &#20; &#20; &#20;if &#20;(slash1 &#20;== &#20;std::string::npos)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;user &#20;= &#20;remote.substr(pos, &#20;slash1 &#20;- &#20;pos);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;start_repo &#20;= &#20;slash1 &#20;+ &#20;1;<br>
 &#20; &#20; &#20; &#20;if &#20;(start_repo &#20;&gt;= &#20;remote.size())<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;slash2 &#20;= &#20;remote.find('/', &#20;start_repo);<br>
 &#20; &#20; &#20; &#20;std::string &#20;repo_part &#20;=<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;(slash2 &#20;== &#20;std::string::npos)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;? &#20;remote.substr(start_repo)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;: &#20;remote.substr(start_repo, &#20;slash2 &#20;- &#20;start_repo);<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;обрежем &#20;.git &#20;в &#20;конце, &#20;если &#20;есть<br>
 &#20; &#20; &#20; &#20;const &#20;std::string &#20;dot_git &#20;= &#20;&quot;.git&quot;;<br>
 &#20; &#20; &#20; &#20;if &#20;(repo_part.size() &#20;&gt; &#20;dot_git.size() &#20;&amp;&amp;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;repo_part.compare(<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;repo_part.size() &#20;- &#20;dot_git.size(), &#20;dot_git.size(), &#20;dot_git) &#20;== &#20;0)<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;repo_part.resize(repo_part.size() &#20;- &#20;dot_git.size());<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(user.empty() &#20;|| &#20;repo_part.empty())<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;repo &#20;= &#20;repo_part;<br>
 &#20; &#20; &#20; &#20;return &#20;true;<br>
}<br>
<br>
GitHubInfo &#20;detect_github_info()<br>
{<br>
 &#20; &#20; &#20; &#20;GitInfo &#20;gi &#20;= &#20;detect_git_info();<br>
 &#20; &#20; &#20; &#20;GitHubInfo &#20;out;<br>
 &#20; &#20; &#20; &#20;if &#20;(!gi.has_commit &#20;|| &#20;!gi.has_remote)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;out;<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(!parse_github_remote(gi.remote, &#20;out.user, &#20;out.repo))<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;out;<br>
<br>
 &#20; &#20; &#20; &#20;out.commit &#20;= &#20;gi.commit;<br>
 &#20; &#20; &#20; &#20;out.ok &#20;= &#20;true;<br>
 &#20; &#20; &#20; &#20;return &#20;out;<br>
}<br>
<br>
std::string &#20;detect_git_dir()<br>
{<br>
#ifdef &#20;_WIN32<br>
 &#20; &#20; &#20; &#20;return &#20;run_command_capture(&quot;git &#20;rev-parse &#20;--git-dir &#20;2&gt;nul&quot;);<br>
#else<br>
 &#20; &#20; &#20; &#20;return &#20;run_command_capture(&quot;git &#20;rev-parse &#20;--git-dir &#20;2&gt;/dev/null&quot;);<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

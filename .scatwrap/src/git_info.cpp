<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;git_info.h&quot;<br>
<br>
#include &lt;cstdio&gt;<br>
#include &lt;string&gt;<br>
<br>
// Runs a shell command and captures its stdout.<br>
// Returns empty string on error or if nothing was printed.<br>
static std::string run_command_capture(const char *cmd)<br>
{<br>
#ifdef _WIN32<br>
&emsp;FILE *pipe = _popen(cmd, &quot;r&quot;);<br>
#else<br>
&emsp;FILE *pipe = popen(cmd, &quot;r&quot;);<br>
#endif<br>
&emsp;if (!pipe)<br>
&emsp;&emsp;return {};<br>
<br>
&emsp;std::string result;<br>
&emsp;char buffer[256];<br>
<br>
&emsp;while (fgets(buffer, sizeof(buffer), pipe))<br>
&emsp;{<br>
&emsp;&emsp;result += buffer;<br>
&emsp;}<br>
<br>
#ifdef _WIN32<br>
&emsp;_pclose(pipe);<br>
#else<br>
&emsp;pclose(pipe);<br>
#endif<br>
<br>
&emsp;// strip trailing newlines<br>
&emsp;while (!result.empty() &amp;&amp; (result.back() == '\n' || result.back() == '\r'))<br>
&emsp;{<br>
&emsp;&emsp;result.pop_back();<br>
&emsp;}<br>
<br>
&emsp;return result;<br>
}<br>
<br>
GitInfo detect_git_info()<br>
{<br>
&emsp;GitInfo info;<br>
<br>
#ifdef _WIN32<br>
&emsp;std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;nul&quot;);<br>
&emsp;std::string remote =<br>
&emsp;&emsp;run_command_capture(&quot;git config --get remote.origin.url 2&gt;nul&quot;);<br>
#else<br>
&emsp;std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;/dev/null&quot;);<br>
&emsp;std::string remote =<br>
&emsp;&emsp;run_command_capture(&quot;git config --get remote.origin.url 2&gt;/dev/null&quot;);<br>
#endif<br>
<br>
&emsp;if (!commit.empty())<br>
&emsp;{<br>
&emsp;&emsp;info.commit = commit;<br>
&emsp;&emsp;info.has_commit = true;<br>
&emsp;}<br>
<br>
&emsp;if (!remote.empty())<br>
&emsp;{<br>
&emsp;&emsp;info.remote = remote;<br>
&emsp;&emsp;info.has_remote = true;<br>
&emsp;}<br>
<br>
&emsp;return info;<br>
}<br>
<br>
// Разбор remote вроде git@github.com:user/repo.git или<br>
// https://github.com/user/repo(.git)<br>
bool parse_github_remote(const std::string &amp;remote,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::string &amp;user,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::string &amp;repo)<br>
{<br>
&emsp;const std::string host = &quot;github.com&quot;;<br>
&emsp;auto pos = remote.find(host);<br>
&emsp;if (pos == std::string::npos)<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;pos += host.size();<br>
<br>
&emsp;// пропускаем ':' или '/' после github.com<br>
&emsp;while (pos &lt; remote.size() &amp;&amp; (remote[pos] == ':' || remote[pos] == '/'))<br>
&emsp;&emsp;++pos;<br>
<br>
&emsp;if (pos &gt;= remote.size())<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;// user / repo[.git] / ...<br>
&emsp;auto slash1 = remote.find('/', pos);<br>
&emsp;if (slash1 == std::string::npos)<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;user = remote.substr(pos, slash1 - pos);<br>
<br>
&emsp;auto start_repo = slash1 + 1;<br>
&emsp;if (start_repo &gt;= remote.size())<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;auto slash2 = remote.find('/', start_repo);<br>
&emsp;std::string repo_part =<br>
&emsp;&emsp;(slash2 == std::string::npos)<br>
&emsp;&emsp;&emsp;? remote.substr(start_repo)<br>
&emsp;&emsp;&emsp;: remote.substr(start_repo, slash2 - start_repo);<br>
<br>
&emsp;// обрежем .git в конце, если есть<br>
&emsp;const std::string dot_git = &quot;.git&quot;;<br>
&emsp;if (repo_part.size() &gt; dot_git.size() &amp;&amp;<br>
&emsp;&emsp;repo_part.compare(<br>
&emsp;&emsp;&emsp;repo_part.size() - dot_git.size(), dot_git.size(), dot_git) == 0)<br>
&emsp;{<br>
&emsp;&emsp;repo_part.resize(repo_part.size() - dot_git.size());<br>
&emsp;}<br>
<br>
&emsp;if (user.empty() || repo_part.empty())<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;repo = repo_part;<br>
&emsp;return true;<br>
}<br>
<br>
GitHubInfo detect_github_info()<br>
{<br>
&emsp;GitInfo gi = detect_git_info();<br>
&emsp;GitHubInfo out;<br>
&emsp;if (!gi.has_commit || !gi.has_remote)<br>
&emsp;&emsp;return out;<br>
<br>
&emsp;if (!parse_github_remote(gi.remote, out.user, out.repo))<br>
&emsp;&emsp;return out;<br>
<br>
&emsp;out.commit = gi.commit;<br>
&emsp;out.ok = true;<br>
&emsp;return out;<br>
}<br>
<br>
std::string detect_git_dir()<br>
{<br>
#ifdef _WIN32<br>
&emsp;return run_command_capture(&quot;git rev-parse --git-dir 2&gt;nul&quot;);<br>
#else<br>
&emsp;return run_command_capture(&quot;git rev-parse --git-dir 2&gt;/dev/null&quot;);<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

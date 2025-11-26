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
&#9;FILE *pipe = _popen(cmd, &quot;r&quot;);<br>
#else<br>
&#9;FILE *pipe = popen(cmd, &quot;r&quot;);<br>
#endif<br>
&#9;if (!pipe)<br>
&#9;&#9;return {};<br>
<br>
&#9;std::string result;<br>
&#9;char buffer[256];<br>
<br>
&#9;while (fgets(buffer, sizeof(buffer), pipe))<br>
&#9;{<br>
&#9;&#9;result += buffer;<br>
&#9;}<br>
<br>
#ifdef _WIN32<br>
&#9;_pclose(pipe);<br>
#else<br>
&#9;pclose(pipe);<br>
#endif<br>
<br>
&#9;// strip trailing newlines<br>
&#9;while (!result.empty() &amp;&amp; (result.back() == '\n' || result.back() == '\r'))<br>
&#9;{<br>
&#9;&#9;result.pop_back();<br>
&#9;}<br>
<br>
&#9;return result;<br>
}<br>
<br>
GitInfo detect_git_info()<br>
{<br>
&#9;GitInfo info;<br>
<br>
#ifdef _WIN32<br>
&#9;std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;nul&quot;);<br>
&#9;std::string remote =<br>
&#9;&#9;run_command_capture(&quot;git config --get remote.origin.url 2&gt;nul&quot;);<br>
#else<br>
&#9;std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;/dev/null&quot;);<br>
&#9;std::string remote =<br>
&#9;&#9;run_command_capture(&quot;git config --get remote.origin.url 2&gt;/dev/null&quot;);<br>
#endif<br>
<br>
&#9;if (!commit.empty())<br>
&#9;{<br>
&#9;&#9;info.commit = commit;<br>
&#9;&#9;info.has_commit = true;<br>
&#9;}<br>
<br>
&#9;if (!remote.empty())<br>
&#9;{<br>
&#9;&#9;info.remote = remote;<br>
&#9;&#9;info.has_remote = true;<br>
&#9;}<br>
<br>
&#9;return info;<br>
}<br>
<br>
// Разбор remote вроде git@github.com:user/repo.git или<br>
// https://github.com/user/repo(.git)<br>
bool parse_github_remote(const std::string &amp;remote,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::string &amp;user,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::string &amp;repo)<br>
{<br>
&#9;const std::string host = &quot;github.com&quot;;<br>
&#9;auto pos = remote.find(host);<br>
&#9;if (pos == std::string::npos)<br>
&#9;&#9;return false;<br>
<br>
&#9;pos += host.size();<br>
<br>
&#9;// пропускаем ':' или '/' после github.com<br>
&#9;while (pos &lt; remote.size() &amp;&amp; (remote[pos] == ':' || remote[pos] == '/'))<br>
&#9;&#9;++pos;<br>
<br>
&#9;if (pos &gt;= remote.size())<br>
&#9;&#9;return false;<br>
<br>
&#9;// user / repo[.git] / ...<br>
&#9;auto slash1 = remote.find('/', pos);<br>
&#9;if (slash1 == std::string::npos)<br>
&#9;&#9;return false;<br>
<br>
&#9;user = remote.substr(pos, slash1 - pos);<br>
<br>
&#9;auto start_repo = slash1 + 1;<br>
&#9;if (start_repo &gt;= remote.size())<br>
&#9;&#9;return false;<br>
<br>
&#9;auto slash2 = remote.find('/', start_repo);<br>
&#9;std::string repo_part =<br>
&#9;&#9;(slash2 == std::string::npos)<br>
&#9;&#9;&#9;? remote.substr(start_repo)<br>
&#9;&#9;&#9;: remote.substr(start_repo, slash2 - start_repo);<br>
<br>
&#9;// обрежем .git в конце, если есть<br>
&#9;const std::string dot_git = &quot;.git&quot;;<br>
&#9;if (repo_part.size() &gt; dot_git.size() &amp;&amp;<br>
&#9;&#9;repo_part.compare(<br>
&#9;&#9;&#9;repo_part.size() - dot_git.size(), dot_git.size(), dot_git) == 0)<br>
&#9;{<br>
&#9;&#9;repo_part.resize(repo_part.size() - dot_git.size());<br>
&#9;}<br>
<br>
&#9;if (user.empty() || repo_part.empty())<br>
&#9;&#9;return false;<br>
<br>
&#9;repo = repo_part;<br>
&#9;return true;<br>
}<br>
<br>
GitHubInfo detect_github_info()<br>
{<br>
&#9;GitInfo gi = detect_git_info();<br>
&#9;GitHubInfo out;<br>
&#9;if (!gi.has_commit || !gi.has_remote)<br>
&#9;&#9;return out;<br>
<br>
&#9;if (!parse_github_remote(gi.remote, out.user, out.repo))<br>
&#9;&#9;return out;<br>
<br>
&#9;out.commit = gi.commit;<br>
&#9;out.ok = true;<br>
&#9;return out;<br>
}<br>
<br>
std::string detect_git_dir()<br>
{<br>
#ifdef _WIN32<br>
&#9;return run_command_capture(&quot;git rev-parse --git-dir 2&gt;nul&quot;);<br>
#else<br>
&#9;return run_command_capture(&quot;git rev-parse --git-dir 2&gt;/dev/null&quot;);<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

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
    FILE *pipe = _popen(cmd, &quot;r&quot;);<br>
#else<br>
    FILE *pipe = popen(cmd, &quot;r&quot;);<br>
#endif<br>
    if (!pipe)<br>
        return {};<br>
<br>
    std::string result;<br>
    char buffer[256];<br>
<br>
    while (fgets(buffer, sizeof(buffer), pipe))<br>
    {<br>
        result += buffer;<br>
    }<br>
<br>
#ifdef _WIN32<br>
    _pclose(pipe);<br>
#else<br>
    pclose(pipe);<br>
#endif<br>
<br>
    // strip trailing newlines<br>
    while (!result.empty() &amp;&amp; (result.back() == '\n' || result.back() == '\r'))<br>
    {<br>
        result.pop_back();<br>
    }<br>
<br>
    return result;<br>
}<br>
<br>
GitInfo detect_git_info()<br>
{<br>
    GitInfo info;<br>
<br>
#ifdef _WIN32<br>
    std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;nul&quot;);<br>
    std::string remote =<br>
        run_command_capture(&quot;git config --get remote.origin.url 2&gt;nul&quot;);<br>
#else<br>
    std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;/dev/null&quot;);<br>
    std::string remote =<br>
        run_command_capture(&quot;git config --get remote.origin.url 2&gt;/dev/null&quot;);<br>
#endif<br>
<br>
    if (!commit.empty())<br>
    {<br>
        info.commit = commit;<br>
        info.has_commit = true;<br>
    }<br>
<br>
    if (!remote.empty())<br>
    {<br>
        info.remote = remote;<br>
        info.has_remote = true;<br>
    }<br>
<br>
    return info;<br>
}<br>
<br>
// Разбор remote вроде git@github.com:user/repo.git или<br>
// https://github.com/user/repo(.git)<br>
bool parse_github_remote(const std::string &amp;remote,<br>
                         std::string &amp;user,<br>
                         std::string &amp;repo)<br>
{<br>
    const std::string host = &quot;github.com&quot;;<br>
    auto pos = remote.find(host);<br>
    if (pos == std::string::npos)<br>
        return false;<br>
<br>
    pos += host.size();<br>
<br>
    // пропускаем ':' или '/' после github.com<br>
    while (pos &lt; remote.size() &amp;&amp; (remote[pos] == ':' || remote[pos] == '/'))<br>
        ++pos;<br>
<br>
    if (pos &gt;= remote.size())<br>
        return false;<br>
<br>
    // user / repo[.git] / ...<br>
    auto slash1 = remote.find('/', pos);<br>
    if (slash1 == std::string::npos)<br>
        return false;<br>
<br>
    user = remote.substr(pos, slash1 - pos);<br>
<br>
    auto start_repo = slash1 + 1;<br>
    if (start_repo &gt;= remote.size())<br>
        return false;<br>
<br>
    auto slash2 = remote.find('/', start_repo);<br>
    std::string repo_part =<br>
        (slash2 == std::string::npos)<br>
            ? remote.substr(start_repo)<br>
            : remote.substr(start_repo, slash2 - start_repo);<br>
<br>
    // обрежем .git в конце, если есть<br>
    const std::string dot_git = &quot;.git&quot;;<br>
    if (repo_part.size() &gt; dot_git.size() &amp;&amp;<br>
        repo_part.compare(<br>
            repo_part.size() - dot_git.size(), dot_git.size(), dot_git) == 0)<br>
    {<br>
        repo_part.resize(repo_part.size() - dot_git.size());<br>
    }<br>
<br>
    if (user.empty() || repo_part.empty())<br>
        return false;<br>
<br>
    repo = repo_part;<br>
    return true;<br>
}<br>
<br>
GitHubInfo detect_github_info()<br>
{<br>
    GitInfo gi = detect_git_info();<br>
    GitHubInfo out;<br>
    if (!gi.has_commit || !gi.has_remote)<br>
        return out;<br>
<br>
    if (!parse_github_remote(gi.remote, out.user, out.repo))<br>
        return out;<br>
<br>
    out.commit = gi.commit;<br>
    out.ok = true;<br>
    return out;<br>
}<br>
<br>
std::string detect_git_dir()<br>
{<br>
#ifdef _WIN32<br>
    return run_command_capture(&quot;git rev-parse --git-dir 2&gt;nul&quot;);<br>
#else<br>
    return run_command_capture(&quot;git rev-parse --git-dir 2&gt;/dev/null&quot;);<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

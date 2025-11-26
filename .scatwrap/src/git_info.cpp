<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.cpp</title>
</head>
<body>
<pre><code>
#include &quot;git_info.h&quot;

#include &lt;cstdio&gt;
#include &lt;string&gt;

// Runs a shell command and captures its stdout.
// Returns empty string on error or if nothing was printed.
static std::string run_command_capture(const char* cmd)
{
#ifdef _WIN32
    FILE* pipe = _popen(cmd, &quot;r&quot;);
#else
    FILE* pipe = popen(cmd, &quot;r&quot;);
#endif
    if (!pipe)
        return {};

    std::string result;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    // strip trailing newlines
    while (!result.empty() &amp;&amp;
           (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}

GitInfo detect_git_info()
{
    GitInfo info;

#ifdef _WIN32
    std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;nul&quot;);
    std::string remote = run_command_capture(&quot;git config --get remote.origin.url 2&gt;nul&quot;);
#else
    std::string commit = run_command_capture(&quot;git rev-parse HEAD 2&gt;/dev/null&quot;);
    std::string remote = run_command_capture(&quot;git config --get remote.origin.url 2&gt;/dev/null&quot;);
#endif

    if (!commit.empty()) {
        info.commit = commit;
        info.has_commit = true;
    }

    if (!remote.empty()) {
        info.remote = remote;
        info.has_remote = true;
    }

    return info;
}

</code></pre>
</body>
</html>

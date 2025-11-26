<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#define DOCTEST_CONFIG_IMPLEMENT<br>
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
#if defined(__WIN32__) || defined(_MSC_VER)<br>
#include &lt;winsock2.h&gt;<br>
WSADATA wsaData;<br>
#endif<br>
<br>
int main(int argc, char **argv)<br>
{<br>
#if defined(__WIN32__) || defined(_MSC_VER)<br>
    int iResult;<br>
<br>
    // Initialize Winsock<br>
    iResult = WSAStartup(MAKEWORD(2, 2), &amp;wsaData);<br>
    if (iResult != 0)<br>
    {<br>
        printf(&quot;WSAStartup failed: %d\n&quot;, iResult);<br>
        return 1;<br>
    }<br>
#endif<br>
<br>
    std::vector&lt;std::string&gt; adjusted_args;<br>
    adjusted_args.reserve(argc);<br>
    for (int i = 0; i &lt; argc; ++i)<br>
    {<br>
        std::string current(argv[i]);<br>
        if ((current == &quot;--test-case&quot; || current == &quot;-tc&quot;) &amp;&amp; i + 1 &lt; argc)<br>
        {<br>
            std::string pattern(argv[i + 1]);<br>
            if (pattern.find('*') == std::string::npos)<br>
                pattern = &quot;*&quot; + pattern + &quot;*&quot;;<br>
            adjusted_args.push_back(current + &quot;=&quot; + pattern);<br>
            ++i;<br>
        }<br>
        else<br>
        {<br>
            adjusted_args.push_back(std::move(current));<br>
        }<br>
    }<br>
<br>
    std::vector&lt;char *&gt; argv_adjusted;<br>
    argv_adjusted.reserve(adjusted_args.size());<br>
    for (auto &amp;arg : adjusted_args)<br>
    {<br>
        argv_adjusted.push_back(arg.data());<br>
    }<br>
<br>
    doctest::Context context;<br>
    context.applyCommandLine(static_cast&lt;int&gt;(argv_adjusted.size()),<br>
                             argv_adjusted.data());<br>
    int res = context.run();  // run<br>
    if (context.shouldExit()) // important - query flags (and --exit) rely on<br>
                              // the user doing this<br>
        return res;           // propagate the result of the tests<br>
    int client_stuff_return_code = 0;<br>
    return res + client_stuff_return_code; // the result from doctest is<br>
                                           // propagated here as well<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

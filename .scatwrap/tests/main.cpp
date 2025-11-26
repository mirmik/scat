<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/main.cpp</title>
</head>
<body>
<pre><code>
#define DOCTEST_CONFIG_IMPLEMENT
#include &quot;doctest/doctest.h&quot;
#include &lt;string&gt;
#include &lt;vector&gt;
#if defined(__WIN32__) || defined(_MSC_VER)
#include &lt;winsock2.h&gt;
WSADATA wsaData;
#endif

int main(int argc, char **argv)
{
#if defined(__WIN32__) || defined(_MSC_VER)
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &amp;wsaData);
    if (iResult != 0)
    {
        printf(&quot;WSAStartup failed: %d\n&quot;, iResult);
        return 1;
    }
#endif

    std::vector&lt;std::string&gt; adjusted_args;
    adjusted_args.reserve(argc);
    for (int i = 0; i &lt; argc; ++i)
    {
        std::string current(argv[i]);
        if ((current == &quot;--test-case&quot; || current == &quot;-tc&quot;) &amp;&amp; i + 1 &lt; argc)
        {
            std::string pattern(argv[i + 1]);
            if (pattern.find('*') == std::string::npos)
                pattern = &quot;*&quot; + pattern + &quot;*&quot;;
            adjusted_args.push_back(current + &quot;=&quot; + pattern);
            ++i;
        }
        else
        {
            adjusted_args.push_back(std::move(current));
        }
    }

    std::vector&lt;char *&gt; argv_adjusted;
    argv_adjusted.reserve(adjusted_args.size());
    for (auto &amp;arg : adjusted_args)
    {
        argv_adjusted.push_back(arg.data());
    }

    doctest::Context context;
    context.applyCommandLine(static_cast&lt;int&gt;(argv_adjusted.size()),
                             argv_adjusted.data());
    int res = context.run();  // run
    if (context.shouldExit()) // important - query flags (and --exit) rely on
                              // the user doing this
        return res;           // propagate the result of the tests
    int client_stuff_return_code = 0;
    return res + client_stuff_return_code; // the result from doctest is
                                           // propagated here as well
}

</code></pre>
</body>
</html>

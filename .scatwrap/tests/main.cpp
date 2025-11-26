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
&#9;int iResult;<br>
<br>
&#9;// Initialize Winsock<br>
&#9;iResult = WSAStartup(MAKEWORD(2, 2), &amp;wsaData);<br>
&#9;if (iResult != 0)<br>
&#9;{<br>
&#9;&#9;printf(&quot;WSAStartup failed: %d\n&quot;, iResult);<br>
&#9;&#9;return 1;<br>
&#9;}<br>
#endif<br>
<br>
&#9;std::vector&lt;std::string&gt; adjusted_args;<br>
&#9;adjusted_args.reserve(argc);<br>
&#9;for (int i = 0; i &lt; argc; ++i)<br>
&#9;{<br>
&#9;&#9;std::string current(argv[i]);<br>
&#9;&#9;if ((current == &quot;--test-case&quot; || current == &quot;-tc&quot;) &amp;&amp; i + 1 &lt; argc)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::string pattern(argv[i + 1]);<br>
&#9;&#9;&#9;if (pattern.find('*') == std::string::npos)<br>
&#9;&#9;&#9;&#9;pattern = &quot;*&quot; + pattern + &quot;*&quot;;<br>
&#9;&#9;&#9;adjusted_args.push_back(current + &quot;=&quot; + pattern);<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;adjusted_args.push_back(std::move(current));<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;std::vector&lt;char *&gt; argv_adjusted;<br>
&#9;argv_adjusted.reserve(adjusted_args.size());<br>
&#9;for (auto &amp;arg : adjusted_args)<br>
&#9;{<br>
&#9;&#9;argv_adjusted.push_back(arg.data());<br>
&#9;}<br>
<br>
&#9;doctest::Context context;<br>
&#9;context.applyCommandLine(static_cast&lt;int&gt;(argv_adjusted.size()),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;argv_adjusted.data());<br>
&#9;int res = context.run();  // run<br>
&#9;if (context.shouldExit()) // important - query flags (and --exit) rely on<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;// the user doing this<br>
&#9;&#9;return res;           // propagate the result of the tests<br>
&#9;int client_stuff_return_code = 0;<br>
&#9;return res + client_stuff_return_code; // the result from doctest is<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;// propagated here as well<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

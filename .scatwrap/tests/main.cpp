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
&emsp;int iResult;<br>
<br>
&emsp;// Initialize Winsock<br>
&emsp;iResult = WSAStartup(MAKEWORD(2, 2), &amp;wsaData);<br>
&emsp;if (iResult != 0)<br>
&emsp;{<br>
&emsp;&emsp;printf(&quot;WSAStartup failed: %d\n&quot;, iResult);<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
#endif<br>
<br>
&emsp;std::vector&lt;std::string&gt; adjusted_args;<br>
&emsp;adjusted_args.reserve(argc);<br>
&emsp;for (int i = 0; i &lt; argc; ++i)<br>
&emsp;{<br>
&emsp;&emsp;std::string current(argv[i]);<br>
&emsp;&emsp;if ((current == &quot;--test-case&quot; || current == &quot;-tc&quot;) &amp;&amp; i + 1 &lt; argc)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::string pattern(argv[i + 1]);<br>
&emsp;&emsp;&emsp;if (pattern.find('*') == std::string::npos)<br>
&emsp;&emsp;&emsp;&emsp;pattern = &quot;*&quot; + pattern + &quot;*&quot;;<br>
&emsp;&emsp;&emsp;adjusted_args.push_back(current + &quot;=&quot; + pattern);<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;adjusted_args.push_back(std::move(current));<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;std::vector&lt;char *&gt; argv_adjusted;<br>
&emsp;argv_adjusted.reserve(adjusted_args.size());<br>
&emsp;for (auto &amp;arg : adjusted_args)<br>
&emsp;{<br>
&emsp;&emsp;argv_adjusted.push_back(arg.data());<br>
&emsp;}<br>
<br>
&emsp;doctest::Context context;<br>
&emsp;context.applyCommandLine(static_cast&lt;int&gt;(argv_adjusted.size()),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;argv_adjusted.data());<br>
&emsp;int res = context.run();  // run<br>
&emsp;if (context.shouldExit()) // important - query flags (and --exit) rely on<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;// the user doing this<br>
&emsp;&emsp;return res;           // propagate the result of the tests<br>
&emsp;int client_stuff_return_code = 0;<br>
&emsp;return res + client_stuff_return_code; // the result from doctest is<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;// propagated here as well<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#define &#20;DOCTEST_CONFIG_IMPLEMENT<br>
#include &#20;&quot;doctest/doctest.h&quot;<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;vector&gt;<br>
#if &#20;defined(__WIN32__) &#20;|| &#20;defined(_MSC_VER)<br>
#include &#20;&lt;winsock2.h&gt;<br>
WSADATA &#20;wsaData;<br>
#endif<br>
<br>
int &#20;main(int &#20;argc, &#20;char &#20;**argv)<br>
{<br>
#if &#20;defined(__WIN32__) &#20;|| &#20;defined(_MSC_VER)<br>
 &#20; &#20; &#20; &#20;int &#20;iResult;<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;Initialize &#20;Winsock<br>
 &#20; &#20; &#20; &#20;iResult &#20;= &#20;WSAStartup(MAKEWORD(2, &#20;2), &#20;&amp;wsaData);<br>
 &#20; &#20; &#20; &#20;if &#20;(iResult &#20;!= &#20;0)<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;printf(&quot;WSAStartup &#20;failed: &#20;%d\n&quot;, &#20;iResult);<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;1;<br>
 &#20; &#20; &#20; &#20;}<br>
#endif<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;std::string&gt; &#20;adjusted_args;<br>
 &#20; &#20; &#20; &#20;adjusted_args.reserve(argc);<br>
 &#20; &#20; &#20; &#20;for &#20;(int &#20;i &#20;= &#20;0; &#20;i &#20;&lt; &#20;argc; &#20;++i)<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;current(argv[i]);<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;if &#20;((current &#20;== &#20;&quot;--test-case&quot; &#20;|| &#20;current &#20;== &#20;&quot;-tc&quot;) &#20;&amp;&amp; &#20;i &#20;+ &#20;1 &#20;&lt; &#20;argc)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string &#20;pattern(argv[i &#20;+ &#20;1]);<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;if &#20;(pattern.find('*') &#20;== &#20;std::string::npos)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;pattern &#20;= &#20;&quot;*&quot; &#20;+ &#20;pattern &#20;+ &#20;&quot;*&quot;;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;adjusted_args.push_back(current &#20;+ &#20;&quot;=&quot; &#20;+ &#20;pattern);<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;++i;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;}<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;else<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;adjusted_args.push_back(std::move(current));<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;}<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;char &#20;*&gt; &#20;argv_adjusted;<br>
 &#20; &#20; &#20; &#20;argv_adjusted.reserve(adjusted_args.size());<br>
 &#20; &#20; &#20; &#20;for &#20;(auto &#20;&amp;arg &#20;: &#20;adjusted_args)<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;argv_adjusted.push_back(arg.data());<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;doctest::Context &#20;context;<br>
 &#20; &#20; &#20; &#20;context.applyCommandLine(static_cast&lt;int&gt;(argv_adjusted.size()),<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;argv_adjusted.data());<br>
 &#20; &#20; &#20; &#20;int &#20;res &#20;= &#20;context.run(); &#20; &#20;// &#20;run<br>
 &#20; &#20; &#20; &#20;if &#20;(context.shouldExit()) &#20;// &#20;important &#20;- &#20;query &#20;flags &#20;(and &#20;--exit) &#20;rely &#20;on<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;the &#20;user &#20;doing &#20;this<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;res; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;propagate &#20;the &#20;result &#20;of &#20;the &#20;tests<br>
 &#20; &#20; &#20; &#20;int &#20;client_stuff_return_code &#20;= &#20;0;<br>
 &#20; &#20; &#20; &#20;return &#20;res &#20;+ &#20;client_stuff_return_code; &#20;// &#20;the &#20;result &#20;from &#20;doctest &#20;is<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;propagated &#20;here &#20;as &#20;well<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

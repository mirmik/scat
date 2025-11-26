<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/scat.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &#20;&quot;scat.h&quot;<br>
#include &#20;&quot;doctest/doctest.h&quot;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;fstream&gt;<br>
#include &#20;&lt;iostream&gt;<br>
#include &#20;&lt;sstream&gt;<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
namespace &#20;fs &#20;= &#20;std::filesystem;<br>
<br>
TEST_CASE(&quot;scat &#20;walks &#20;example/ &#20;correctly&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;scat_test&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
 &#20; &#20; &#20; &#20;fs::create_directories(tmp &#20;/ &#20;&quot;a/b&quot;);<br>
 &#20; &#20; &#20; &#20;fs::create_directories(tmp &#20;/ &#20;&quot;c&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::ofstream(tmp &#20;/ &#20;&quot;1.txt&quot;) &#20;&lt;&lt; &#20;&quot;Hi&quot;;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::ofstream(tmp &#20;/ &#20;&quot;a/b/2.txt&quot;) &#20;&lt;&lt; &#20;&quot;Hello &#20;World!&quot;;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::ofstream(tmp &#20;/ &#20;&quot;c/3.txt&quot;) &#20;&lt;&lt; &#20;&quot;You &#20;find &#20;me!&quot;;<br>
 &#20; &#20; &#20; &#20;}<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;перенаправляем &#20;stdout<br>
 &#20; &#20; &#20; &#20;std::stringstream &#20;buffer;<br>
 &#20; &#20; &#20; &#20;auto &#20;old &#20;= &#20;std::cout.rdbuf(buffer.rdbuf());<br>
<br>
 &#20; &#20; &#20; &#20;const &#20;char &#20;*argv[] &#20;= &#20;{&quot;scat&quot;, &#20;tmp.string().c_str(), &#20;&quot;-r&quot;};<br>
 &#20; &#20; &#20; &#20;scat_main(3, &#20;(char &#20;**)argv);<br>
<br>
 &#20; &#20; &#20; &#20;std::cout.rdbuf(old);<br>
<br>
 &#20; &#20; &#20; &#20;std::string &#20;out &#20;= &#20;buffer.str();<br>
 &#20; &#20; &#20; &#20;CHECK(out.find(&quot;=====&quot;) &#20;!= &#20;std::string::npos);<br>
 &#20; &#20; &#20; &#20;CHECK(out.find(&quot;Hello &#20;World!&quot;) &#20;!= &#20;std::string::npos);<br>
 &#20; &#20; &#20; &#20;CHECK(out.find(&quot;Hi&quot;) &#20;!= &#20;std::string::npos);<br>
 &#20; &#20; &#20; &#20;CHECK(out.find(&quot;You &#20;find &#20;me!&quot;) &#20;!= &#20;std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

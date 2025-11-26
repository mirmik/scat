<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/scat.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;scat.h&quot;<br>
#include&nbsp;&quot;doctest/doctest.h&quot;<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;fstream&gt;<br>
#include&nbsp;&lt;iostream&gt;<br>
#include&nbsp;&lt;sstream&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
namespace&nbsp;fs&nbsp;=&nbsp;std::filesystem;<br>
<br>
TEST_CASE(&quot;scat&nbsp;walks&nbsp;example/&nbsp;correctly&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;fs::path&nbsp;tmp&nbsp;=&nbsp;fs::temp_directory_path()&nbsp;/&nbsp;&quot;scat_test&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;fs::remove_all(tmp);<br>
&nbsp;&nbsp;&nbsp;&nbsp;fs::create_directories(tmp&nbsp;/&nbsp;&quot;a/b&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;fs::create_directories(tmp&nbsp;/&nbsp;&quot;c&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::ofstream(tmp&nbsp;/&nbsp;&quot;1.txt&quot;)&nbsp;&lt;&lt;&nbsp;&quot;Hi&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::ofstream(tmp&nbsp;/&nbsp;&quot;a/b/2.txt&quot;)&nbsp;&lt;&lt;&nbsp;&quot;Hello&nbsp;World!&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::ofstream(tmp&nbsp;/&nbsp;&quot;c/3.txt&quot;)&nbsp;&lt;&lt;&nbsp;&quot;You&nbsp;find&nbsp;me!&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;перенаправляем&nbsp;stdout<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::stringstream&nbsp;buffer;<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;old&nbsp;=&nbsp;std::cout.rdbuf(buffer.rdbuf());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;char&nbsp;*argv[]&nbsp;=&nbsp;{&quot;scat&quot;,&nbsp;tmp.string().c_str(),&nbsp;&quot;-r&quot;};<br>
&nbsp;&nbsp;&nbsp;&nbsp;scat_main(3,&nbsp;(char&nbsp;**)argv);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::cout.rdbuf(old);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;out&nbsp;=&nbsp;buffer.str();<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK(out.find(&quot;=====&quot;)&nbsp;!=&nbsp;std::string::npos);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK(out.find(&quot;Hello&nbsp;World!&quot;)&nbsp;!=&nbsp;std::string::npos);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK(out.find(&quot;Hi&quot;)&nbsp;!=&nbsp;std::string::npos);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK(out.find(&quot;You&nbsp;find&nbsp;me!&quot;)&nbsp;!=&nbsp;std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

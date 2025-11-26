<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/scat.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&#32;&quot;scat.h&quot;<br>
#include&#32;&quot;doctest/doctest.h&quot;<br>
#include&#32;&lt;filesystem&gt;<br>
#include&#32;&lt;fstream&gt;<br>
#include&#32;&lt;iostream&gt;<br>
#include&#32;&lt;sstream&gt;<br>
#include&#32;&lt;string&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
namespace&#32;fs&#32;=&#32;std::filesystem;<br>
<br>
TEST_CASE(&quot;scat&#32;walks&#32;example/&#32;correctly&quot;)<br>
{<br>
&#32;&#32;&#32;&#32;fs::path&#32;tmp&#32;=&#32;fs::temp_directory_path()&#32;/&#32;&quot;scat_test&quot;;<br>
&#32;&#32;&#32;&#32;fs::remove_all(tmp);<br>
&#32;&#32;&#32;&#32;fs::create_directories(tmp&#32;/&#32;&quot;a/b&quot;);<br>
&#32;&#32;&#32;&#32;fs::create_directories(tmp&#32;/&#32;&quot;c&quot;);<br>
<br>
&#32;&#32;&#32;&#32;{<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::ofstream(tmp&#32;/&#32;&quot;1.txt&quot;)&#32;&lt;&lt;&#32;&quot;Hi&quot;;<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::ofstream(tmp&#32;/&#32;&quot;a/b/2.txt&quot;)&#32;&lt;&lt;&#32;&quot;Hello&#32;World!&quot;;<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::ofstream(tmp&#32;/&#32;&quot;c/3.txt&quot;)&#32;&lt;&lt;&#32;&quot;You&#32;find&#32;me!&quot;;<br>
&#32;&#32;&#32;&#32;}<br>
<br>
&#32;&#32;&#32;&#32;//&#32;перенаправляем&#32;stdout<br>
&#32;&#32;&#32;&#32;std::stringstream&#32;buffer;<br>
&#32;&#32;&#32;&#32;auto&#32;old&#32;=&#32;std::cout.rdbuf(buffer.rdbuf());<br>
<br>
&#32;&#32;&#32;&#32;const&#32;char&#32;*argv[]&#32;=&#32;{&quot;scat&quot;,&#32;tmp.string().c_str(),&#32;&quot;-r&quot;};<br>
&#32;&#32;&#32;&#32;scat_main(3,&#32;(char&#32;**)argv);<br>
<br>
&#32;&#32;&#32;&#32;std::cout.rdbuf(old);<br>
<br>
&#32;&#32;&#32;&#32;std::string&#32;out&#32;=&#32;buffer.str();<br>
&#32;&#32;&#32;&#32;CHECK(out.find(&quot;=====&quot;)&#32;!=&#32;std::string::npos);<br>
&#32;&#32;&#32;&#32;CHECK(out.find(&quot;Hello&#32;World!&quot;)&#32;!=&#32;std::string::npos);<br>
&#32;&#32;&#32;&#32;CHECK(out.find(&quot;Hi&quot;)&#32;!=&#32;std::string::npos);<br>
&#32;&#32;&#32;&#32;CHECK(out.find(&quot;You&#32;find&#32;me!&quot;)&#32;!=&#32;std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

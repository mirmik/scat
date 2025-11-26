<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/scat.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;scat.h&quot;<br>
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
TEST_CASE(&quot;scat walks example/ correctly&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;scat_test&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp / &quot;a/b&quot;);<br>
&emsp;fs::create_directories(tmp / &quot;c&quot;);<br>
<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream(tmp / &quot;1.txt&quot;) &lt;&lt; &quot;Hi&quot;;<br>
&emsp;&emsp;std::ofstream(tmp / &quot;a/b/2.txt&quot;) &lt;&lt; &quot;Hello World!&quot;;<br>
&emsp;&emsp;std::ofstream(tmp / &quot;c/3.txt&quot;) &lt;&lt; &quot;You find me!&quot;;<br>
&emsp;}<br>
<br>
&emsp;// перенаправляем stdout<br>
&emsp;std::stringstream buffer;<br>
&emsp;auto old = std::cout.rdbuf(buffer.rdbuf());<br>
<br>
&emsp;const char *argv[] = {&quot;scat&quot;, tmp.string().c_str(), &quot;-r&quot;};<br>
&emsp;scat_main(3, (char **)argv);<br>
<br>
&emsp;std::cout.rdbuf(old);<br>
<br>
&emsp;std::string out = buffer.str();<br>
&emsp;CHECK(out.find(&quot;=====&quot;) != std::string::npos);<br>
&emsp;CHECK(out.find(&quot;Hello World!&quot;) != std::string::npos);<br>
&emsp;CHECK(out.find(&quot;Hi&quot;) != std::string::npos);<br>
&emsp;CHECK(out.find(&quot;You find me!&quot;) != std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

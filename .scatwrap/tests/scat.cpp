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
&#9;fs::path tmp = fs::temp_directory_path() / &quot;scat_test&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp / &quot;a/b&quot;);<br>
&#9;fs::create_directories(tmp / &quot;c&quot;);<br>
<br>
&#9;{<br>
&#9;&#9;std::ofstream(tmp / &quot;1.txt&quot;) &lt;&lt; &quot;Hi&quot;;<br>
&#9;&#9;std::ofstream(tmp / &quot;a/b/2.txt&quot;) &lt;&lt; &quot;Hello World!&quot;;<br>
&#9;&#9;std::ofstream(tmp / &quot;c/3.txt&quot;) &lt;&lt; &quot;You find me!&quot;;<br>
&#9;}<br>
<br>
&#9;// перенаправляем stdout<br>
&#9;std::stringstream buffer;<br>
&#9;auto old = std::cout.rdbuf(buffer.rdbuf());<br>
<br>
&#9;const char *argv[] = {&quot;scat&quot;, tmp.string().c_str(), &quot;-r&quot;};<br>
&#9;scat_main(3, (char **)argv);<br>
<br>
&#9;std::cout.rdbuf(old);<br>
<br>
&#9;std::string out = buffer.str();<br>
&#9;CHECK(out.find(&quot;=====&quot;) != std::string::npos);<br>
&#9;CHECK(out.find(&quot;Hello World!&quot;) != std::string::npos);<br>
&#9;CHECK(out.find(&quot;Hi&quot;) != std::string::npos);<br>
&#9;CHECK(out.find(&quot;You find me!&quot;) != std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

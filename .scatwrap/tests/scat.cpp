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
    fs::path tmp = fs::temp_directory_path() / &quot;scat_test&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp / &quot;a/b&quot;);<br>
    fs::create_directories(tmp / &quot;c&quot;);<br>
<br>
    {<br>
        std::ofstream(tmp / &quot;1.txt&quot;) &lt;&lt; &quot;Hi&quot;;<br>
        std::ofstream(tmp / &quot;a/b/2.txt&quot;) &lt;&lt; &quot;Hello World!&quot;;<br>
        std::ofstream(tmp / &quot;c/3.txt&quot;) &lt;&lt; &quot;You find me!&quot;;<br>
    }<br>
<br>
    // перенаправляем stdout<br>
    std::stringstream buffer;<br>
    auto old = std::cout.rdbuf(buffer.rdbuf());<br>
<br>
    const char *argv[] = {&quot;scat&quot;, tmp.string().c_str(), &quot;-r&quot;};<br>
    scat_main(3, (char **)argv);<br>
<br>
    std::cout.rdbuf(old);<br>
<br>
    std::string out = buffer.str();<br>
    CHECK(out.find(&quot;=====&quot;) != std::string::npos);<br>
    CHECK(out.find(&quot;Hello World!&quot;) != std::string::npos);<br>
    CHECK(out.find(&quot;Hi&quot;) != std::string::npos);<br>
    CHECK(out.find(&quot;You find me!&quot;) != std::string::npos);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

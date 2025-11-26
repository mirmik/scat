<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/scat.cpp</title>
</head>
<body>
<pre><code>
#include &quot;scat.h&quot;
#include &quot;doctest/doctest.h&quot;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;iostream&gt;
#include &lt;sstream&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

namespace fs = std::filesystem;

TEST_CASE(&quot;scat walks example/ correctly&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;scat_test&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp / &quot;a/b&quot;);
    fs::create_directories(tmp / &quot;c&quot;);

    {
        std::ofstream(tmp / &quot;1.txt&quot;) &lt;&lt; &quot;Hi&quot;;
        std::ofstream(tmp / &quot;a/b/2.txt&quot;) &lt;&lt; &quot;Hello World!&quot;;
        std::ofstream(tmp / &quot;c/3.txt&quot;) &lt;&lt; &quot;You find me!&quot;;
    }

    // перенаправляем stdout
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());

    const char *argv[] = {&quot;scat&quot;, tmp.string().c_str(), &quot;-r&quot;};
    scat_main(3, (char **)argv);

    std::cout.rdbuf(old);

    std::string out = buffer.str();
    CHECK(out.find(&quot;=====&quot;) != std::string::npos);
    CHECK(out.find(&quot;Hello World!&quot;) != std::string::npos);
    CHECK(out.find(&quot;Hi&quot;) != std::string::npos);
    CHECK(out.find(&quot;You find me!&quot;) != std::string::npos);
}

</code></pre>
</body>
</html>

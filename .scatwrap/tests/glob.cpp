<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/glob.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;glob.h&quot;<br>
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
static void write_file(const fs::path &amp;p, const std::string &amp;text)<br>
{<br>
&emsp;fs::create_directories(p.parent_path());<br>
&emsp;std::ofstream out(p);<br>
&emsp;out &lt;&lt; text;<br>
}<br>
<br>
static std::vector&lt;std::string&gt; to_rel(const std::vector&lt;fs::path&gt; &amp;v,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const fs::path &amp;root)<br>
{<br>
&emsp;std::vector&lt;std::string&gt; out;<br>
&emsp;for (auto &amp;p : v)<br>
&emsp;&emsp;out.push_back(fs::relative(p, root).generic_string());<br>
&emsp;std::sort(out.begin(), out.end());<br>
&emsp;return out;<br>
}<br>
<br>
static void check_paths(const std::vector&lt;std::string&gt; &amp;actual,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::initializer_list&lt;const char *&gt; expected)<br>
{<br>
&emsp;CHECK(actual.size() == expected.size());<br>
<br>
&emsp;size_t i = 0;<br>
&emsp;for (auto *e : expected)<br>
&emsp;{<br>
&emsp;&emsp;CHECK(actual[i] == e);<br>
&emsp;&emsp;++i;<br>
&emsp;}<br>
}<br>
<br>
TEST_CASE(&quot;glob: basic *&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
&emsp;write_file(tmp / &quot;b.cpp&quot;, &quot;B&quot;);<br>
&emsp;write_file(tmp / &quot;c.txt&quot;, &quot;C&quot;);<br>
<br>
&emsp;auto out = expand_glob((tmp / &quot;*.txt&quot;).generic_string());<br>
&emsp;auto rel = to_rel(out, tmp);<br>
<br>
&emsp;check_paths(rel, {&quot;a.txt&quot;, &quot;c.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: recursive **&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;1.txt&quot;, &quot;A&quot;);<br>
&emsp;write_file(tmp / &quot;x/2.txt&quot;, &quot;B&quot;);<br>
&emsp;write_file(tmp / &quot;x/y/3.txt&quot;, &quot;C&quot;);<br>
<br>
&emsp;auto out = expand_glob((tmp / &quot;**&quot;).generic_string());<br>
&emsp;auto rel = to_rel(out, tmp);<br>
<br>
&emsp;check_paths(rel, {&quot;1.txt&quot;, &quot;x/2.txt&quot;, &quot;x/y/3.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: **/*.cpp&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
&emsp;write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);<br>
&emsp;write_file(tmp / &quot;x/c.cpp&quot;, &quot;C&quot;);<br>
&emsp;write_file(tmp / &quot;x/y/z.cpp&quot;, &quot;Z&quot;);<br>
<br>
&emsp;auto out = expand_glob((tmp / &quot;**/*.cpp&quot;).generic_string());<br>
&emsp;auto rel = to_rel(out, tmp);<br>
<br>
&emsp;check_paths(rel, {&quot;a.cpp&quot;, &quot;x/c.cpp&quot;, &quot;x/y/z.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: foo/*/bar/**/*.txt&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;foo/K/bar/a.txt&quot;, &quot;A&quot;);<br>
&emsp;write_file(tmp / &quot;foo/K/bar/x/b.txt&quot;, &quot;B&quot;);<br>
&emsp;write_file(tmp / &quot;foo/X/bar/c.bin&quot;, &quot;C&quot;);<br>
&emsp;write_file(tmp / &quot;foo/Z/bar/y/z.txt&quot;, &quot;Z&quot;);<br>
<br>
&emsp;auto pat = (tmp / &quot;foo/*/bar/**/*.txt&quot;).generic_string();<br>
&emsp;auto out = expand_glob(pat);<br>
&emsp;auto rel = to_rel(out, tmp);<br>
<br>
&emsp;check_paths(rel,<br>
&emsp;&emsp;&emsp;&emsp;{&quot;foo/K/bar/a.txt&quot;, &quot;foo/K/bar/x/b.txt&quot;, &quot;foo/Z/bar/y/z.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: no matches returns empty&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);<br>
<br>
&emsp;auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());<br>
&emsp;CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;glob: duplicates removed&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_f&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
<br>
&emsp;write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
&emsp;write_file(tmp / &quot;d/a.txt&quot;, &quot;A2&quot;);<br>
<br>
&emsp;// отработает и * и **<br>
&emsp;auto pat1 = (tmp / &quot;*.txt&quot;).generic_string();<br>
&emsp;auto pat2 = (tmp / &quot;**/*.txt&quot;).generic_string();<br>
<br>
&emsp;auto out1 = expand_glob(pat1);<br>
&emsp;auto out2 = expand_glob(pat2);<br>
<br>
&emsp;std::vector&lt;fs::path&gt; combined;<br>
&emsp;combined.insert(combined.end(), out1.begin(), out1.end());<br>
&emsp;combined.insert(combined.end(), out2.begin(), out2.end());<br>
<br>
&emsp;// удаляем дубликаты вручную, как collector делает<br>
&emsp;std::sort(combined.begin(), combined.end());<br>
&emsp;combined.erase(std::unique(combined.begin(), combined.end()),<br>
&emsp;&emsp;&emsp;&emsp;combined.end());<br>
<br>
&emsp;auto rel = to_rel(combined, tmp);<br>
&emsp;check_paths(rel, {&quot;a.txt&quot;, &quot;d/a.txt&quot;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

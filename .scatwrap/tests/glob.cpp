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
&#9;fs::create_directories(p.parent_path());<br>
&#9;std::ofstream out(p);<br>
&#9;out &lt;&lt; text;<br>
}<br>
<br>
static std::vector&lt;std::string&gt; to_rel(const std::vector&lt;fs::path&gt; &amp;v,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const fs::path &amp;root)<br>
{<br>
&#9;std::vector&lt;std::string&gt; out;<br>
&#9;for (auto &amp;p : v)<br>
&#9;&#9;out.push_back(fs::relative(p, root).generic_string());<br>
&#9;std::sort(out.begin(), out.end());<br>
&#9;return out;<br>
}<br>
<br>
static void check_paths(const std::vector&lt;std::string&gt; &amp;actual,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::initializer_list&lt;const char *&gt; expected)<br>
{<br>
&#9;CHECK(actual.size() == expected.size());<br>
<br>
&#9;size_t i = 0;<br>
&#9;for (auto *e : expected)<br>
&#9;{<br>
&#9;&#9;CHECK(actual[i] == e);<br>
&#9;&#9;++i;<br>
&#9;}<br>
}<br>
<br>
TEST_CASE(&quot;glob: basic *&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;b.cpp&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;c.txt&quot;, &quot;C&quot;);<br>
<br>
&#9;auto out = expand_glob((tmp / &quot;*.txt&quot;).generic_string());<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;a.txt&quot;, &quot;c.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: recursive **&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;1.txt&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;x/2.txt&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;x/y/3.txt&quot;, &quot;C&quot;);<br>
<br>
&#9;auto out = expand_glob((tmp / &quot;**&quot;).generic_string());<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;1.txt&quot;, &quot;x/2.txt&quot;, &quot;x/y/3.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: **/*.cpp&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;x/c.cpp&quot;, &quot;C&quot;);<br>
&#9;write_file(tmp / &quot;x/y/z.cpp&quot;, &quot;Z&quot;);<br>
<br>
&#9;auto out = expand_glob((tmp / &quot;**/*.cpp&quot;).generic_string());<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;a.cpp&quot;, &quot;x/c.cpp&quot;, &quot;x/y/z.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: foo/*/bar/**/*.txt&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;foo/K/bar/a.txt&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;foo/K/bar/x/b.txt&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;foo/X/bar/c.bin&quot;, &quot;C&quot;);<br>
&#9;write_file(tmp / &quot;foo/Z/bar/y/z.txt&quot;, &quot;Z&quot;);<br>
<br>
&#9;auto pat = (tmp / &quot;foo/*/bar/**/*.txt&quot;).generic_string();<br>
&#9;auto out = expand_glob(pat);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel,<br>
&#9;&#9;&#9;&#9;{&quot;foo/K/bar/a.txt&quot;, &quot;foo/K/bar/x/b.txt&quot;, &quot;foo/Z/bar/y/z.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: no matches returns empty&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);<br>
<br>
&#9;auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());<br>
&#9;CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;glob: duplicates removed&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_f&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;d/a.txt&quot;, &quot;A2&quot;);<br>
<br>
&#9;// отработает и * и **<br>
&#9;auto pat1 = (tmp / &quot;*.txt&quot;).generic_string();<br>
&#9;auto pat2 = (tmp / &quot;**/*.txt&quot;).generic_string();<br>
<br>
&#9;auto out1 = expand_glob(pat1);<br>
&#9;auto out2 = expand_glob(pat2);<br>
<br>
&#9;std::vector&lt;fs::path&gt; combined;<br>
&#9;combined.insert(combined.end(), out1.begin(), out1.end());<br>
&#9;combined.insert(combined.end(), out2.begin(), out2.end());<br>
<br>
&#9;// удаляем дубликаты вручную, как collector делает<br>
&#9;std::sort(combined.begin(), combined.end());<br>
&#9;combined.erase(std::unique(combined.begin(), combined.end()),<br>
&#9;&#9;&#9;&#9;combined.end());<br>
<br>
&#9;auto rel = to_rel(combined, tmp);<br>
&#9;check_paths(rel, {&quot;a.txt&quot;, &quot;d/a.txt&quot;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/glob.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &#20;&quot;glob.h&quot;<br>
#include &#20;&quot;doctest/doctest.h&quot;<br>
#include &#20;&lt;algorithm&gt;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;fstream&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
namespace &#20;fs &#20;= &#20;std::filesystem;<br>
<br>
static &#20;void &#20;write_file(const &#20;fs::path &#20;&amp;p, &#20;const &#20;std::string &#20;&amp;text)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::create_directories(p.parent_path());<br>
 &#20; &#20; &#20; &#20;std::ofstream &#20;out(p);<br>
 &#20; &#20; &#20; &#20;out &#20;&lt;&lt; &#20;text;<br>
}<br>
<br>
static &#20;std::vector&lt;std::string&gt; &#20;to_rel(const &#20;std::vector&lt;fs::path&gt; &#20;&amp;v,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;const &#20;fs::path &#20;&amp;root)<br>
{<br>
 &#20; &#20; &#20; &#20;std::vector&lt;std::string&gt; &#20;out;<br>
 &#20; &#20; &#20; &#20;for &#20;(auto &#20;&amp;p &#20;: &#20;v)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;out.push_back(fs::relative(p, &#20;root).generic_string());<br>
 &#20; &#20; &#20; &#20;std::sort(out.begin(), &#20;out.end());<br>
 &#20; &#20; &#20; &#20;return &#20;out;<br>
}<br>
<br>
static &#20;void &#20;check_paths(const &#20;std::vector&lt;std::string&gt; &#20;&amp;actual,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::initializer_list&lt;const &#20;char &#20;*&gt; &#20;expected)<br>
{<br>
 &#20; &#20; &#20; &#20;CHECK(actual.size() &#20;== &#20;expected.size());<br>
<br>
 &#20; &#20; &#20; &#20;size_t &#20;i &#20;= &#20;0;<br>
 &#20; &#20; &#20; &#20;for &#20;(auto &#20;*e &#20;: &#20;expected)<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;CHECK(actual[i] &#20;== &#20;e);<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;++i;<br>
 &#20; &#20; &#20; &#20;}<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;basic &#20;*&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_a&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.txt&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;b.cpp&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;c.txt&quot;, &#20;&quot;C&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob((tmp &#20;/ &#20;&quot;*.txt&quot;).generic_string());<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;a.txt&quot;, &#20;&quot;c.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;recursive &#20;**&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_b&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;1.txt&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/2.txt&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/y/3.txt&quot;, &#20;&quot;C&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob((tmp &#20;/ &#20;&quot;**&quot;).generic_string());<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;1.txt&quot;, &#20;&quot;x/2.txt&quot;, &#20;&quot;x/y/3.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;**/*.cpp&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_c&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.cpp&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;b.h&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/c.cpp&quot;, &#20;&quot;C&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/y/z.cpp&quot;, &#20;&quot;Z&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob((tmp &#20;/ &#20;&quot;**/*.cpp&quot;).generic_string());<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;a.cpp&quot;, &#20;&quot;x/c.cpp&quot;, &#20;&quot;x/y/z.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;foo/*/bar/**/*.txt&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_d&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;foo/K/bar/a.txt&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;foo/K/bar/x/b.txt&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;foo/X/bar/c.bin&quot;, &#20;&quot;C&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;foo/Z/bar/y/z.txt&quot;, &#20;&quot;Z&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;pat &#20;= &#20;(tmp &#20;/ &#20;&quot;foo/*/bar/**/*.txt&quot;).generic_string();<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob(pat);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{&quot;foo/K/bar/a.txt&quot;, &#20;&quot;foo/K/bar/x/b.txt&quot;, &#20;&quot;foo/Z/bar/y/z.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;no &#20;matches &#20;returns &#20;empty&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_e&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;file.cpp&quot;, &#20;&quot;A&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob((tmp &#20;/ &#20;&quot;**/*.txt&quot;).generic_string());<br>
 &#20; &#20; &#20; &#20;CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;glob: &#20;duplicates &#20;removed&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_f&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.txt&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;d/a.txt&quot;, &#20;&quot;A2&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;отработает &#20;и &#20;* &#20;и &#20;**<br>
 &#20; &#20; &#20; &#20;auto &#20;pat1 &#20;= &#20;(tmp &#20;/ &#20;&quot;*.txt&quot;).generic_string();<br>
 &#20; &#20; &#20; &#20;auto &#20;pat2 &#20;= &#20;(tmp &#20;/ &#20;&quot;**/*.txt&quot;).generic_string();<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out1 &#20;= &#20;expand_glob(pat1);<br>
 &#20; &#20; &#20; &#20;auto &#20;out2 &#20;= &#20;expand_glob(pat2);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;fs::path&gt; &#20;combined;<br>
 &#20; &#20; &#20; &#20;combined.insert(combined.end(), &#20;out1.begin(), &#20;out1.end());<br>
 &#20; &#20; &#20; &#20;combined.insert(combined.end(), &#20;out2.begin(), &#20;out2.end());<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;удаляем &#20;дубликаты &#20;вручную, &#20;как &#20;collector &#20;делает<br>
 &#20; &#20; &#20; &#20;std::sort(combined.begin(), &#20;combined.end());<br>
 &#20; &#20; &#20; &#20;combined.erase(std::unique(combined.begin(), &#20;combined.end()),<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;combined.end());<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(combined, &#20;tmp);<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;a.txt&quot;, &#20;&quot;d/a.txt&quot;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

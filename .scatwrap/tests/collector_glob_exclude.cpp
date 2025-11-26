<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/collector_glob_exclude.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &#20;&quot;collector.h&quot;<br>
#include &#20;&quot;doctest/doctest.h&quot;<br>
#include &#20;&quot;glob.h&quot;<br>
#include &#20;&quot;options.h&quot;<br>
#include &#20;&quot;rules.h&quot;<br>
<br>
#include &#20;&lt;algorithm&gt;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;fstream&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
namespace &#20;fs &#20;= &#20;std::filesystem;<br>
<br>
// &#20;-----------------------------------------------------------------------------<br>
// &#20;Helpers<br>
// &#20;-----------------------------------------------------------------------------<br>
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
 &#20; &#20; &#20; &#20;std::vector&lt;std::string&gt; &#20;exp_vec;<br>
 &#20; &#20; &#20; &#20;exp_vec.reserve(expected.size());<br>
 &#20; &#20; &#20; &#20;for &#20;(auto &#20;*e &#20;: &#20;expected)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;exp_vec.emplace_back(e);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;std::string&gt; &#20;act &#20;= &#20;actual;<br>
<br>
 &#20; &#20; &#20; &#20;std::sort(exp_vec.begin(), &#20;exp_vec.end());<br>
 &#20; &#20; &#20; &#20;std::sort(act.begin(), &#20;act.end());<br>
<br>
 &#20; &#20; &#20; &#20;CHECK(act.size() &#20;== &#20;exp_vec.size());<br>
<br>
 &#20; &#20; &#20; &#20;if &#20;(act.size() &#20;!= &#20;exp_vec.size())<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return; &#20;// &#20;предотвратить &#20;каскад &#20;ошибок<br>
<br>
 &#20; &#20; &#20; &#20;for &#20;(size_t &#20;i &#20;= &#20;0; &#20;i &#20;&lt; &#20;act.size(); &#20;++i)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;CHECK(act[i] &#20;== &#20;exp_vec[i]);<br>
}<br>
<br>
// &#20;============================================================================<br>
// &#20;glob &#20;tests &#20;— &#20;как &#20;раньше, &#20;без &#20;смены &#20;current_path<br>
// &#20;============================================================================<br>
<br>
TEST_CASE(&quot;glob: &#20;basic &#20;*&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_a2&quot;;<br>
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
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_b2&quot;;<br>
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
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_c2&quot;;<br>
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
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_d2&quot;;<br>
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
TEST_CASE(&quot;glob: &#20;no &#20;matches&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;glob_test_e2&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;file.cpp&quot;, &#20;&quot;A&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;expand_glob((tmp &#20;/ &#20;&quot;**/*.txt&quot;).generic_string());<br>
 &#20; &#20; &#20; &#20;CHECK(out.empty());<br>
}<br>
<br>
// &#20;============================================================================<br>
// &#20;collector &#20;tests &#20;— &#20;тоже &#20;на &#20;абсолютных &#20;путях<br>
// &#20;============================================================================<br>
<br>
TEST_CASE(&quot;collector: &#20;include &#20;then &#20;exclude &#20;subdir&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_test_1&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/a.cpp&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/b.cpp&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/tests/c.cpp&quot;, &#20;&quot;C&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{{(tmp &#20;/ &#20;&quot;src/**/*.cpp&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;src/tests/**&quot;).generic_string(), &#20;true}};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;src/a.cpp&quot;, &#20;&quot;src/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: &#20;exclude &#20;*.cpp &#20;top &#20;level&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_test_2&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.cpp&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;b.h&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;sub/c.cpp&quot;, &#20;&quot;C&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;**&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;*.cpp&quot;).generic_string(), &#20;true},<br>
 &#20; &#20; &#20; &#20;};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;b.h&quot;, &#20;&quot;sub/c.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: &#20;exclude &#20;all &#20;cpp &#20;recursively&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_test_3&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.cpp&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/b.cpp&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;x/y/c.cpp&quot;, &#20;&quot;C&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;ok.txt&quot;, &#20;&quot;D&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;**&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;**/*.cpp&quot;).generic_string(), &#20;true},<br>
 &#20; &#20; &#20; &#20;};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;ok.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: &#20;include-exclude-include &#20;chain&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_test_4&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/keep/a.txt&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/keep/b.cpp&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/drop/c.txt&quot;, &#20;&quot;C&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;data/**&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;data/drop/**&quot;).generic_string(), &#20;true},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;data/keep/*.txt&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20;};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;check_paths(rel, &#20;{&quot;data/keep/a.txt&quot;, &#20;&quot;data/keep/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: &#20;empty &#20;after &#20;exclude&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_test_5&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;a.txt&quot;, &#20;&quot;A&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;*.txt&quot;).generic_string(), &#20;false},<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;*.txt&quot;).generic_string(), &#20;true},<br>
 &#20; &#20; &#20; &#20;};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
<br>
 &#20; &#20; &#20; &#20;CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;collector: &#20;complex &#20;structure &#20;with &#20;5 &#20;rules&quot;)<br>
{<br>
 &#20; &#20; &#20; &#20;fs::path &#20;tmp &#20;= &#20;fs::temp_directory_path() &#20;/ &#20;&quot;collector_complex_test&quot;;<br>
 &#20; &#20; &#20; &#20;fs::remove_all(tmp);<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
 &#20; &#20; &#20; &#20;// &#20;создаём &#20;структуру &#20;на &#20;15 &#20;файлов<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;root.txt&quot;, &#20;&quot;R&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;root.bin&quot;, &#20;&quot;RB&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;readme.md&quot;, &#20;&quot;MD&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/a.cpp&quot;, &#20;&quot;A&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/b.cpp&quot;, &#20;&quot;B&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/c.log&quot;, &#20;&quot;CLOG&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/tools/t1.txt&quot;, &#20;&quot;T1&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/tools/t2.txt&quot;, &#20;&quot;T2&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;src/tools/helper.cpp&quot;, &#20;&quot;HC&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/info.txt&quot;, &#20;&quot;INFO&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/raw/raw1.bin&quot;, &#20;&quot;BIN1&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;data/raw/raw2.bin&quot;, &#20;&quot;BIN2&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;build/tmp1.log&quot;, &#20;&quot;TMP1&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;build/tmp2.log&quot;, &#20;&quot;TMP2&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;misc/x.cpp&quot;, &#20;&quot;X&quot;);<br>
 &#20; &#20; &#20; &#20;write_file(tmp &#20;/ &#20;&quot;misc/y.txt&quot;, &#20;&quot;Y&quot;);<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
 &#20; &#20; &#20; &#20;// &#20;ПРАВИЛА<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;rules &#20;= &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;1) &#20;собрать &#20;всё<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;**&quot;).generic_string(), &#20;false},<br>
<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;2) &#20;исключить &#20;data/raw/**<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;data/raw/**&quot;).generic_string(), &#20;true},<br>
<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;3) &#20;исключить &#20;все &#20;*.log<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;**/*.log&quot;).generic_string(), &#20;true},<br>
<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;4) &#20;добавить &#20;текстовые &#20;файлы &#20;только &#20;в &#20;tools<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;src/tools/*.txt&quot;).generic_string(), &#20;false},<br>
<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;5) &#20;исключить &#20;бинарники &#20;только &#20;в &#20;корне &#20;(root.bin)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{(tmp &#20;/ &#20;&quot;*.bin&quot;).generic_string(), &#20;true}};<br>
<br>
 &#20; &#20; &#20; &#20;Options &#20;opt;<br>
 &#20; &#20; &#20; &#20;auto &#20;out &#20;= &#20;collect_from_rules(rules, &#20;opt);<br>
 &#20; &#20; &#20; &#20;auto &#20;rel &#20;= &#20;to_rel(out, &#20;tmp);<br>
<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
 &#20; &#20; &#20; &#20;// &#20;ОЖИДАЕМЫЙ &#20;РЕЗУЛЬТАТ<br>
 &#20; &#20; &#20; &#20;// &#20;----------------------------<br>
 &#20; &#20; &#20; &#20;//<br>
 &#20; &#20; &#20; &#20;// &#20;Файлы, &#20;которые &#20;должны &#20;остаться:<br>
 &#20; &#20; &#20; &#20;//<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;root.txt<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;readme.md<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;src/a.cpp<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;src/b.cpp<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;src/tools/t1.txt<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;src/tools/t2.txt<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;src/tools/helper.cpp<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;data/info.txt<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;misc/x.cpp<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;misc/y.txt<br>
 &#20; &#20; &#20; &#20;//<br>
 &#20; &#20; &#20; &#20;// &#20;НЕ &#20;должно &#20;быть:<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;data/raw/* &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;(правило &#20;#2)<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;*.log &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;(правило &#20;#3)<br>
 &#20; &#20; &#20; &#20;// &#20; &#20; &#20;root.bin &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;(правило &#20;#5)<br>
 &#20; &#20; &#20; &#20;//<br>
 &#20; &#20; &#20; &#20;check_paths(rel,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;root.txt&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;readme.md&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;src/a.cpp&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;src/b.cpp&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;src/tools/t1.txt&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;src/tools/t2.txt&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;src/tools/helper.cpp&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;data/info.txt&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;misc/x.cpp&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;&quot;misc/y.txt&quot;,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

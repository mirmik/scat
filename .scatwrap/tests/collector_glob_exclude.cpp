<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/collector_glob_exclude.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;collector.h&quot;<br>
#include &quot;doctest/doctest.h&quot;<br>
#include &quot;glob.h&quot;<br>
#include &quot;options.h&quot;<br>
#include &quot;rules.h&quot;<br>
<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
// -----------------------------------------------------------------------------<br>
// Helpers<br>
// -----------------------------------------------------------------------------<br>
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
&#9;std::vector&lt;std::string&gt; exp_vec;<br>
&#9;exp_vec.reserve(expected.size());<br>
&#9;for (auto *e : expected)<br>
&#9;&#9;exp_vec.emplace_back(e);<br>
<br>
&#9;std::vector&lt;std::string&gt; act = actual;<br>
<br>
&#9;std::sort(exp_vec.begin(), exp_vec.end());<br>
&#9;std::sort(act.begin(), act.end());<br>
<br>
&#9;CHECK(act.size() == exp_vec.size());<br>
<br>
&#9;if (act.size() != exp_vec.size())<br>
&#9;&#9;return; // предотвратить каскад ошибок<br>
<br>
&#9;for (size_t i = 0; i &lt; act.size(); ++i)<br>
&#9;&#9;CHECK(act[i] == exp_vec[i]);<br>
}<br>
<br>
// ============================================================================<br>
// glob tests — как раньше, без смены current_path<br>
// ============================================================================<br>
<br>
TEST_CASE(&quot;glob: basic *&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a2&quot;;<br>
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
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b2&quot;;<br>
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
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c2&quot;;<br>
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
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d2&quot;;<br>
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
TEST_CASE(&quot;glob: no matches&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);<br>
<br>
&#9;auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());<br>
&#9;CHECK(out.empty());<br>
}<br>
<br>
// ============================================================================<br>
// collector tests — тоже на абсолютных путях<br>
// ============================================================================<br>
<br>
TEST_CASE(&quot;collector: include then exclude subdir&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_test_1&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;src/tests/c.cpp&quot;, &quot;C&quot;);<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {{(tmp / &quot;src/**/*.cpp&quot;).generic_string(), false},<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;{(tmp / &quot;src/tests/**&quot;).generic_string(), true}};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;src/a.cpp&quot;, &quot;src/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: exclude *.cpp top level&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_test_2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;sub/c.cpp&quot;, &quot;C&quot;);<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {<br>
&#9;&#9;{(tmp / &quot;**&quot;).generic_string(), false},<br>
&#9;&#9;{(tmp / &quot;*.cpp&quot;).generic_string(), true},<br>
&#9;};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;b.h&quot;, &quot;sub/c.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: exclude all cpp recursively&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_test_3&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;x/b.cpp&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;x/y/c.cpp&quot;, &quot;C&quot;);<br>
&#9;write_file(tmp / &quot;ok.txt&quot;, &quot;D&quot;);<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {<br>
&#9;&#9;{(tmp / &quot;**&quot;).generic_string(), false},<br>
&#9;&#9;{(tmp / &quot;**/*.cpp&quot;).generic_string(), true},<br>
&#9;};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;ok.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: include-exclude-include chain&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_test_4&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;data/keep/a.txt&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;data/keep/b.cpp&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;data/drop/c.txt&quot;, &quot;C&quot;);<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {<br>
&#9;&#9;{(tmp / &quot;data/**&quot;).generic_string(), false},<br>
&#9;&#9;{(tmp / &quot;data/drop/**&quot;).generic_string(), true},<br>
&#9;&#9;{(tmp / &quot;data/keep/*.txt&quot;).generic_string(), false},<br>
&#9;};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;check_paths(rel, {&quot;data/keep/a.txt&quot;, &quot;data/keep/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: empty after exclude&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_test_5&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {<br>
&#9;&#9;{(tmp / &quot;*.txt&quot;).generic_string(), false},<br>
&#9;&#9;{(tmp / &quot;*.txt&quot;).generic_string(), true},<br>
&#9;};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
<br>
&#9;CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;collector: complex structure with 5 rules&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;collector_complex_test&quot;;<br>
&#9;fs::remove_all(tmp);<br>
<br>
&#9;// ----------------------------<br>
&#9;// создаём структуру на 15 файлов<br>
&#9;// ----------------------------<br>
&#9;write_file(tmp / &quot;root.txt&quot;, &quot;R&quot;);<br>
&#9;write_file(tmp / &quot;root.bin&quot;, &quot;RB&quot;);<br>
&#9;write_file(tmp / &quot;readme.md&quot;, &quot;MD&quot;);<br>
<br>
&#9;write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);<br>
&#9;write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);<br>
&#9;write_file(tmp / &quot;src/c.log&quot;, &quot;CLOG&quot;);<br>
<br>
&#9;write_file(tmp / &quot;src/tools/t1.txt&quot;, &quot;T1&quot;);<br>
&#9;write_file(tmp / &quot;src/tools/t2.txt&quot;, &quot;T2&quot;);<br>
&#9;write_file(tmp / &quot;src/tools/helper.cpp&quot;, &quot;HC&quot;);<br>
<br>
&#9;write_file(tmp / &quot;data/info.txt&quot;, &quot;INFO&quot;);<br>
&#9;write_file(tmp / &quot;data/raw/raw1.bin&quot;, &quot;BIN1&quot;);<br>
&#9;write_file(tmp / &quot;data/raw/raw2.bin&quot;, &quot;BIN2&quot;);<br>
<br>
&#9;write_file(tmp / &quot;build/tmp1.log&quot;, &quot;TMP1&quot;);<br>
&#9;write_file(tmp / &quot;build/tmp2.log&quot;, &quot;TMP2&quot;);<br>
<br>
&#9;write_file(tmp / &quot;misc/x.cpp&quot;, &quot;X&quot;);<br>
&#9;write_file(tmp / &quot;misc/y.txt&quot;, &quot;Y&quot;);<br>
<br>
&#9;// ----------------------------<br>
&#9;// ПРАВИЛА<br>
&#9;// ----------------------------<br>
<br>
&#9;std::vector&lt;Rule&gt; rules = {<br>
&#9;&#9;// 1) собрать всё<br>
&#9;&#9;{(tmp / &quot;**&quot;).generic_string(), false},<br>
<br>
&#9;&#9;// 2) исключить data/raw/**<br>
&#9;&#9;{(tmp / &quot;data/raw/**&quot;).generic_string(), true},<br>
<br>
&#9;&#9;// 3) исключить все *.log<br>
&#9;&#9;{(tmp / &quot;**/*.log&quot;).generic_string(), true},<br>
<br>
&#9;&#9;// 4) добавить текстовые файлы только в tools<br>
&#9;&#9;{(tmp / &quot;src/tools/*.txt&quot;).generic_string(), false},<br>
<br>
&#9;&#9;// 5) исключить бинарники только в корне (root.bin)<br>
&#9;&#9;{(tmp / &quot;*.bin&quot;).generic_string(), true}};<br>
<br>
&#9;Options opt;<br>
&#9;auto out = collect_from_rules(rules, opt);<br>
&#9;auto rel = to_rel(out, tmp);<br>
<br>
&#9;// ----------------------------<br>
&#9;// ОЖИДАЕМЫЙ РЕЗУЛЬТАТ<br>
&#9;// ----------------------------<br>
&#9;//<br>
&#9;// Файлы, которые должны остаться:<br>
&#9;//<br>
&#9;//   root.txt<br>
&#9;//   readme.md<br>
&#9;//   src/a.cpp<br>
&#9;//   src/b.cpp<br>
&#9;//   src/tools/t1.txt<br>
&#9;//   src/tools/t2.txt<br>
&#9;//   src/tools/helper.cpp<br>
&#9;//   data/info.txt<br>
&#9;//   misc/x.cpp<br>
&#9;//   misc/y.txt<br>
&#9;//<br>
&#9;// НЕ должно быть:<br>
&#9;//   data/raw/*        (правило #2)<br>
&#9;//   *.log             (правило #3)<br>
&#9;//   root.bin          (правило #5)<br>
&#9;//<br>
&#9;check_paths(rel,<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&quot;root.txt&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;readme.md&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;src/a.cpp&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;src/b.cpp&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;src/tools/t1.txt&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;src/tools/t2.txt&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;src/tools/helper.cpp&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;data/info.txt&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;misc/x.cpp&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&quot;misc/y.txt&quot;,<br>
&#9;&#9;&#9;&#9;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

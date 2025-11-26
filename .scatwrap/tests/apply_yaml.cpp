<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply_yaml.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
int apply_chunk_main(int argc, char **argv);<br>
<br>
namespace fs = std::filesystem;<br>
<br>
static std::vector&lt;std::string&gt; read_lines(const fs::path &amp;p)<br>
{<br>
&emsp;std::ifstream in(p);<br>
&emsp;std::vector&lt;std::string&gt; v;<br>
&emsp;std::string s;<br>
&emsp;while (std::getline(in, s))<br>
&emsp;&emsp;v.push_back(s);<br>
&emsp;return v;<br>
}<br>
<br>
static int run_apply(const fs::path &amp;patch)<br>
{<br>
&emsp;std::string a0 = &quot;apply&quot;;<br>
&emsp;std::string a1 = patch.string();<br>
<br>
&emsp;std::vector&lt;std::string&gt; store = {a0, a1};<br>
&emsp;std::vector&lt;char *&gt; argv;<br>
&emsp;for (auto &amp;s : store)<br>
&emsp;&emsp;argv.push_back(s.data());<br>
<br>
&emsp;return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// 1. MARKER: без BEFORE/AFTER — работает как старый режим<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: only MARKER: behaves like legacy replace-text&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;yaml_marker_only_test&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;a.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch1.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;MARKER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;B\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 3);<br>
&emsp;CHECK(L[0] == &quot;A&quot;);<br>
&emsp;CHECK(L[1] == &quot;X&quot;);<br>
&emsp;CHECK(L[2] == &quot;C&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 2. BEFORE fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: BEFORE fuzzy selects the correct marker&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_test2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;b.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;foo\n&quot;<br>
&emsp;&emsp;&emsp;&quot;target\n&quot;<br>
&emsp;&emsp;&emsp;&quot;bar\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;XXX\n&quot;<br>
&emsp;&emsp;&emsp;&quot;target\n&quot;<br>
&emsp;&emsp;&emsp;&quot;YYY\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch2.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BEFORE:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;XXX\n&quot;<br>
&emsp;&emsp;&emsp;&quot;MARKER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;target\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;SECOND\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 7);<br>
&emsp;CHECK(L[5] == &quot;SECOND&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 3. AFTER fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: AFTER fuzzy selects correct block&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;yaml_after_test2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;c.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;log\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;done\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;log\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;finish\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch3.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;MARKER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;AFTER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;finish\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;CHANGED\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 7);<br>
&emsp;CHECK(L[5] == &quot;CHANGED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 4. BEFORE + AFTER together<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: strong fuzzy match with BEFORE + AFTER&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_after_test2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;d.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;A\n&quot;<br>
&emsp;&emsp;&emsp;&quot;mark\n&quot;<br>
&emsp;&emsp;&emsp;&quot;B\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;C\n&quot;<br>
&emsp;&emsp;&emsp;&quot;mark\n&quot;<br>
&emsp;&emsp;&emsp;&quot;D\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch4.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BEFORE:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;C\n&quot;<br>
&emsp;&emsp;&emsp;&quot;MARKER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;mark\n&quot;<br>
&emsp;&emsp;&emsp;&quot;AFTER:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;D\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;SELECTED\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 7);<br>
&emsp;CHECK(L[5] == &quot;SELECTED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 7. Legacy format still works<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: legacy replace-text still works&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;yaml_legacy_test2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;g.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;3\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch7.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;CHECK(L[1] == &quot;X&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

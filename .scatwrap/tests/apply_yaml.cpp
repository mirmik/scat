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
&#9;std::ifstream in(p);<br>
&#9;std::vector&lt;std::string&gt; v;<br>
&#9;std::string s;<br>
&#9;while (std::getline(in, s))<br>
&#9;&#9;v.push_back(s);<br>
&#9;return v;<br>
}<br>
<br>
static int run_apply(const fs::path &amp;patch)<br>
{<br>
&#9;std::string a0 = &quot;apply&quot;;<br>
&#9;std::string a1 = patch.string();<br>
<br>
&#9;std::vector&lt;std::string&gt; store = {a0, a1};<br>
&#9;std::vector&lt;char *&gt; argv;<br>
&#9;for (auto &amp;s : store)<br>
&#9;&#9;argv.push_back(s.data());<br>
<br>
&#9;return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// 1. MARKER: без BEFORE/AFTER — работает как старый режим<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: only MARKER: behaves like legacy replace-text&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;yaml_marker_only_test&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;a.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch1.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;MARKER:\n&quot;<br>
&#9;&#9;&#9;&quot;B\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 3);<br>
&#9;CHECK(L[0] == &quot;A&quot;);<br>
&#9;CHECK(L[1] == &quot;X&quot;);<br>
&#9;CHECK(L[2] == &quot;C&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 2. BEFORE fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: BEFORE fuzzy selects the correct marker&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_test2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;b.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;foo\n&quot;<br>
&#9;&#9;&#9;&quot;target\n&quot;<br>
&#9;&#9;&#9;&quot;bar\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;XXX\n&quot;<br>
&#9;&#9;&#9;&quot;target\n&quot;<br>
&#9;&#9;&#9;&quot;YYY\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch2.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;BEFORE:\n&quot;<br>
&#9;&#9;&#9;&quot;XXX\n&quot;<br>
&#9;&#9;&#9;&quot;MARKER:\n&quot;<br>
&#9;&#9;&#9;&quot;target\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;SECOND\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 7);<br>
&#9;CHECK(L[5] == &quot;SECOND&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 3. AFTER fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: AFTER fuzzy selects correct block&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;yaml_after_test2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;c.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;log\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;done\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;log\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;finish\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch3.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;MARKER:\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;AFTER:\n&quot;<br>
&#9;&#9;&#9;&quot;finish\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;CHANGED\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 7);<br>
&#9;CHECK(L[5] == &quot;CHANGED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 4. BEFORE + AFTER together<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: strong fuzzy match with BEFORE + AFTER&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_after_test2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;d.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;A\n&quot;<br>
&#9;&#9;&#9;&quot;mark\n&quot;<br>
&#9;&#9;&#9;&quot;B\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;C\n&quot;<br>
&#9;&#9;&#9;&quot;mark\n&quot;<br>
&#9;&#9;&#9;&quot;D\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch4.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;BEFORE:\n&quot;<br>
&#9;&#9;&#9;&quot;C\n&quot;<br>
&#9;&#9;&#9;&quot;MARKER:\n&quot;<br>
&#9;&#9;&#9;&quot;mark\n&quot;<br>
&#9;&#9;&#9;&quot;AFTER:\n&quot;<br>
&#9;&#9;&#9;&quot;D\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;SELECTED\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 7);<br>
&#9;CHECK(L[5] == &quot;SELECTED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 7. Legacy format still works<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: legacy replace-text still works&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;yaml_legacy_test2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;g.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;1\n&quot;<br>
&#9;&#9;&#9;&quot;2\n&quot;<br>
&#9;&#9;&#9;&quot;3\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch7.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;2\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;CHECK(L[1] == &quot;X&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

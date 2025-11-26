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
    std::ifstream in(p);<br>
    std::vector&lt;std::string&gt; v;<br>
    std::string s;<br>
    while (std::getline(in, s))<br>
        v.push_back(s);<br>
    return v;<br>
}<br>
<br>
static int run_apply(const fs::path &amp;patch)<br>
{<br>
    std::string a0 = &quot;apply&quot;;<br>
    std::string a1 = patch.string();<br>
<br>
    std::vector&lt;std::string&gt; store = {a0, a1};<br>
    std::vector&lt;char *&gt; argv;<br>
    for (auto &amp;s : store)<br>
        argv.push_back(s.data());<br>
<br>
    return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// 1. MARKER: без BEFORE/AFTER — работает как старый режим<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: only MARKER: behaves like legacy replace-text&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_marker_only_test&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;a.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch1.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;MARKER:\n&quot;<br>
               &quot;B\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 3);<br>
    CHECK(L[0] == &quot;A&quot;);<br>
    CHECK(L[1] == &quot;X&quot;);<br>
    CHECK(L[2] == &quot;C&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 2. BEFORE fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: BEFORE fuzzy selects the correct marker&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_test2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;b.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;foo\n&quot;<br>
               &quot;target\n&quot;<br>
               &quot;bar\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;XXX\n&quot;<br>
               &quot;target\n&quot;<br>
               &quot;YYY\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch2.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;BEFORE:\n&quot;<br>
               &quot;XXX\n&quot;<br>
               &quot;MARKER:\n&quot;<br>
               &quot;target\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;SECOND\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 7);<br>
    CHECK(L[5] == &quot;SECOND&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 3. AFTER fuzzy<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: AFTER fuzzy selects correct block&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_after_test2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;c.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;log\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;done\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;log\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;finish\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch3.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;MARKER:\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;AFTER:\n&quot;<br>
               &quot;finish\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;CHANGED\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 7);<br>
    CHECK(L[5] == &quot;CHANGED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 4. BEFORE + AFTER together<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: strong fuzzy match with BEFORE + AFTER&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_after_test2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;d.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;A\n&quot;<br>
               &quot;mark\n&quot;<br>
               &quot;B\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;C\n&quot;<br>
               &quot;mark\n&quot;<br>
               &quot;D\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch4.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;BEFORE:\n&quot;<br>
               &quot;C\n&quot;<br>
               &quot;MARKER:\n&quot;<br>
               &quot;mark\n&quot;<br>
               &quot;AFTER:\n&quot;<br>
               &quot;D\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;SELECTED\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 7);<br>
    CHECK(L[5] == &quot;SELECTED&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// 7. Legacy format still works<br>
// ============================================================================<br>
TEST_CASE(&quot;YAML: legacy replace-text still works&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_legacy_test2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;g.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;1\n&quot;<br>
               &quot;2\n&quot;<br>
               &quot;3\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch7.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;2\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    CHECK(L[1] == &quot;X&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

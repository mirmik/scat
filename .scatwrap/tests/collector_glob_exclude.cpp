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
    fs::create_directories(p.parent_path());<br>
    std::ofstream out(p);<br>
    out &lt;&lt; text;<br>
}<br>
<br>
static std::vector&lt;std::string&gt; to_rel(const std::vector&lt;fs::path&gt; &amp;v,<br>
                                       const fs::path &amp;root)<br>
{<br>
    std::vector&lt;std::string&gt; out;<br>
    for (auto &amp;p : v)<br>
        out.push_back(fs::relative(p, root).generic_string());<br>
    std::sort(out.begin(), out.end());<br>
    return out;<br>
}<br>
<br>
static void check_paths(const std::vector&lt;std::string&gt; &amp;actual,<br>
                        std::initializer_list&lt;const char *&gt; expected)<br>
{<br>
    std::vector&lt;std::string&gt; exp_vec;<br>
    exp_vec.reserve(expected.size());<br>
    for (auto *e : expected)<br>
        exp_vec.emplace_back(e);<br>
<br>
    std::vector&lt;std::string&gt; act = actual;<br>
<br>
    std::sort(exp_vec.begin(), exp_vec.end());<br>
    std::sort(act.begin(), act.end());<br>
<br>
    CHECK(act.size() == exp_vec.size());<br>
<br>
    if (act.size() != exp_vec.size())<br>
        return; // предотвратить каскад ошибок<br>
<br>
    for (size_t i = 0; i &lt; act.size(); ++i)<br>
        CHECK(act[i] == exp_vec[i]);<br>
}<br>
<br>
// ============================================================================<br>
// glob tests — как раньше, без смены current_path<br>
// ============================================================================<br>
<br>
TEST_CASE(&quot;glob: basic *&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;b.cpp&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;c.txt&quot;, &quot;C&quot;);<br>
<br>
    auto out = expand_glob((tmp / &quot;*.txt&quot;).generic_string());<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;a.txt&quot;, &quot;c.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: recursive **&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;1.txt&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;x/2.txt&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;x/y/3.txt&quot;, &quot;C&quot;);<br>
<br>
    auto out = expand_glob((tmp / &quot;**&quot;).generic_string());<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;1.txt&quot;, &quot;x/2.txt&quot;, &quot;x/y/3.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: **/*.cpp&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;x/c.cpp&quot;, &quot;C&quot;);<br>
    write_file(tmp / &quot;x/y/z.cpp&quot;, &quot;Z&quot;);<br>
<br>
    auto out = expand_glob((tmp / &quot;**/*.cpp&quot;).generic_string());<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;a.cpp&quot;, &quot;x/c.cpp&quot;, &quot;x/y/z.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: foo/*/bar/**/*.txt&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;foo/K/bar/a.txt&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;foo/K/bar/x/b.txt&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;foo/X/bar/c.bin&quot;, &quot;C&quot;);<br>
    write_file(tmp / &quot;foo/Z/bar/y/z.txt&quot;, &quot;Z&quot;);<br>
<br>
    auto pat = (tmp / &quot;foo/*/bar/**/*.txt&quot;).generic_string();<br>
    auto out = expand_glob(pat);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel,<br>
                {&quot;foo/K/bar/a.txt&quot;, &quot;foo/K/bar/x/b.txt&quot;, &quot;foo/Z/bar/y/z.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;glob: no matches&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);<br>
<br>
    auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());<br>
    CHECK(out.empty());<br>
}<br>
<br>
// ============================================================================<br>
// collector tests — тоже на абсолютных путях<br>
// ============================================================================<br>
<br>
TEST_CASE(&quot;collector: include then exclude subdir&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_1&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;src/tests/c.cpp&quot;, &quot;C&quot;);<br>
<br>
    std::vector&lt;Rule&gt; rules = {{(tmp / &quot;src/**/*.cpp&quot;).generic_string(), false},<br>
                               {(tmp / &quot;src/tests/**&quot;).generic_string(), true}};<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;src/a.cpp&quot;, &quot;src/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: exclude *.cpp top level&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_2&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;sub/c.cpp&quot;, &quot;C&quot;);<br>
<br>
    std::vector&lt;Rule&gt; rules = {<br>
        {(tmp / &quot;**&quot;).generic_string(), false},<br>
        {(tmp / &quot;*.cpp&quot;).generic_string(), true},<br>
    };<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;b.h&quot;, &quot;sub/c.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: exclude all cpp recursively&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_3&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;x/b.cpp&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;x/y/c.cpp&quot;, &quot;C&quot;);<br>
    write_file(tmp / &quot;ok.txt&quot;, &quot;D&quot;);<br>
<br>
    std::vector&lt;Rule&gt; rules = {<br>
        {(tmp / &quot;**&quot;).generic_string(), false},<br>
        {(tmp / &quot;**/*.cpp&quot;).generic_string(), true},<br>
    };<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;ok.txt&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: include-exclude-include chain&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_4&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;data/keep/a.txt&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;data/keep/b.cpp&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;data/drop/c.txt&quot;, &quot;C&quot;);<br>
<br>
    std::vector&lt;Rule&gt; rules = {<br>
        {(tmp / &quot;data/**&quot;).generic_string(), false},<br>
        {(tmp / &quot;data/drop/**&quot;).generic_string(), true},<br>
        {(tmp / &quot;data/keep/*.txt&quot;).generic_string(), false},<br>
    };<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    check_paths(rel, {&quot;data/keep/a.txt&quot;, &quot;data/keep/b.cpp&quot;});<br>
}<br>
<br>
TEST_CASE(&quot;collector: empty after exclude&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_5&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
<br>
    std::vector&lt;Rule&gt; rules = {<br>
        {(tmp / &quot;*.txt&quot;).generic_string(), false},<br>
        {(tmp / &quot;*.txt&quot;).generic_string(), true},<br>
    };<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
<br>
    CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;collector: complex structure with 5 rules&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;collector_complex_test&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    // ----------------------------<br>
    // создаём структуру на 15 файлов<br>
    // ----------------------------<br>
    write_file(tmp / &quot;root.txt&quot;, &quot;R&quot;);<br>
    write_file(tmp / &quot;root.bin&quot;, &quot;RB&quot;);<br>
    write_file(tmp / &quot;readme.md&quot;, &quot;MD&quot;);<br>
<br>
    write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);<br>
    write_file(tmp / &quot;src/c.log&quot;, &quot;CLOG&quot;);<br>
<br>
    write_file(tmp / &quot;src/tools/t1.txt&quot;, &quot;T1&quot;);<br>
    write_file(tmp / &quot;src/tools/t2.txt&quot;, &quot;T2&quot;);<br>
    write_file(tmp / &quot;src/tools/helper.cpp&quot;, &quot;HC&quot;);<br>
<br>
    write_file(tmp / &quot;data/info.txt&quot;, &quot;INFO&quot;);<br>
    write_file(tmp / &quot;data/raw/raw1.bin&quot;, &quot;BIN1&quot;);<br>
    write_file(tmp / &quot;data/raw/raw2.bin&quot;, &quot;BIN2&quot;);<br>
<br>
    write_file(tmp / &quot;build/tmp1.log&quot;, &quot;TMP1&quot;);<br>
    write_file(tmp / &quot;build/tmp2.log&quot;, &quot;TMP2&quot;);<br>
<br>
    write_file(tmp / &quot;misc/x.cpp&quot;, &quot;X&quot;);<br>
    write_file(tmp / &quot;misc/y.txt&quot;, &quot;Y&quot;);<br>
<br>
    // ----------------------------<br>
    // ПРАВИЛА<br>
    // ----------------------------<br>
<br>
    std::vector&lt;Rule&gt; rules = {<br>
        // 1) собрать всё<br>
        {(tmp / &quot;**&quot;).generic_string(), false},<br>
<br>
        // 2) исключить data/raw/**<br>
        {(tmp / &quot;data/raw/**&quot;).generic_string(), true},<br>
<br>
        // 3) исключить все *.log<br>
        {(tmp / &quot;**/*.log&quot;).generic_string(), true},<br>
<br>
        // 4) добавить текстовые файлы только в tools<br>
        {(tmp / &quot;src/tools/*.txt&quot;).generic_string(), false},<br>
<br>
        // 5) исключить бинарники только в корне (root.bin)<br>
        {(tmp / &quot;*.bin&quot;).generic_string(), true}};<br>
<br>
    Options opt;<br>
    auto out = collect_from_rules(rules, opt);<br>
    auto rel = to_rel(out, tmp);<br>
<br>
    // ----------------------------<br>
    // ОЖИДАЕМЫЙ РЕЗУЛЬТАТ<br>
    // ----------------------------<br>
    //<br>
    // Файлы, которые должны остаться:<br>
    //<br>
    //   root.txt<br>
    //   readme.md<br>
    //   src/a.cpp<br>
    //   src/b.cpp<br>
    //   src/tools/t1.txt<br>
    //   src/tools/t2.txt<br>
    //   src/tools/helper.cpp<br>
    //   data/info.txt<br>
    //   misc/x.cpp<br>
    //   misc/y.txt<br>
    //<br>
    // НЕ должно быть:<br>
    //   data/raw/*        (правило #2)<br>
    //   *.log             (правило #3)<br>
    //   root.bin          (правило #5)<br>
    //<br>
    check_paths(rel,<br>
                {<br>
                    &quot;root.txt&quot;,<br>
                    &quot;readme.md&quot;,<br>
                    &quot;src/a.cpp&quot;,<br>
                    &quot;src/b.cpp&quot;,<br>
                    &quot;src/tools/t1.txt&quot;,<br>
                    &quot;src/tools/t2.txt&quot;,<br>
                    &quot;src/tools/helper.cpp&quot;,<br>
                    &quot;data/info.txt&quot;,<br>
                    &quot;misc/x.cpp&quot;,<br>
                    &quot;misc/y.txt&quot;,<br>
                });<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

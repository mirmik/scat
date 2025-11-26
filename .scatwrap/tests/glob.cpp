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
    CHECK(actual.size() == expected.size());<br>
<br>
    size_t i = 0;<br>
    for (auto *e : expected)<br>
    {<br>
        CHECK(actual[i] == e);<br>
        ++i;<br>
    }<br>
}<br>
<br>
TEST_CASE(&quot;glob: basic *&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a&quot;;<br>
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b&quot;;<br>
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c&quot;;<br>
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d&quot;;<br>
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
TEST_CASE(&quot;glob: no matches returns empty&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);<br>
<br>
    auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());<br>
    CHECK(out.empty());<br>
}<br>
<br>
TEST_CASE(&quot;glob: duplicates removed&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_f&quot;;<br>
    fs::remove_all(tmp);<br>
<br>
    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);<br>
    write_file(tmp / &quot;d/a.txt&quot;, &quot;A2&quot;);<br>
<br>
    // отработает и * и **<br>
    auto pat1 = (tmp / &quot;*.txt&quot;).generic_string();<br>
    auto pat2 = (tmp / &quot;**/*.txt&quot;).generic_string();<br>
<br>
    auto out1 = expand_glob(pat1);<br>
    auto out2 = expand_glob(pat2);<br>
<br>
    std::vector&lt;fs::path&gt; combined;<br>
    combined.insert(combined.end(), out1.begin(), out1.end());<br>
    combined.insert(combined.end(), out2.begin(), out2.end());<br>
<br>
    // удаляем дубликаты вручную, как collector делает<br>
    std::sort(combined.begin(), combined.end());<br>
    combined.erase(std::unique(combined.begin(), combined.end()),<br>
                   combined.end());<br>
<br>
    auto rel = to_rel(combined, tmp);<br>
    check_paths(rel, {&quot;a.txt&quot;, &quot;d/a.txt&quot;});<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

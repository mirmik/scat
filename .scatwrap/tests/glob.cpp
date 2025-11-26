<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/glob.cpp</title>
</head>
<body>
<pre><code>
#include &quot;glob.h&quot;
#include &quot;doctest/doctest.h&quot;
#include &lt;algorithm&gt;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;vector&gt;

namespace fs = std::filesystem;

static void write_file(const fs::path &amp;p, const std::string &amp;text)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p);
    out &lt;&lt; text;
}

static std::vector&lt;std::string&gt; to_rel(const std::vector&lt;fs::path&gt; &amp;v,
                                       const fs::path &amp;root)
{
    std::vector&lt;std::string&gt; out;
    for (auto &amp;p : v)
        out.push_back(fs::relative(p, root).generic_string());
    std::sort(out.begin(), out.end());
    return out;
}

static void check_paths(const std::vector&lt;std::string&gt; &amp;actual,
                        std::initializer_list&lt;const char *&gt; expected)
{
    CHECK(actual.size() == expected.size());

    size_t i = 0;
    for (auto *e : expected)
    {
        CHECK(actual[i] == e);
        ++i;
    }
}

TEST_CASE(&quot;glob: basic *&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);
    write_file(tmp / &quot;b.cpp&quot;, &quot;B&quot;);
    write_file(tmp / &quot;c.txt&quot;, &quot;C&quot;);

    auto out = expand_glob((tmp / &quot;*.txt&quot;).generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;a.txt&quot;, &quot;c.txt&quot;});
}

TEST_CASE(&quot;glob: recursive **&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;1.txt&quot;, &quot;A&quot;);
    write_file(tmp / &quot;x/2.txt&quot;, &quot;B&quot;);
    write_file(tmp / &quot;x/y/3.txt&quot;, &quot;C&quot;);

    auto out = expand_glob((tmp / &quot;**&quot;).generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;1.txt&quot;, &quot;x/2.txt&quot;, &quot;x/y/3.txt&quot;});
}

TEST_CASE(&quot;glob: **/*.cpp&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);
    write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);
    write_file(tmp / &quot;x/c.cpp&quot;, &quot;C&quot;);
    write_file(tmp / &quot;x/y/z.cpp&quot;, &quot;Z&quot;);

    auto out = expand_glob((tmp / &quot;**/*.cpp&quot;).generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;a.cpp&quot;, &quot;x/c.cpp&quot;, &quot;x/y/z.cpp&quot;});
}

TEST_CASE(&quot;glob: foo/*/bar/**/*.txt&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;foo/K/bar/a.txt&quot;, &quot;A&quot;);
    write_file(tmp / &quot;foo/K/bar/x/b.txt&quot;, &quot;B&quot;);
    write_file(tmp / &quot;foo/X/bar/c.bin&quot;, &quot;C&quot;);
    write_file(tmp / &quot;foo/Z/bar/y/z.txt&quot;, &quot;Z&quot;);

    auto pat = (tmp / &quot;foo/*/bar/**/*.txt&quot;).generic_string();
    auto out = expand_glob(pat);
    auto rel = to_rel(out, tmp);

    check_paths(rel,
                {&quot;foo/K/bar/a.txt&quot;, &quot;foo/K/bar/x/b.txt&quot;, &quot;foo/Z/bar/y/z.txt&quot;});
}

TEST_CASE(&quot;glob: no matches returns empty&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);

    auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());
    CHECK(out.empty());
}

TEST_CASE(&quot;glob: duplicates removed&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_f&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);
    write_file(tmp / &quot;d/a.txt&quot;, &quot;A2&quot;);

    // отработает и * и **
    auto pat1 = (tmp / &quot;*.txt&quot;).generic_string();
    auto pat2 = (tmp / &quot;**/*.txt&quot;).generic_string();

    auto out1 = expand_glob(pat1);
    auto out2 = expand_glob(pat2);

    std::vector&lt;fs::path&gt; combined;
    combined.insert(combined.end(), out1.begin(), out1.end());
    combined.insert(combined.end(), out2.begin(), out2.end());

    // удаляем дубликаты вручную, как collector делает
    std::sort(combined.begin(), combined.end());
    combined.erase(std::unique(combined.begin(), combined.end()),
                   combined.end());

    auto rel = to_rel(combined, tmp);
    check_paths(rel, {&quot;a.txt&quot;, &quot;d/a.txt&quot;});
}

</code></pre>
</body>
</html>

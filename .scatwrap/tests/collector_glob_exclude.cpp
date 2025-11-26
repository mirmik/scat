<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/collector_glob_exclude.cpp</title>
</head>
<body>
<pre><code>
#include &quot;collector.h&quot;
#include &quot;doctest/doctest.h&quot;
#include &quot;glob.h&quot;
#include &quot;options.h&quot;
#include &quot;rules.h&quot;

#include &lt;algorithm&gt;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;vector&gt;

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

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
    std::vector&lt;std::string&gt; exp_vec;
    exp_vec.reserve(expected.size());
    for (auto *e : expected)
        exp_vec.emplace_back(e);

    std::vector&lt;std::string&gt; act = actual;

    std::sort(exp_vec.begin(), exp_vec.end());
    std::sort(act.begin(), act.end());

    CHECK(act.size() == exp_vec.size());

    if (act.size() != exp_vec.size())
        return; // предотвратить каскад ошибок

    for (size_t i = 0; i &lt; act.size(); ++i)
        CHECK(act[i] == exp_vec[i]);
}

// ============================================================================
// glob tests — как раньше, без смены current_path
// ============================================================================

TEST_CASE(&quot;glob: basic *&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_a2&quot;;
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_b2&quot;;
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_c2&quot;;
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
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_d2&quot;;
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

TEST_CASE(&quot;glob: no matches&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;glob_test_e2&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;file.cpp&quot;, &quot;A&quot;);

    auto out = expand_glob((tmp / &quot;**/*.txt&quot;).generic_string());
    CHECK(out.empty());
}

// ============================================================================
// collector tests — тоже на абсолютных путях
// ============================================================================

TEST_CASE(&quot;collector: include then exclude subdir&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_1&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);
    write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);
    write_file(tmp / &quot;src/tests/c.cpp&quot;, &quot;C&quot;);

    std::vector&lt;Rule&gt; rules = {{(tmp / &quot;src/**/*.cpp&quot;).generic_string(), false},
                               {(tmp / &quot;src/tests/**&quot;).generic_string(), true}};

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;src/a.cpp&quot;, &quot;src/b.cpp&quot;});
}

TEST_CASE(&quot;collector: exclude *.cpp top level&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_2&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);
    write_file(tmp / &quot;b.h&quot;, &quot;B&quot;);
    write_file(tmp / &quot;sub/c.cpp&quot;, &quot;C&quot;);

    std::vector&lt;Rule&gt; rules = {
        {(tmp / &quot;**&quot;).generic_string(), false},
        {(tmp / &quot;*.cpp&quot;).generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;b.h&quot;, &quot;sub/c.cpp&quot;});
}

TEST_CASE(&quot;collector: exclude all cpp recursively&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_3&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.cpp&quot;, &quot;A&quot;);
    write_file(tmp / &quot;x/b.cpp&quot;, &quot;B&quot;);
    write_file(tmp / &quot;x/y/c.cpp&quot;, &quot;C&quot;);
    write_file(tmp / &quot;ok.txt&quot;, &quot;D&quot;);

    std::vector&lt;Rule&gt; rules = {
        {(tmp / &quot;**&quot;).generic_string(), false},
        {(tmp / &quot;**/*.cpp&quot;).generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;ok.txt&quot;});
}

TEST_CASE(&quot;collector: include-exclude-include chain&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_4&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;data/keep/a.txt&quot;, &quot;A&quot;);
    write_file(tmp / &quot;data/keep/b.cpp&quot;, &quot;B&quot;);
    write_file(tmp / &quot;data/drop/c.txt&quot;, &quot;C&quot;);

    std::vector&lt;Rule&gt; rules = {
        {(tmp / &quot;data/**&quot;).generic_string(), false},
        {(tmp / &quot;data/drop/**&quot;).generic_string(), true},
        {(tmp / &quot;data/keep/*.txt&quot;).generic_string(), false},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {&quot;data/keep/a.txt&quot;, &quot;data/keep/b.cpp&quot;});
}

TEST_CASE(&quot;collector: empty after exclude&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_test_5&quot;;
    fs::remove_all(tmp);

    write_file(tmp / &quot;a.txt&quot;, &quot;A&quot;);

    std::vector&lt;Rule&gt; rules = {
        {(tmp / &quot;*.txt&quot;).generic_string(), false},
        {(tmp / &quot;*.txt&quot;).generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);

    CHECK(out.empty());
}

TEST_CASE(&quot;collector: complex structure with 5 rules&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;collector_complex_test&quot;;
    fs::remove_all(tmp);

    // ----------------------------
    // создаём структуру на 15 файлов
    // ----------------------------
    write_file(tmp / &quot;root.txt&quot;, &quot;R&quot;);
    write_file(tmp / &quot;root.bin&quot;, &quot;RB&quot;);
    write_file(tmp / &quot;readme.md&quot;, &quot;MD&quot;);

    write_file(tmp / &quot;src/a.cpp&quot;, &quot;A&quot;);
    write_file(tmp / &quot;src/b.cpp&quot;, &quot;B&quot;);
    write_file(tmp / &quot;src/c.log&quot;, &quot;CLOG&quot;);

    write_file(tmp / &quot;src/tools/t1.txt&quot;, &quot;T1&quot;);
    write_file(tmp / &quot;src/tools/t2.txt&quot;, &quot;T2&quot;);
    write_file(tmp / &quot;src/tools/helper.cpp&quot;, &quot;HC&quot;);

    write_file(tmp / &quot;data/info.txt&quot;, &quot;INFO&quot;);
    write_file(tmp / &quot;data/raw/raw1.bin&quot;, &quot;BIN1&quot;);
    write_file(tmp / &quot;data/raw/raw2.bin&quot;, &quot;BIN2&quot;);

    write_file(tmp / &quot;build/tmp1.log&quot;, &quot;TMP1&quot;);
    write_file(tmp / &quot;build/tmp2.log&quot;, &quot;TMP2&quot;);

    write_file(tmp / &quot;misc/x.cpp&quot;, &quot;X&quot;);
    write_file(tmp / &quot;misc/y.txt&quot;, &quot;Y&quot;);

    // ----------------------------
    // ПРАВИЛА
    // ----------------------------

    std::vector&lt;Rule&gt; rules = {
        // 1) собрать всё
        {(tmp / &quot;**&quot;).generic_string(), false},

        // 2) исключить data/raw/**
        {(tmp / &quot;data/raw/**&quot;).generic_string(), true},

        // 3) исключить все *.log
        {(tmp / &quot;**/*.log&quot;).generic_string(), true},

        // 4) добавить текстовые файлы только в tools
        {(tmp / &quot;src/tools/*.txt&quot;).generic_string(), false},

        // 5) исключить бинарники только в корне (root.bin)
        {(tmp / &quot;*.bin&quot;).generic_string(), true}};

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    // ----------------------------
    // ОЖИДАЕМЫЙ РЕЗУЛЬТАТ
    // ----------------------------
    //
    // Файлы, которые должны остаться:
    //
    //   root.txt
    //   readme.md
    //   src/a.cpp
    //   src/b.cpp
    //   src/tools/t1.txt
    //   src/tools/t2.txt
    //   src/tools/helper.cpp
    //   data/info.txt
    //   misc/x.cpp
    //   misc/y.txt
    //
    // НЕ должно быть:
    //   data/raw/*        (правило #2)
    //   *.log             (правило #3)
    //   root.bin          (правило #5)
    //
    check_paths(rel,
                {
                    &quot;root.txt&quot;,
                    &quot;readme.md&quot;,
                    &quot;src/a.cpp&quot;,
                    &quot;src/b.cpp&quot;,
                    &quot;src/tools/t1.txt&quot;,
                    &quot;src/tools/t2.txt&quot;,
                    &quot;src/tools/helper.cpp&quot;,
                    &quot;data/info.txt&quot;,
                    &quot;misc/x.cpp&quot;,
                    &quot;misc/y.txt&quot;,
                });
}

</code></pre>
</body>
</html>

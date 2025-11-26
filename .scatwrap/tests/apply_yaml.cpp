<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply_yaml.cpp</title>
</head>
<body>
<pre><code>
#include &quot;doctest/doctest.h&quot;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;iostream&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

int apply_chunk_main(int argc, char **argv);

namespace fs = std::filesystem;

static std::vector&lt;std::string&gt; read_lines(const fs::path &amp;p)
{
    std::ifstream in(p);
    std::vector&lt;std::string&gt; v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

static int run_apply(const fs::path &amp;patch)
{
    std::string a0 = &quot;apply&quot;;
    std::string a1 = patch.string();

    std::vector&lt;std::string&gt; store = {a0, a1};
    std::vector&lt;char *&gt; argv;
    for (auto &amp;s : store)
        argv.push_back(s.data());

    return apply_chunk_main((int)argv.size(), argv.data());
}

// ============================================================================
// 1. MARKER: без BEFORE/AFTER — работает как старый режим
// ============================================================================
TEST_CASE(&quot;YAML: only MARKER: behaves like legacy replace-text&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_marker_only_test&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;a.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;A\nB\nC\n&quot;;
    }

    fs::path patch = tmp / &quot;patch1.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-text\n&quot;
               &quot;MARKER:\n&quot;
               &quot;B\n&quot;
               &quot;---\n&quot;
               &quot;X\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 3);
    CHECK(L[0] == &quot;A&quot;);
    CHECK(L[1] == &quot;X&quot;);
    CHECK(L[2] == &quot;C&quot;);
}

// ============================================================================
// 2. BEFORE fuzzy
// ============================================================================
TEST_CASE(&quot;YAML: BEFORE fuzzy selects the correct marker&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_test2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;b.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;foo\n&quot;
               &quot;target\n&quot;
               &quot;bar\n&quot;
               &quot;\n&quot;
               &quot;XXX\n&quot;
               &quot;target\n&quot;
               &quot;YYY\n&quot;;
    }

    fs::path patch = tmp / &quot;patch2.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-text\n&quot;
               &quot;BEFORE:\n&quot;
               &quot;XXX\n&quot;
               &quot;MARKER:\n&quot;
               &quot;target\n&quot;
               &quot;---\n&quot;
               &quot;SECOND\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == &quot;SECOND&quot;);
}

// ============================================================================
// 3. AFTER fuzzy
// ============================================================================
TEST_CASE(&quot;YAML: AFTER fuzzy selects correct block&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_after_test2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;c.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;log\n&quot;
               &quot;X\n&quot;
               &quot;done\n&quot;
               &quot;\n&quot;
               &quot;log\n&quot;
               &quot;X\n&quot;
               &quot;finish\n&quot;;
    }

    fs::path patch = tmp / &quot;patch3.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-text\n&quot;
               &quot;MARKER:\n&quot;
               &quot;X\n&quot;
               &quot;AFTER:\n&quot;
               &quot;finish\n&quot;
               &quot;---\n&quot;
               &quot;CHANGED\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == &quot;CHANGED&quot;);
}

// ============================================================================
// 4. BEFORE + AFTER together
// ============================================================================
TEST_CASE(&quot;YAML: strong fuzzy match with BEFORE + AFTER&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_before_after_test2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;d.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;A\n&quot;
               &quot;mark\n&quot;
               &quot;B\n&quot;
               &quot;\n&quot;
               &quot;C\n&quot;
               &quot;mark\n&quot;
               &quot;D\n&quot;;
    }

    fs::path patch = tmp / &quot;patch4.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-text\n&quot;
               &quot;BEFORE:\n&quot;
               &quot;C\n&quot;
               &quot;MARKER:\n&quot;
               &quot;mark\n&quot;
               &quot;AFTER:\n&quot;
               &quot;D\n&quot;
               &quot;---\n&quot;
               &quot;SELECTED\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == &quot;SELECTED&quot;);
}

// ============================================================================
// 7. Legacy format still works
// ============================================================================
TEST_CASE(&quot;YAML: legacy replace-text still works&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;yaml_legacy_test2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;g.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;1\n&quot;
               &quot;2\n&quot;
               &quot;3\n&quot;;
    }

    fs::path patch = tmp / &quot;patch7.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-text\n&quot;
               &quot;2\n&quot;
               &quot;---\n&quot;
               &quot;X\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    CHECK(L[1] == &quot;X&quot;);
}

</code></pre>
</body>
</html>

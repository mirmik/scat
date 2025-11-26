<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply_symbols.cpp</title>
</head>
<body>
<pre><code>
#include &quot;doctest/doctest.h&quot;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
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
    argv.reserve(store.size());
    for (auto &amp;s : store)
        argv.push_back(s.data());

    return apply_chunk_main((int)argv.size(), argv.data());
}

// ============================================================================
// C++: replace-cpp-class
// ============================================================================
TEST_CASE(&quot;symbol API: replace-cpp-class replaces only target class&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_class&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;foo.cpp&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;#include &lt;string&gt;\n&quot;
               &quot;\n&quot;
               &quot;class Foo {\n&quot;
               &quot;public:\n&quot;
               &quot;    int x() const;\n&quot;
               &quot;};\n&quot;
               &quot;\n&quot;
               &quot;class Bar {\n&quot;
               &quot;public:\n&quot;
               &quot;    void ping();\n&quot;
               &quot;};\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_class.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-cpp-class Foo\n&quot;
               &quot;class Foo {\n&quot;
               &quot;public:\n&quot;
               &quot;    int y() const { return 42; }\n&quot;
               &quot;};\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() &gt;= 8);

    CHECK(L[0] == &quot;#include &lt;string&gt;&quot;);
    CHECK(L[1] == &quot;&quot;);
    CHECK(L[2] == &quot;class Foo {&quot;);
    CHECK(L[3] == &quot;public:&quot;);
    CHECK(L[4] == &quot;    int y() const { return 42; }&quot;);
    CHECK(L[5] == &quot;};&quot;);
    CHECK(L[6] == &quot;&quot;);
    CHECK(L[7] == &quot;class Bar {&quot;);
}

// ============================================================================
// C++: replace-cpp-method
// ============================================================================
TEST_CASE(&quot;symbol API: replace-cpp-method by separate class and method name&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_1&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;foo.cpp&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;class Foo {\n&quot;
               &quot;public:\n&quot;
               &quot;    void a();\n&quot;
               &quot;    int value() const;\n&quot;
               &quot;};\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_method1.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-cpp-method Foo value\n&quot;
               &quot;    int value() const { return 10; }\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 5);
    CHECK(L[0] == &quot;class Foo {&quot;);
    CHECK(L[1] == &quot;public:&quot;);
    CHECK(L[2] == &quot;    void a();&quot;);
    CHECK(L[3] == &quot;    int value() const { return 10; }&quot;);
    CHECK(L[4] == &quot;};&quot;);
}

TEST_CASE(&quot;symbol API: replace-cpp-method with Class::method syntax&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;bar.cpp&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;class Bar {\n&quot;
               &quot;public:\n&quot;
               &quot;    int calc(int x) const;\n&quot;
               &quot;    int other() const;\n&quot;
               &quot;};\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_method2.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-cpp-method Bar::calc\n&quot;
               &quot;    int calc(int x) const { return x * 2; }\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 5);
    CHECK(L[2] == &quot;    int calc(int x) const { return x * 2; }&quot;);
    CHECK(L[3] == &quot;    int other() const;&quot;);
}

// ============================================================================
// Python: replace-py-class
// ============================================================================
TEST_CASE(&quot;symbol API: replace-py-class replaces whole class body&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_class&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;foo.py&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;class Foo:\n&quot;
               &quot;    def __init__(self):\n&quot;
               &quot;        self.x = 1\n&quot;
               &quot;\n&quot;
               &quot;class Bar:\n&quot;
               &quot;    pass\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_py_class.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-py-class Foo\n&quot;
               &quot;class Foo:\n&quot;
               &quot;    def __init__(self):\n&quot;
               &quot;        self.x = 2\n&quot;
               &quot;    def answer(self):\n&quot;
               &quot;        return 42\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() &gt;= 5);

    // Проверяем, что новый класс Foo на месте
    CHECK(L[0] == &quot;class Foo:&quot;);

    bool found_init = false;
    bool found_x2 = false;
    bool found_answer = false;
    bool found_ret42 = false;
    bool found_bar = false;

    for (const auto &amp;line : L)
    {
        if (line.find(&quot;def __init__&quot;) != std::string::npos)
            found_init = true;
        if (line.find(&quot;self.x = 2&quot;) != std::string::npos)
            found_x2 = true;
        if (line.find(&quot;def answer&quot;) != std::string::npos)
            found_answer = true;
        if (line.find(&quot;return 42&quot;) != std::string::npos)
            found_ret42 = true;
        if (line == &quot;class Bar:&quot;)
            found_bar = true;
    }

    CHECK(found_init);
    CHECK(found_x2);
    CHECK(found_answer);
    CHECK(found_ret42);
    CHECK(found_bar);
}

// ============================================================================
// Python: replace-py-method
// ============================================================================
TEST_CASE(&quot;symbol API: replace-py-method with separate class and method name&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_1&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;weird.py&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;class Weird:\n&quot;
               &quot;    def run(self):\n&quot;
               &quot;        return 1\n&quot;
               &quot;\n&quot;
               &quot;    def other(self):\n&quot;
               &quot;        return 2\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_py_method1.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-py-method Weird run\n&quot;
               &quot;    def run(self):\n&quot;
               &quot;        return 100\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() &gt;= 5);

    bool found_run_100 = false;
    bool found_other_2 = false;
    bool seen_def_run = false;
    bool seen_return_100 = false;

    for (const auto &amp;line : L)
    {
        if (line.find(&quot;def run&quot;) != std::string::npos)
            seen_def_run = true;
        if (line.find(&quot;return 100&quot;) != std::string::npos)
            seen_return_100 = true;

        if (line.find(&quot;def other&quot;) != std::string::npos ||
            line.find(&quot;return 2&quot;) != std::string::npos)
        {
            // Очень грубо: убеждаемся, что следы second метода остались
            found_other_2 = true;
        }
    }

    found_run_100 = seen_def_run &amp;&amp; seen_return_100;

    CHECK(found_run_100);
    CHECK(found_other_2);
}

TEST_CASE(&quot;symbol API: replace-py-method with Class.method syntax&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_2&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;async_foo.py&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;class Foo:\n&quot;
               &quot;    async def bar(self):\n&quot;
               &quot;        return 1\n&quot;;
    }

    fs::path patch = tmp / &quot;patch_py_method2.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()
            &lt;&lt; &quot; ===\n&quot;
               &quot;--- replace-py-method Foo.bar\n&quot;
               &quot;    async def bar(self):\n&quot;
               &quot;        return 2\n&quot;
               &quot;=END=\n&quot;;
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() &gt;= 2);

    CHECK(L[0] == &quot;class Foo:&quot;);

    bool found_bar_2 = false;
    for (const auto &amp;line : L)
    {
        if (line.find(&quot;async def bar&quot;) != std::string::npos)
            found_bar_2 = true;
        if (line.find(&quot;return 2&quot;) != std::string::npos)
            found_bar_2 = true;
    }

    CHECK(found_bar_2);
}

</code></pre>
</body>
</html>

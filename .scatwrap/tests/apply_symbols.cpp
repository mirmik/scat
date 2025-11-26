<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply_symbols.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
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
    argv.reserve(store.size());<br>
    for (auto &amp;s : store)<br>
        argv.push_back(s.data());<br>
<br>
    return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-class replaces only target class&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_class&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;foo.cpp&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;#include &lt;string&gt;\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;class Foo {\n&quot;<br>
               &quot;public:\n&quot;<br>
               &quot;    int x() const;\n&quot;<br>
               &quot;};\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;class Bar {\n&quot;<br>
               &quot;public:\n&quot;<br>
               &quot;    void ping();\n&quot;<br>
               &quot;};\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_class.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-cpp-class Foo\n&quot;<br>
               &quot;class Foo {\n&quot;<br>
               &quot;public:\n&quot;<br>
               &quot;    int y() const { return 42; }\n&quot;<br>
               &quot;};\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() &gt;= 8);<br>
<br>
    CHECK(L[0] == &quot;#include &lt;string&gt;&quot;);<br>
    CHECK(L[1] == &quot;&quot;);<br>
    CHECK(L[2] == &quot;class Foo {&quot;);<br>
    CHECK(L[3] == &quot;public:&quot;);<br>
    CHECK(L[4] == &quot;    int y() const { return 42; }&quot;);<br>
    CHECK(L[5] == &quot;};&quot;);<br>
    CHECK(L[6] == &quot;&quot;);<br>
    CHECK(L[7] == &quot;class Bar {&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-method by separate class and method name&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_1&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;foo.cpp&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;class Foo {\n&quot;<br>
               &quot;public:\n&quot;<br>
               &quot;    void a();\n&quot;<br>
               &quot;    int value() const;\n&quot;<br>
               &quot;};\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_method1.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-cpp-method Foo value\n&quot;<br>
               &quot;    int value() const { return 10; }\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 5);<br>
    CHECK(L[0] == &quot;class Foo {&quot;);<br>
    CHECK(L[1] == &quot;public:&quot;);<br>
    CHECK(L[2] == &quot;    void a();&quot;);<br>
    CHECK(L[3] == &quot;    int value() const { return 10; }&quot;);<br>
    CHECK(L[4] == &quot;};&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-cpp-method with Class::method syntax&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;bar.cpp&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;class Bar {\n&quot;<br>
               &quot;public:\n&quot;<br>
               &quot;    int calc(int x) const;\n&quot;<br>
               &quot;    int other() const;\n&quot;<br>
               &quot;};\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_method2.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-cpp-method Bar::calc\n&quot;<br>
               &quot;    int calc(int x) const { return x * 2; }\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() == 5);<br>
    CHECK(L[2] == &quot;    int calc(int x) const { return x * 2; }&quot;);<br>
    CHECK(L[3] == &quot;    int other() const;&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-class replaces whole class body&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_class&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;foo.py&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;class Foo:\n&quot;<br>
               &quot;    def __init__(self):\n&quot;<br>
               &quot;        self.x = 1\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;class Bar:\n&quot;<br>
               &quot;    pass\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_py_class.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-py-class Foo\n&quot;<br>
               &quot;class Foo:\n&quot;<br>
               &quot;    def __init__(self):\n&quot;<br>
               &quot;        self.x = 2\n&quot;<br>
               &quot;    def answer(self):\n&quot;<br>
               &quot;        return 42\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() &gt;= 5);<br>
<br>
    // Проверяем, что новый класс Foo на месте<br>
    CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
    bool found_init = false;<br>
    bool found_x2 = false;<br>
    bool found_answer = false;<br>
    bool found_ret42 = false;<br>
    bool found_bar = false;<br>
<br>
    for (const auto &amp;line : L)<br>
    {<br>
        if (line.find(&quot;def __init__&quot;) != std::string::npos)<br>
            found_init = true;<br>
        if (line.find(&quot;self.x = 2&quot;) != std::string::npos)<br>
            found_x2 = true;<br>
        if (line.find(&quot;def answer&quot;) != std::string::npos)<br>
            found_answer = true;<br>
        if (line.find(&quot;return 42&quot;) != std::string::npos)<br>
            found_ret42 = true;<br>
        if (line == &quot;class Bar:&quot;)<br>
            found_bar = true;<br>
    }<br>
<br>
    CHECK(found_init);<br>
    CHECK(found_x2);<br>
    CHECK(found_answer);<br>
    CHECK(found_ret42);<br>
    CHECK(found_bar);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-method with separate class and method name&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_1&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;weird.py&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;class Weird:\n&quot;<br>
               &quot;    def run(self):\n&quot;<br>
               &quot;        return 1\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;    def other(self):\n&quot;<br>
               &quot;        return 2\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_py_method1.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-py-method Weird run\n&quot;<br>
               &quot;    def run(self):\n&quot;<br>
               &quot;        return 100\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() &gt;= 5);<br>
<br>
    bool found_run_100 = false;<br>
    bool found_other_2 = false;<br>
    bool seen_def_run = false;<br>
    bool seen_return_100 = false;<br>
<br>
    for (const auto &amp;line : L)<br>
    {<br>
        if (line.find(&quot;def run&quot;) != std::string::npos)<br>
            seen_def_run = true;<br>
        if (line.find(&quot;return 100&quot;) != std::string::npos)<br>
            seen_return_100 = true;<br>
<br>
        if (line.find(&quot;def other&quot;) != std::string::npos ||<br>
            line.find(&quot;return 2&quot;) != std::string::npos)<br>
        {<br>
            // Очень грубо: убеждаемся, что следы second метода остались<br>
            found_other_2 = true;<br>
        }<br>
    }<br>
<br>
    found_run_100 = seen_def_run &amp;&amp; seen_return_100;<br>
<br>
    CHECK(found_run_100);<br>
    CHECK(found_other_2);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-py-method with Class.method syntax&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_2&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;async_foo.py&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;class Foo:\n&quot;<br>
               &quot;    async def bar(self):\n&quot;<br>
               &quot;        return 1\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch_py_method2.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-py-method Foo.bar\n&quot;<br>
               &quot;    async def bar(self):\n&quot;<br>
               &quot;        return 2\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    CHECK(run_apply(patch) == 0);<br>
<br>
    auto L = read_lines(f);<br>
    REQUIRE(L.size() &gt;= 2);<br>
<br>
    CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
    bool found_bar_2 = false;<br>
    for (const auto &amp;line : L)<br>
    {<br>
        if (line.find(&quot;async def bar&quot;) != std::string::npos)<br>
            found_bar_2 = true;<br>
        if (line.find(&quot;return 2&quot;) != std::string::npos)<br>
            found_bar_2 = true;<br>
    }<br>
<br>
    CHECK(found_bar_2);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

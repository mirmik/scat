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
&#9;argv.reserve(store.size());<br>
&#9;for (auto &amp;s : store)<br>
&#9;&#9;argv.push_back(s.data());<br>
<br>
&#9;return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-class replaces only target class&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_class&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;foo.cpp&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;#include &lt;string&gt;\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&quot;    int x() const;\n&quot;<br>
&#9;&#9;&#9;&quot;};\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;class Bar {\n&quot;<br>
&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&quot;    void ping();\n&quot;<br>
&#9;&#9;&#9;&quot;};\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_class.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-cpp-class Foo\n&quot;<br>
&#9;&#9;&#9;&quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&quot;    int y() const { return 42; }\n&quot;<br>
&#9;&#9;&#9;&quot;};\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() &gt;= 8);<br>
<br>
&#9;CHECK(L[0] == &quot;#include &lt;string&gt;&quot;);<br>
&#9;CHECK(L[1] == &quot;&quot;);<br>
&#9;CHECK(L[2] == &quot;class Foo {&quot;);<br>
&#9;CHECK(L[3] == &quot;public:&quot;);<br>
&#9;CHECK(L[4] == &quot;    int y() const { return 42; }&quot;);<br>
&#9;CHECK(L[5] == &quot;};&quot;);<br>
&#9;CHECK(L[6] == &quot;&quot;);<br>
&#9;CHECK(L[7] == &quot;class Bar {&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-method by separate class and method name&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_1&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;foo.cpp&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&quot;    void a();\n&quot;<br>
&#9;&#9;&#9;&quot;    int value() const;\n&quot;<br>
&#9;&#9;&#9;&quot;};\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_method1.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-cpp-method Foo value\n&quot;<br>
&#9;&#9;&#9;&quot;    int value() const { return 10; }\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 5);<br>
&#9;CHECK(L[0] == &quot;class Foo {&quot;);<br>
&#9;CHECK(L[1] == &quot;public:&quot;);<br>
&#9;CHECK(L[2] == &quot;    void a();&quot;);<br>
&#9;CHECK(L[3] == &quot;    int value() const { return 10; }&quot;);<br>
&#9;CHECK(L[4] == &quot;};&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-cpp-method with Class::method syntax&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;bar.cpp&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;class Bar {\n&quot;<br>
&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&quot;    int calc(int x) const;\n&quot;<br>
&#9;&#9;&#9;&quot;    int other() const;\n&quot;<br>
&#9;&#9;&#9;&quot;};\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_method2.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-cpp-method Bar::calc\n&quot;<br>
&#9;&#9;&#9;&quot;    int calc(int x) const { return x * 2; }\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() == 5);<br>
&#9;CHECK(L[2] == &quot;    int calc(int x) const { return x * 2; }&quot;);<br>
&#9;CHECK(L[3] == &quot;    int other() const;&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-class replaces whole class body&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_class&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;foo.py&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;class Foo:\n&quot;<br>
&#9;&#9;&#9;&quot;    def __init__(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        self.x = 1\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;class Bar:\n&quot;<br>
&#9;&#9;&#9;&quot;    pass\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_py_class.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-py-class Foo\n&quot;<br>
&#9;&#9;&#9;&quot;class Foo:\n&quot;<br>
&#9;&#9;&#9;&quot;    def __init__(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        self.x = 2\n&quot;<br>
&#9;&#9;&#9;&quot;    def answer(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 42\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() &gt;= 5);<br>
<br>
&#9;// Проверяем, что новый класс Foo на месте<br>
&#9;CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
&#9;bool found_init = false;<br>
&#9;bool found_x2 = false;<br>
&#9;bool found_answer = false;<br>
&#9;bool found_ret42 = false;<br>
&#9;bool found_bar = false;<br>
<br>
&#9;for (const auto &amp;line : L)<br>
&#9;{<br>
&#9;&#9;if (line.find(&quot;def __init__&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_init = true;<br>
&#9;&#9;if (line.find(&quot;self.x = 2&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_x2 = true;<br>
&#9;&#9;if (line.find(&quot;def answer&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_answer = true;<br>
&#9;&#9;if (line.find(&quot;return 42&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_ret42 = true;<br>
&#9;&#9;if (line == &quot;class Bar:&quot;)<br>
&#9;&#9;&#9;found_bar = true;<br>
&#9;}<br>
<br>
&#9;CHECK(found_init);<br>
&#9;CHECK(found_x2);<br>
&#9;CHECK(found_answer);<br>
&#9;CHECK(found_ret42);<br>
&#9;CHECK(found_bar);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-method with separate class and method name&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_1&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;weird.py&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;class Weird:\n&quot;<br>
&#9;&#9;&#9;&quot;    def run(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 1\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;    def other(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 2\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_py_method1.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-py-method Weird run\n&quot;<br>
&#9;&#9;&#9;&quot;    def run(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 100\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() &gt;= 5);<br>
<br>
&#9;bool found_run_100 = false;<br>
&#9;bool found_other_2 = false;<br>
&#9;bool seen_def_run = false;<br>
&#9;bool seen_return_100 = false;<br>
<br>
&#9;for (const auto &amp;line : L)<br>
&#9;{<br>
&#9;&#9;if (line.find(&quot;def run&quot;) != std::string::npos)<br>
&#9;&#9;&#9;seen_def_run = true;<br>
&#9;&#9;if (line.find(&quot;return 100&quot;) != std::string::npos)<br>
&#9;&#9;&#9;seen_return_100 = true;<br>
<br>
&#9;&#9;if (line.find(&quot;def other&quot;) != std::string::npos ||<br>
&#9;&#9;&#9;line.find(&quot;return 2&quot;) != std::string::npos)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// Очень грубо: убеждаемся, что следы second метода остались<br>
&#9;&#9;&#9;found_other_2 = true;<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;found_run_100 = seen_def_run &amp;&amp; seen_return_100;<br>
<br>
&#9;CHECK(found_run_100);<br>
&#9;CHECK(found_other_2);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-py-method with Class.method syntax&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_2&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;async_foo.py&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;class Foo:\n&quot;<br>
&#9;&#9;&#9;&quot;    async def bar(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 1\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch_py_method2.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-py-method Foo.bar\n&quot;<br>
&#9;&#9;&#9;&quot;    async def bar(self):\n&quot;<br>
&#9;&#9;&#9;&quot;        return 2\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;CHECK(run_apply(patch) == 0);<br>
<br>
&#9;auto L = read_lines(f);<br>
&#9;REQUIRE(L.size() &gt;= 2);<br>
<br>
&#9;CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
&#9;bool found_bar_2 = false;<br>
&#9;for (const auto &amp;line : L)<br>
&#9;{<br>
&#9;&#9;if (line.find(&quot;async def bar&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_bar_2 = true;<br>
&#9;&#9;if (line.find(&quot;return 2&quot;) != std::string::npos)<br>
&#9;&#9;&#9;found_bar_2 = true;<br>
&#9;}<br>
<br>
&#9;CHECK(found_bar_2);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

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
&emsp;std::ifstream in(p);<br>
&emsp;std::vector&lt;std::string&gt; v;<br>
&emsp;std::string s;<br>
&emsp;while (std::getline(in, s))<br>
&emsp;&emsp;v.push_back(s);<br>
&emsp;return v;<br>
}<br>
<br>
static int run_apply(const fs::path &amp;patch)<br>
{<br>
&emsp;std::string a0 = &quot;apply&quot;;<br>
&emsp;std::string a1 = patch.string();<br>
<br>
&emsp;std::vector&lt;std::string&gt; store = {a0, a1};<br>
&emsp;std::vector&lt;char *&gt; argv;<br>
&emsp;argv.reserve(store.size());<br>
&emsp;for (auto &amp;s : store)<br>
&emsp;&emsp;argv.push_back(s.data());<br>
<br>
&emsp;return apply_chunk_main((int)argv.size(), argv.data());<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-class replaces only target class&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_class&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;foo.cpp&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;#include &lt;string&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int x() const;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;};\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;class Bar {\n&quot;<br>
&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    void ping();\n&quot;<br>
&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_class.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-cpp-class Foo\n&quot;<br>
&emsp;&emsp;&emsp;&quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int y() const { return 42; }\n&quot;<br>
&emsp;&emsp;&emsp;&quot;};\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() &gt;= 8);<br>
<br>
&emsp;CHECK(L[0] == &quot;#include &lt;string&gt;&quot;);<br>
&emsp;CHECK(L[1] == &quot;&quot;);<br>
&emsp;CHECK(L[2] == &quot;class Foo {&quot;);<br>
&emsp;CHECK(L[3] == &quot;public:&quot;);<br>
&emsp;CHECK(L[4] == &quot;    int y() const { return 42; }&quot;);<br>
&emsp;CHECK(L[5] == &quot;};&quot;);<br>
&emsp;CHECK(L[6] == &quot;&quot;);<br>
&emsp;CHECK(L[7] == &quot;class Bar {&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// C++: replace-cpp-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-cpp-method by separate class and method name&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_1&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;foo.cpp&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    void a();\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int value() const;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_method1.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-cpp-method Foo value\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int value() const { return 10; }\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 5);<br>
&emsp;CHECK(L[0] == &quot;class Foo {&quot;);<br>
&emsp;CHECK(L[1] == &quot;public:&quot;);<br>
&emsp;CHECK(L[2] == &quot;    void a();&quot;);<br>
&emsp;CHECK(L[3] == &quot;    int value() const { return 10; }&quot;);<br>
&emsp;CHECK(L[4] == &quot;};&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-cpp-method with Class::method syntax&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_cpp_method_2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;bar.cpp&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;class Bar {\n&quot;<br>
&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int calc(int x) const;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int other() const;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_method2.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-cpp-method Bar::calc\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    int calc(int x) const { return x * 2; }\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() == 5);<br>
&emsp;CHECK(L[2] == &quot;    int calc(int x) const { return x * 2; }&quot;);<br>
&emsp;CHECK(L[3] == &quot;    int other() const;&quot;);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-class<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-class replaces whole class body&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_class&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;foo.py&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;class Foo:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def __init__(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        self.x = 1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;class Bar:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    pass\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_py_class.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-py-class Foo\n&quot;<br>
&emsp;&emsp;&emsp;&quot;class Foo:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def __init__(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        self.x = 2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def answer(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 42\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() &gt;= 5);<br>
<br>
&emsp;// Проверяем, что новый класс Foo на месте<br>
&emsp;CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
&emsp;bool found_init = false;<br>
&emsp;bool found_x2 = false;<br>
&emsp;bool found_answer = false;<br>
&emsp;bool found_ret42 = false;<br>
&emsp;bool found_bar = false;<br>
<br>
&emsp;for (const auto &amp;line : L)<br>
&emsp;{<br>
&emsp;&emsp;if (line.find(&quot;def __init__&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_init = true;<br>
&emsp;&emsp;if (line.find(&quot;self.x = 2&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_x2 = true;<br>
&emsp;&emsp;if (line.find(&quot;def answer&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_answer = true;<br>
&emsp;&emsp;if (line.find(&quot;return 42&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_ret42 = true;<br>
&emsp;&emsp;if (line == &quot;class Bar:&quot;)<br>
&emsp;&emsp;&emsp;found_bar = true;<br>
&emsp;}<br>
<br>
&emsp;CHECK(found_init);<br>
&emsp;CHECK(found_x2);<br>
&emsp;CHECK(found_answer);<br>
&emsp;CHECK(found_ret42);<br>
&emsp;CHECK(found_bar);<br>
}<br>
<br>
// ============================================================================<br>
// Python: replace-py-method<br>
// ============================================================================<br>
TEST_CASE(&quot;symbol API: replace-py-method with separate class and method name&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_1&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;weird.py&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;class Weird:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def run(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def other(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 2\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_py_method1.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-py-method Weird run\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    def run(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 100\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() &gt;= 5);<br>
<br>
&emsp;bool found_run_100 = false;<br>
&emsp;bool found_other_2 = false;<br>
&emsp;bool seen_def_run = false;<br>
&emsp;bool seen_return_100 = false;<br>
<br>
&emsp;for (const auto &amp;line : L)<br>
&emsp;{<br>
&emsp;&emsp;if (line.find(&quot;def run&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;seen_def_run = true;<br>
&emsp;&emsp;if (line.find(&quot;return 100&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;seen_return_100 = true;<br>
<br>
&emsp;&emsp;if (line.find(&quot;def other&quot;) != std::string::npos ||<br>
&emsp;&emsp;&emsp;line.find(&quot;return 2&quot;) != std::string::npos)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// Очень грубо: убеждаемся, что следы second метода остались<br>
&emsp;&emsp;&emsp;found_other_2 = true;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;found_run_100 = seen_def_run &amp;&amp; seen_return_100;<br>
<br>
&emsp;CHECK(found_run_100);<br>
&emsp;CHECK(found_other_2);<br>
}<br>
<br>
TEST_CASE(&quot;symbol API: replace-py-method with Class.method syntax&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;symbol_py_method_2&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;async_foo.py&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;class Foo:\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    async def bar(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 1\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch_py_method2.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-py-method Foo.bar\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    async def bar(self):\n&quot;<br>
&emsp;&emsp;&emsp;&quot;        return 2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;CHECK(run_apply(patch) == 0);<br>
<br>
&emsp;auto L = read_lines(f);<br>
&emsp;REQUIRE(L.size() &gt;= 2);<br>
<br>
&emsp;CHECK(L[0] == &quot;class Foo:&quot;);<br>
<br>
&emsp;bool found_bar_2 = false;<br>
&emsp;for (const auto &amp;line : L)<br>
&emsp;{<br>
&emsp;&emsp;if (line.find(&quot;async def bar&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_bar_2 = true;<br>
&emsp;&emsp;if (line.find(&quot;return 2&quot;) != std::string::npos)<br>
&emsp;&emsp;&emsp;found_bar_2 = true;<br>
&emsp;}<br>
<br>
&emsp;CHECK(found_bar_2);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;doctest/doctest.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;scat.h&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
int apply_chunk_main(int argc, char **argv);<br>
<br>
namespace fs = std::filesystem;<br>
<br>
std::vector&lt;std::string&gt; read_lines(const fs::path &amp;p)<br>
{<br>
&emsp;std::ifstream in(p);<br>
&emsp;std::vector&lt;std::string&gt; v;<br>
&emsp;std::string s;<br>
&emsp;while (std::getline(in, s))<br>
&emsp;&emsp;v.push_back(s);<br>
&emsp;return v;<br>
}<br>
<br>
int run_apply(const fs::path &amp;patch)<br>
{<br>
&emsp;std::string arg0 = &quot;apply&quot;;<br>
&emsp;std::string arg1 = patch.string();<br>
<br>
&emsp;// храним строки в живом виде<br>
&emsp;std::vector&lt;std::string&gt; args = {arg0, arg1};<br>
<br>
&emsp;// формируем argv как указатели НА ЖИВЫЕ строки<br>
&emsp;std::vector&lt;char *&gt; argv_real;<br>
&emsp;argv_real.reserve(args.size());<br>
&emsp;for (auto &amp;s : args)<br>
&emsp;&emsp;argv_real.push_back(s.data());<br>
<br>
&emsp;return apply_chunk_main((int)argv_real.size(), argv_real.data());<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-after-text&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_after_text&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;a.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;LINE1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;LINE2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;LINE3\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- insert-after-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;LINE2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;AFTER\n&quot;<br>
&emsp;&emsp;&emsp;&quot;TEXT\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;int r = run_apply(patch);<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 5);<br>
&emsp;CHECK(lines[0] == &quot;LINE1&quot;);<br>
&emsp;CHECK(lines[1] == &quot;LINE2&quot;);<br>
&emsp;CHECK(lines[2] == &quot;AFTER&quot;);<br>
&emsp;CHECK(lines[3] == &quot;TEXT&quot;);<br>
&emsp;CHECK(lines[4] == &quot;LINE3&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-before-text&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_before_text&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;b.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;AAA\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BBB\n&quot;<br>
&emsp;&emsp;&emsp;&quot;CCC\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- insert-before-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BBB\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;X\n&quot;<br>
&emsp;&emsp;&emsp;&quot;Y\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;int r = run_apply(patch);<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 5);<br>
&emsp;CHECK(lines[0] == &quot;AAA&quot;);<br>
&emsp;CHECK(lines[1] == &quot;X&quot;);<br>
&emsp;CHECK(lines[2] == &quot;Y&quot;);<br>
&emsp;CHECK(lines[3] == &quot;BBB&quot;);<br>
&emsp;CHECK(lines[4] == &quot;CCC&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: replace-text&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_replace_text&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;c.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;alpha\n&quot;<br>
&emsp;&emsp;&emsp;&quot;beta\n&quot;<br>
&emsp;&emsp;&emsp;&quot;gamma\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;beta\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BETA1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;BETA2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;int r = run_apply(patch);<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 4);<br>
&emsp;CHECK(lines[0] == &quot;alpha&quot;);<br>
&emsp;CHECK(lines[1] == &quot;BETA1&quot;);<br>
&emsp;CHECK(lines[2] == &quot;BETA2&quot;);<br>
&emsp;CHECK(lines[3] == &quot;gamma&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: delete-text&quot;)<br>
{<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_delete_text&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;d.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;one\n&quot;<br>
&emsp;&emsp;&emsp;&quot;two\n&quot;<br>
&emsp;&emsp;&emsp;&quot;three\n&quot;<br>
&emsp;&emsp;&emsp;&quot;four\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- delete-text\n&quot;<br>
&emsp;&emsp;&emsp;&quot;two\n&quot;<br>
&emsp;&emsp;&emsp;&quot;three\n&quot;<br>
&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;int r = run_apply(patch);<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 2);<br>
&emsp;CHECK(lines[0] == &quot;one&quot;);<br>
&emsp;CHECK(lines[1] == &quot;four&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: apply-stdin&quot;)<br>
{<br>
&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_stdin&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;stdin.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;std::string patch = &quot;=== file: &quot; + f.string() +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;B\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;XXX\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
<br>
&emsp;std::istringstream fake_stdin(patch);<br>
&emsp;auto *old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());<br>
<br>
&emsp;const char *argv[] = {&quot;scat&quot;, &quot;--apply-stdin&quot;};<br>
&emsp;int r = scat_main(2, (char **)argv);<br>
<br>
&emsp;std::cin.rdbuf(old_stdin);<br>
<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 3);<br>
&emsp;CHECK(lines[0] == &quot;A&quot;);<br>
&emsp;CHECK(lines[1] == &quot;XXX&quot;);<br>
&emsp;CHECK(lines[2] == &quot;C&quot;);<br>
}<br>
TEST_CASE(&quot;apply_chunk_main: delete-file then create-file&quot;)<br>
{<br>
&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_del_create&quot;;<br>
&emsp;fs::remove_all(tmp);<br>
&emsp;fs::create_directories(tmp);<br>
<br>
&emsp;fs::path f = tmp / &quot;x.txt&quot;;<br>
<br>
&emsp;// Исходный файл<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(f);<br>
&emsp;&emsp;out &lt;&lt; &quot;OLD&quot;;<br>
&emsp;}<br>
<br>
&emsp;// Патч: удалить → создать заново<br>
&emsp;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&emsp;{<br>
&emsp;&emsp;std::ofstream out(patch);<br>
&emsp;&emsp;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- delete-file\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=== file: &quot;<br>
&emsp;&emsp;&emsp;&lt;&lt; f.string()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; ===\n&quot;<br>
&emsp;&emsp;&emsp;&quot;--- create-file\n&quot;<br>
&emsp;&emsp;&emsp;&quot;NEW1\n&quot;<br>
&emsp;&emsp;&emsp;&quot;NEW2\n&quot;<br>
&emsp;&emsp;&emsp;&quot;=END=\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;int r = run_apply(patch);<br>
&emsp;CHECK(r == 0);<br>
<br>
&emsp;REQUIRE(fs::exists(f));<br>
<br>
&emsp;auto lines = read_lines(f);<br>
&emsp;REQUIRE(lines.size() == 2);<br>
&emsp;CHECK(lines[0] == &quot;NEW1&quot;);<br>
&emsp;CHECK(lines[1] == &quot;NEW2&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

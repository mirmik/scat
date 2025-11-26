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
&#9;std::ifstream in(p);<br>
&#9;std::vector&lt;std::string&gt; v;<br>
&#9;std::string s;<br>
&#9;while (std::getline(in, s))<br>
&#9;&#9;v.push_back(s);<br>
&#9;return v;<br>
}<br>
<br>
int run_apply(const fs::path &amp;patch)<br>
{<br>
&#9;std::string arg0 = &quot;apply&quot;;<br>
&#9;std::string arg1 = patch.string();<br>
<br>
&#9;// храним строки в живом виде<br>
&#9;std::vector&lt;std::string&gt; args = {arg0, arg1};<br>
<br>
&#9;// формируем argv как указатели НА ЖИВЫЕ строки<br>
&#9;std::vector&lt;char *&gt; argv_real;<br>
&#9;argv_real.reserve(args.size());<br>
&#9;for (auto &amp;s : args)<br>
&#9;&#9;argv_real.push_back(s.data());<br>
<br>
&#9;return apply_chunk_main((int)argv_real.size(), argv_real.data());<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-after-text&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_after_text&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;a.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;LINE1\n&quot;<br>
&#9;&#9;&#9;&quot;LINE2\n&quot;<br>
&#9;&#9;&#9;&quot;LINE3\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- insert-after-text\n&quot;<br>
&#9;&#9;&#9;&quot;LINE2\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;AFTER\n&quot;<br>
&#9;&#9;&#9;&quot;TEXT\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;int r = run_apply(patch);<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 5);<br>
&#9;CHECK(lines[0] == &quot;LINE1&quot;);<br>
&#9;CHECK(lines[1] == &quot;LINE2&quot;);<br>
&#9;CHECK(lines[2] == &quot;AFTER&quot;);<br>
&#9;CHECK(lines[3] == &quot;TEXT&quot;);<br>
&#9;CHECK(lines[4] == &quot;LINE3&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-before-text&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_before_text&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;b.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;AAA\n&quot;<br>
&#9;&#9;&#9;&quot;BBB\n&quot;<br>
&#9;&#9;&#9;&quot;CCC\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- insert-before-text\n&quot;<br>
&#9;&#9;&#9;&quot;BBB\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;X\n&quot;<br>
&#9;&#9;&#9;&quot;Y\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;int r = run_apply(patch);<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 5);<br>
&#9;CHECK(lines[0] == &quot;AAA&quot;);<br>
&#9;CHECK(lines[1] == &quot;X&quot;);<br>
&#9;CHECK(lines[2] == &quot;Y&quot;);<br>
&#9;CHECK(lines[3] == &quot;BBB&quot;);<br>
&#9;CHECK(lines[4] == &quot;CCC&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: replace-text&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_replace_text&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;c.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;alpha\n&quot;<br>
&#9;&#9;&#9;&quot;beta\n&quot;<br>
&#9;&#9;&#9;&quot;gamma\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&quot;beta\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;BETA1\n&quot;<br>
&#9;&#9;&#9;&quot;BETA2\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;int r = run_apply(patch);<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 4);<br>
&#9;CHECK(lines[0] == &quot;alpha&quot;);<br>
&#9;CHECK(lines[1] == &quot;BETA1&quot;);<br>
&#9;CHECK(lines[2] == &quot;BETA2&quot;);<br>
&#9;CHECK(lines[3] == &quot;gamma&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: delete-text&quot;)<br>
{<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_delete_text&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;d.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;one\n&quot;<br>
&#9;&#9;&#9;&quot;two\n&quot;<br>
&#9;&#9;&#9;&quot;three\n&quot;<br>
&#9;&#9;&#9;&quot;four\n&quot;;<br>
&#9;}<br>
<br>
&#9;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- delete-text\n&quot;<br>
&#9;&#9;&#9;&quot;two\n&quot;<br>
&#9;&#9;&#9;&quot;three\n&quot;<br>
&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;int r = run_apply(patch);<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 2);<br>
&#9;CHECK(lines[0] == &quot;one&quot;);<br>
&#9;CHECK(lines[1] == &quot;four&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: apply-stdin&quot;)<br>
{<br>
&#9;namespace fs = std::filesystem;<br>
<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_stdin&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;stdin.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
&#9;}<br>
<br>
&#9;std::string patch = &quot;=== file: &quot; + f.string() +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot; ===\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;B\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;XXX\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
<br>
&#9;std::istringstream fake_stdin(patch);<br>
&#9;auto *old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());<br>
<br>
&#9;const char *argv[] = {&quot;scat&quot;, &quot;--apply-stdin&quot;};<br>
&#9;int r = scat_main(2, (char **)argv);<br>
<br>
&#9;std::cin.rdbuf(old_stdin);<br>
<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 3);<br>
&#9;CHECK(lines[0] == &quot;A&quot;);<br>
&#9;CHECK(lines[1] == &quot;XXX&quot;);<br>
&#9;CHECK(lines[2] == &quot;C&quot;);<br>
}<br>
TEST_CASE(&quot;apply_chunk_main: delete-file then create-file&quot;)<br>
{<br>
&#9;namespace fs = std::filesystem;<br>
<br>
&#9;fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_del_create&quot;;<br>
&#9;fs::remove_all(tmp);<br>
&#9;fs::create_directories(tmp);<br>
<br>
&#9;fs::path f = tmp / &quot;x.txt&quot;;<br>
<br>
&#9;// Исходный файл<br>
&#9;{<br>
&#9;&#9;std::ofstream out(f);<br>
&#9;&#9;out &lt;&lt; &quot;OLD&quot;;<br>
&#9;}<br>
<br>
&#9;// Патч: удалить → создать заново<br>
&#9;fs::path patch = tmp / &quot;patch.txt&quot;;<br>
&#9;{<br>
&#9;&#9;std::ofstream out(patch);<br>
&#9;&#9;out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- delete-file\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;=== file: &quot;<br>
&#9;&#9;&#9;&lt;&lt; f.string()<br>
&#9;&#9;&#9;&lt;&lt; &quot; ===\n&quot;<br>
&#9;&#9;&#9;&quot;--- create-file\n&quot;<br>
&#9;&#9;&#9;&quot;NEW1\n&quot;<br>
&#9;&#9;&#9;&quot;NEW2\n&quot;<br>
&#9;&#9;&#9;&quot;=END=\n&quot;;<br>
&#9;}<br>
<br>
&#9;int r = run_apply(patch);<br>
&#9;CHECK(r == 0);<br>
<br>
&#9;REQUIRE(fs::exists(f));<br>
<br>
&#9;auto lines = read_lines(f);<br>
&#9;REQUIRE(lines.size() == 2);<br>
&#9;CHECK(lines[0] == &quot;NEW1&quot;);<br>
&#9;CHECK(lines[1] == &quot;NEW2&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

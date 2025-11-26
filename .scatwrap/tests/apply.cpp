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
    std::ifstream in(p);<br>
    std::vector&lt;std::string&gt; v;<br>
    std::string s;<br>
    while (std::getline(in, s))<br>
        v.push_back(s);<br>
    return v;<br>
}<br>
<br>
int run_apply(const fs::path &amp;patch)<br>
{<br>
    std::string arg0 = &quot;apply&quot;;<br>
    std::string arg1 = patch.string();<br>
<br>
    // храним строки в живом виде<br>
    std::vector&lt;std::string&gt; args = {arg0, arg1};<br>
<br>
    // формируем argv как указатели НА ЖИВЫЕ строки<br>
    std::vector&lt;char *&gt; argv_real;<br>
    argv_real.reserve(args.size());<br>
    for (auto &amp;s : args)<br>
        argv_real.push_back(s.data());<br>
<br>
    return apply_chunk_main((int)argv_real.size(), argv_real.data());<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-after-text&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_after_text&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;a.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;LINE1\n&quot;<br>
               &quot;LINE2\n&quot;<br>
               &quot;LINE3\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- insert-after-text\n&quot;<br>
               &quot;LINE2\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;AFTER\n&quot;<br>
               &quot;TEXT\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    int r = run_apply(patch);<br>
    CHECK(r == 0);<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 5);<br>
    CHECK(lines[0] == &quot;LINE1&quot;);<br>
    CHECK(lines[1] == &quot;LINE2&quot;);<br>
    CHECK(lines[2] == &quot;AFTER&quot;);<br>
    CHECK(lines[3] == &quot;TEXT&quot;);<br>
    CHECK(lines[4] == &quot;LINE3&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: insert-before-text&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_before_text&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;b.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;AAA\n&quot;<br>
               &quot;BBB\n&quot;<br>
               &quot;CCC\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- insert-before-text\n&quot;<br>
               &quot;BBB\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;X\n&quot;<br>
               &quot;Y\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    int r = run_apply(patch);<br>
    CHECK(r == 0);<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 5);<br>
    CHECK(lines[0] == &quot;AAA&quot;);<br>
    CHECK(lines[1] == &quot;X&quot;);<br>
    CHECK(lines[2] == &quot;Y&quot;);<br>
    CHECK(lines[3] == &quot;BBB&quot;);<br>
    CHECK(lines[4] == &quot;CCC&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: replace-text&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_replace_text&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;c.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;alpha\n&quot;<br>
               &quot;beta\n&quot;<br>
               &quot;gamma\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- replace-text\n&quot;<br>
               &quot;beta\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;BETA1\n&quot;<br>
               &quot;BETA2\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    int r = run_apply(patch);<br>
    CHECK(r == 0);<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 4);<br>
    CHECK(lines[0] == &quot;alpha&quot;);<br>
    CHECK(lines[1] == &quot;BETA1&quot;);<br>
    CHECK(lines[2] == &quot;BETA2&quot;);<br>
    CHECK(lines[3] == &quot;gamma&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: delete-text&quot;)<br>
{<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_delete_text&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;d.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;one\n&quot;<br>
               &quot;two\n&quot;<br>
               &quot;three\n&quot;<br>
               &quot;four\n&quot;;<br>
    }<br>
<br>
    fs::path patch = tmp / &quot;patch.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- delete-text\n&quot;<br>
               &quot;two\n&quot;<br>
               &quot;three\n&quot;<br>
               &quot;---\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    int r = run_apply(patch);<br>
    CHECK(r == 0);<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 2);<br>
    CHECK(lines[0] == &quot;one&quot;);<br>
    CHECK(lines[1] == &quot;four&quot;);<br>
}<br>
<br>
TEST_CASE(&quot;apply_chunk_main: apply-stdin&quot;)<br>
{<br>
    namespace fs = std::filesystem;<br>
<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_stdin&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;stdin.txt&quot;;<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;A\nB\nC\n&quot;;<br>
    }<br>
<br>
    std::string patch = &quot;=== file: &quot; + f.string() +<br>
                        &quot; ===\n&quot;<br>
                        &quot;--- replace-text\n&quot;<br>
                        &quot;B\n&quot;<br>
                        &quot;---\n&quot;<br>
                        &quot;XXX\n&quot;<br>
                        &quot;=END=\n&quot;;<br>
<br>
    std::istringstream fake_stdin(patch);<br>
    auto *old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());<br>
<br>
    const char *argv[] = {&quot;scat&quot;, &quot;--apply-stdin&quot;};<br>
    int r = scat_main(2, (char **)argv);<br>
<br>
    std::cin.rdbuf(old_stdin);<br>
<br>
    CHECK(r == 0);<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 3);<br>
    CHECK(lines[0] == &quot;A&quot;);<br>
    CHECK(lines[1] == &quot;XXX&quot;);<br>
    CHECK(lines[2] == &quot;C&quot;);<br>
}<br>
TEST_CASE(&quot;apply_chunk_main: delete-file then create-file&quot;)<br>
{<br>
    namespace fs = std::filesystem;<br>
<br>
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_del_create&quot;;<br>
    fs::remove_all(tmp);<br>
    fs::create_directories(tmp);<br>
<br>
    fs::path f = tmp / &quot;x.txt&quot;;<br>
<br>
    // Исходный файл<br>
    {<br>
        std::ofstream out(f);<br>
        out &lt;&lt; &quot;OLD&quot;;<br>
    }<br>
<br>
    // Патч: удалить → создать заново<br>
    fs::path patch = tmp / &quot;patch.txt&quot;;<br>
    {<br>
        std::ofstream out(patch);<br>
        out &lt;&lt; &quot;=== file: &quot; &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- delete-file\n&quot;<br>
               &quot;=END=\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;=== file: &quot;<br>
            &lt;&lt; f.string()<br>
            &lt;&lt; &quot; ===\n&quot;<br>
               &quot;--- create-file\n&quot;<br>
               &quot;NEW1\n&quot;<br>
               &quot;NEW2\n&quot;<br>
               &quot;=END=\n&quot;;<br>
    }<br>
<br>
    int r = run_apply(patch);<br>
    CHECK(r == 0);<br>
<br>
    REQUIRE(fs::exists(f));<br>
<br>
    auto lines = read_lines(f);<br>
    REQUIRE(lines.size() == 2);<br>
    CHECK(lines[0] == &quot;NEW1&quot;);<br>
    CHECK(lines[1] == &quot;NEW2&quot;);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

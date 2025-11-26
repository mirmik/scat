<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/apply.cpp</title>
</head>
<body>
<pre><code>
#include &quot;doctest/doctest.h&quot;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;string&gt;
#include &lt;vector&gt;
#include &lt;sstream&gt;
#include &lt;scat.h&gt;
#include &lt;iostream&gt;

int apply_chunk_main(int argc, char** argv);

namespace fs = std::filesystem;

std::vector&lt;std::string&gt; read_lines(const fs::path&amp; p)
{
    std::ifstream in(p);
    std::vector&lt;std::string&gt; v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

int run_apply(const fs::path&amp; patch)
{
    std::string arg0 = &quot;apply&quot;;
    std::string arg1 = patch.string();

    // храним строки в живом виде
    std::vector&lt;std::string&gt; args = {arg0, arg1};

    // формируем argv как указатели НА ЖИВЫЕ строки
    std::vector&lt;char*&gt; argv_real;
    argv_real.reserve(args.size());
    for (auto&amp; s : args)
        argv_real.push_back(s.data());

    return apply_chunk_main((int)argv_real.size(), argv_real.data());
}



TEST_CASE(&quot;apply_chunk_main: insert-after-text&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_after_text&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;a.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt;
            &quot;LINE1\n&quot;
            &quot;LINE2\n&quot;
            &quot;LINE3\n&quot;;
    }

    fs::path patch = tmp / &quot;patch.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- insert-after-text\n&quot;
            &quot;LINE2\n&quot;
            &quot;---\n&quot;
            &quot;AFTER\n&quot;
            &quot;TEXT\n&quot;
            &quot;=END=\n&quot;;
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == &quot;LINE1&quot;);
    CHECK(lines[1] == &quot;LINE2&quot;);
    CHECK(lines[2] == &quot;AFTER&quot;);
    CHECK(lines[3] == &quot;TEXT&quot;);
    CHECK(lines[4] == &quot;LINE3&quot;);
}

TEST_CASE(&quot;apply_chunk_main: insert-before-text&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_insert_before_text&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;b.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt;
            &quot;AAA\n&quot;
            &quot;BBB\n&quot;
            &quot;CCC\n&quot;;
    }

    fs::path patch = tmp / &quot;patch.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- insert-before-text\n&quot;
            &quot;BBB\n&quot;
            &quot;---\n&quot;
            &quot;X\n&quot;
            &quot;Y\n&quot;
            &quot;=END=\n&quot;;
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == &quot;AAA&quot;);
    CHECK(lines[1] == &quot;X&quot;);
    CHECK(lines[2] == &quot;Y&quot;);
    CHECK(lines[3] == &quot;BBB&quot;);
    CHECK(lines[4] == &quot;CCC&quot;);
}

TEST_CASE(&quot;apply_chunk_main: replace-text&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_replace_text&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;c.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt;
            &quot;alpha\n&quot;
            &quot;beta\n&quot;
            &quot;gamma\n&quot;;
    }

    fs::path patch = tmp / &quot;patch.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- replace-text\n&quot;
            &quot;beta\n&quot;
            &quot;---\n&quot;
            &quot;BETA1\n&quot;
            &quot;BETA2\n&quot;
            &quot;=END=\n&quot;;
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == &quot;alpha&quot;);
    CHECK(lines[1] == &quot;BETA1&quot;);
    CHECK(lines[2] == &quot;BETA2&quot;);
    CHECK(lines[3] == &quot;gamma&quot;);
}

TEST_CASE(&quot;apply_chunk_main: delete-text&quot;)
{
    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_delete_text&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;d.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt;
            &quot;one\n&quot;
            &quot;two\n&quot;
            &quot;three\n&quot;
            &quot;four\n&quot;;
    }

    fs::path patch = tmp / &quot;patch.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- delete-text\n&quot;
            &quot;two\n&quot;
            &quot;three\n&quot;
            &quot;---\n&quot;
            &quot;=END=\n&quot;;
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == &quot;one&quot;);
    CHECK(lines[1] == &quot;four&quot;);
}

TEST_CASE(&quot;apply_chunk_main: apply-stdin&quot;)
{
    namespace fs = std::filesystem;

    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_stdin&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;stdin.txt&quot;;
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;A\nB\nC\n&quot;;
    }

    std::string patch =
        &quot;=== file: &quot; + f.string() + &quot; ===\n&quot;
        &quot;--- replace-text\n&quot;
        &quot;B\n&quot;
        &quot;---\n&quot;
        &quot;XXX\n&quot;
        &quot;=END=\n&quot;;

    std::istringstream fake_stdin(patch);
    auto* old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());

    const char* argv[] = {&quot;scat&quot;, &quot;--apply-stdin&quot;};
    int r = scat_main(2, (char**)argv);

    std::cin.rdbuf(old_stdin);

    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == &quot;A&quot;);
    CHECK(lines[1] == &quot;XXX&quot;);
    CHECK(lines[2] == &quot;C&quot;);
}
TEST_CASE(&quot;apply_chunk_main: delete-file then create-file&quot;)
{
    namespace fs = std::filesystem;

    fs::path tmp = fs::temp_directory_path() / &quot;chunk_test_del_create&quot;;
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / &quot;x.txt&quot;;

    // Исходный файл
    {
        std::ofstream out(f);
        out &lt;&lt; &quot;OLD&quot;;
    }

    // Патч: удалить → создать заново
    fs::path patch = tmp / &quot;patch.txt&quot;;
    {
        std::ofstream out(patch);
        out &lt;&lt;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- delete-file\n&quot;
            &quot;=END=\n&quot;
            &quot;\n&quot;
            &quot;=== file: &quot; &lt;&lt; f.string() &lt;&lt; &quot; ===\n&quot;
            &quot;--- create-file\n&quot;
            &quot;NEW1\n&quot;
            &quot;NEW2\n&quot;
            &quot;=END=\n&quot;;
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    REQUIRE(fs::exists(f));

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == &quot;NEW1&quot;);
    CHECK(lines[1] == &quot;NEW2&quot;);
}

</code></pre>
</body>
</html>

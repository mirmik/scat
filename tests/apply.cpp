#include "doctest/doctest.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <scat.h>
#include <iostream>

int apply_chunk_main(int argc, char** argv);

namespace fs = std::filesystem;

std::vector<std::string> read_lines(const fs::path& p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

int run_apply(const fs::path& patch)
{
    std::string arg0 = "apply";
    std::string arg1 = patch.string();

    // храним строки в живом виде
    std::vector<std::string> args = {arg0, arg1};

    // формируем argv как указатели НА ЖИВЫЕ строки
    std::vector<char*> argv_real;
    argv_real.reserve(args.size());
    for (auto& s : args)
        argv_real.push_back(s.data());

    return apply_chunk_main((int)argv_real.size(), argv_real.data());
}

TEST_CASE("apply_chunk_main: replace")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out << "line0\nline1\nline2\nline3\nline4\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string() << " ===\n"
            << "--- replace 1:3\n"
            << "NEW_A\n"
            << "NEW_B\n"
            << "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "line0");
    CHECK(lines[1] == "NEW_A");
    CHECK(lines[2] == "NEW_B");
    CHECK(lines[3] == "line4");
}

TEST_CASE("apply_chunk_main: delete")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_delete";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "b.txt";
    {
        std::ofstream out(f);
        out << "A\nB\nC\nD\nE\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string() << " ===\n"
            << "--- delete 1:3\n"
            << "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "A");
    CHECK(lines[1] == "E");
}

TEST_CASE("apply_chunk_main: insert-after")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "c.txt";
    {
        std::ofstream out(f);
        out << "X\nY\nZ\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string() << " ===\n"
            << "--- insert-after 1\n"
            << "MIDDLE_1\n"
            << "MIDDLE_2\n"
            << "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);

    CHECK(lines[0] == "X");
    CHECK(lines[1] == "Y");
    CHECK(lines[2] == "MIDDLE_1");
    CHECK(lines[3] == "MIDDLE_2");
    CHECK(lines[4] == "Z");
}

TEST_CASE("apply_chunk_main: insert-after on new file")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_newfile";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "new.txt";

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string() << " ===\n"
            << "--- insert-after -1\n"
            << "HELLO\n"
            << "WORLD\n"
            << "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "HELLO");
    CHECK(lines[1] == "WORLD");
}

TEST_CASE("apply_chunk_main: multiple sections for same file, index-stable")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_multi";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "f.txt";
    {
        std::ofstream out(f);
        out << "A\nB\nC\nD\nE\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out <<
"=== file: " << f.string() << " ===\n"
"--- insert-after 0\n"
"Y\n"
"=END=\n"
"=== file: " << f.string() << " ===\n"
"--- replace 1:1\n"
"X\n"
"=END=\n"
"\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 6);

    CHECK(lines[0] == "A");
    CHECK(lines[1] == "Y");
    CHECK(lines[2] == "X");
    CHECK(lines[3] == "C");
    CHECK(lines[4] == "D");
    CHECK(lines[5] == "E");
}

TEST_CASE("apply_chunk_main: insert-after-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_after_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out <<
            "LINE1\n"
            "LINE2\n"
            "LINE3\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out <<
            "=== file: " << f.string() << " ===\n"
            "--- insert-after-text\n"
            "LINE2\n"
            "---\n"
            "AFTER\n"
            "TEXT\n"
            "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "LINE1");
    CHECK(lines[1] == "LINE2");
    CHECK(lines[2] == "AFTER");
    CHECK(lines[3] == "TEXT");
    CHECK(lines[4] == "LINE3");
}

TEST_CASE("apply_chunk_main: insert-before-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_before_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "b.txt";
    {
        std::ofstream out(f);
        out <<
            "AAA\n"
            "BBB\n"
            "CCC\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out <<
            "=== file: " << f.string() << " ===\n"
            "--- insert-before-text\n"
            "BBB\n"
            "---\n"
            "X\n"
            "Y\n"
            "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "AAA");
    CHECK(lines[1] == "X");
    CHECK(lines[2] == "Y");
    CHECK(lines[3] == "BBB");
    CHECK(lines[4] == "CCC");
}

TEST_CASE("apply_chunk_main: replace-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "c.txt";
    {
        std::ofstream out(f);
        out <<
            "alpha\n"
            "beta\n"
            "gamma\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out <<
            "=== file: " << f.string() << " ===\n"
            "--- replace-text\n"
            "beta\n"
            "---\n"
            "BETA1\n"
            "BETA2\n"
            "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "alpha");
    CHECK(lines[1] == "BETA1");
    CHECK(lines[2] == "BETA2");
    CHECK(lines[3] == "gamma");
}

TEST_CASE("apply_chunk_main: delete-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_delete_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "d.txt";
    {
        std::ofstream out(f);
        out <<
            "one\n"
            "two\n"
            "three\n"
            "four\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out <<
            "=== file: " << f.string() << " ===\n"
            "--- delete-text\n"
            "two\n"
            "three\n"
            "---\n"
            "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "one");
    CHECK(lines[1] == "four");
}
TEST_CASE("apply_chunk_main: apply-stdin")
{
    namespace fs = std::filesystem;

    fs::path tmp = fs::temp_directory_path() / "chunk_test_stdin";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "stdin.txt";
    {
        std::ofstream out(f);
        out << "A\nB\nC\n";
    }

    std::string patch =
        "=== file: " + f.string() + " ===\n"
        "--- replace 1:1\n"
        "XXX\n"
        "=END=\n";

    std::istringstream fake_stdin(patch);
    auto* old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());

    const char* argv[] = {"scat", "--apply-stdin"};
    int r = scat_main(2, (char**)argv);

    std::cin.rdbuf(old_stdin);

    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "A");
    CHECK(lines[1] == "XXX");
    CHECK(lines[2] == "C");
}

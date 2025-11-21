#include "doctest/doctest.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int apply_chunk_main(int argc, char** argv);

namespace fs = std::filesystem;

static std::vector<std::string> read_lines(const fs::path& p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

static int run_apply(const fs::path& patch)
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

#include "doctest/doctest.h"
#include "scat.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::vector<std::string> read_lines(const fs::path &p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

TEST_CASE("apply_chunk_main: apply-stdin via scat")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_stdin";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "stdin.txt";
    {
        std::ofstream out(f);
        out << "A\nB\nC\n";
    }

    std::string patch = "=== file: " + f.string() +
                        " ===\n"
                        "--- replace-text\n"
                        "B\n"
                        "---\n"
                        "XXX\n"
                        "=END=\n";

    std::istringstream fake_stdin(patch);
    auto *old_stdin = std::cin.rdbuf(fake_stdin.rdbuf());

    const char *argv[] = {"scat", "--apply-stdin"};
    int r = scat_main(2, (char **)argv);

    std::cin.rdbuf(old_stdin);

    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "A");
    CHECK(lines[1] == "XXX");
    CHECK(lines[2] == "C");
}

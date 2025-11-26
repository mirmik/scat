#include "glob.h"
#include "doctest/doctest.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

static void write_file(const fs::path &p, const std::string &text)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p);
    out << text;
}

static std::vector<std::string> to_rel(const std::vector<fs::path> &v,
                                       const fs::path &root)
{
    std::vector<std::string> out;
    for (auto &p : v)
        out.push_back(fs::relative(p, root).generic_string());
    std::sort(out.begin(), out.end());
    return out;
}

static void check_paths(const std::vector<std::string> &actual,
                        std::initializer_list<const char *> expected)
{
    CHECK(actual.size() == expected.size());

    size_t i = 0;
    for (auto *e : expected)
    {
        CHECK(actual[i] == e);
        ++i;
    }
}

TEST_CASE("glob: basic *")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_a";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");
    write_file(tmp / "b.cpp", "B");
    write_file(tmp / "c.txt", "C");

    auto out = expand_glob((tmp / "*.txt").generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.txt", "c.txt"});
}

TEST_CASE("glob: recursive **")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_b";
    fs::remove_all(tmp);

    write_file(tmp / "1.txt", "A");
    write_file(tmp / "x/2.txt", "B");
    write_file(tmp / "x/y/3.txt", "C");

    auto out = expand_glob((tmp / "**").generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"1.txt", "x/2.txt", "x/y/3.txt"});
}

TEST_CASE("glob: **/*.cpp")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_c";
    fs::remove_all(tmp);

    write_file(tmp / "a.cpp", "A");
    write_file(tmp / "b.h", "B");
    write_file(tmp / "x/c.cpp", "C");
    write_file(tmp / "x/y/z.cpp", "Z");

    auto out = expand_glob((tmp / "**/*.cpp").generic_string());
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.cpp", "x/c.cpp", "x/y/z.cpp"});
}

TEST_CASE("glob: foo/*/bar/**/*.txt")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_d";
    fs::remove_all(tmp);

    write_file(tmp / "foo/K/bar/a.txt", "A");
    write_file(tmp / "foo/K/bar/x/b.txt", "B");
    write_file(tmp / "foo/X/bar/c.bin", "C");
    write_file(tmp / "foo/Z/bar/y/z.txt", "Z");

    auto pat = (tmp / "foo/*/bar/**/*.txt").generic_string();
    auto out = expand_glob(pat);
    auto rel = to_rel(out, tmp);

    check_paths(rel,
                {"foo/K/bar/a.txt", "foo/K/bar/x/b.txt", "foo/Z/bar/y/z.txt"});
}

TEST_CASE("glob: no matches returns empty")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_e";
    fs::remove_all(tmp);

    write_file(tmp / "file.cpp", "A");

    auto out = expand_glob((tmp / "**/*.txt").generic_string());
    CHECK(out.empty());
}

TEST_CASE("glob: duplicates removed")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_f";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");
    write_file(tmp / "d/a.txt", "A2");

    // отработает и * и **
    auto pat1 = (tmp / "*.txt").generic_string();
    auto pat2 = (tmp / "**/*.txt").generic_string();

    auto out1 = expand_glob(pat1);
    auto out2 = expand_glob(pat2);

    std::vector<fs::path> combined;
    combined.insert(combined.end(), out1.begin(), out1.end());
    combined.insert(combined.end(), out2.begin(), out2.end());

    // удаляем дубликаты вручную, как collector делает
    std::sort(combined.begin(), combined.end());
    combined.erase(std::unique(combined.begin(), combined.end()),
                   combined.end());

    auto rel = to_rel(combined, tmp);
    check_paths(rel, {"a.txt", "d/a.txt"});
}

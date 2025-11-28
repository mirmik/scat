#include "collector.h"
#include "options.h"
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
    out.reserve(v.size());
    for (auto &p : v)
        out.push_back(fs::relative(p, root).generic_string());
    std::sort(out.begin(), out.end());
    return out;
}

static void check_paths(const std::vector<std::string> &actual,
                        std::initializer_list<const char *> expected)
{
    std::vector<std::string> exp_vec;
    exp_vec.reserve(expected.size());
    for (auto *e : expected)
        exp_vec.emplace_back(e);

    std::vector<std::string> act = actual;

    std::sort(exp_vec.begin(), exp_vec.end());
    std::sort(act.begin(), act.end());

    CHECK(act.size() == exp_vec.size());

    if (act.size() != exp_vec.size())
        return; // предотвратить каскад ошибок

    for (size_t i = 0; i < act.size(); ++i)
        CHECK(act[i] == exp_vec[i]);
}

TEST_CASE("collect_from_paths: basic * glob")
{
    fs::path tmp = fs::temp_directory_path() / "paths_glob_test_1";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");
    write_file(tmp / "b.cpp", "B");
    write_file(tmp / "c.txt", "C");

    Options opt;
    std::vector<std::string> paths = {(tmp / "*.txt").generic_string()};

    auto out = collect_from_paths(paths, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.txt", "c.txt"});
}

TEST_CASE("collect_from_paths: recursive ** glob")
{
    fs::path tmp = fs::temp_directory_path() / "paths_glob_test_2";
    fs::remove_all(tmp);

    write_file(tmp / "1.txt", "A");
    write_file(tmp / "x/2.txt", "B");
    write_file(tmp / "x/y/3.txt", "C");

    Options opt;
    std::vector<std::string> paths = {(tmp / "**/*.txt").generic_string()};

    auto out = collect_from_paths(paths, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"1.txt", "x/2.txt", "x/y/3.txt"});
}

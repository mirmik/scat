#include "collector.h"
#include "doctest/doctest.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

static void write(const fs::path& p, const std::string& text) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p);
    out << text;
}

static std::vector<std::string> to_rel(const std::vector<fs::path>& v, const fs::path& root) {
    std::vector<std::string> out;
    for (auto& p : v)
        out.push_back(fs::relative(p, root).generic_string());
    std::sort(out.begin(), out.end());
    return out;
}

TEST_CASE("collector glob: basic *") {
    fs::path tmp = fs::temp_directory_path() / "glob_test1";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    write(tmp / "a.txt", "A");
    write(tmp / "b.cpp", "B");
    write(tmp / "c.txt", "C");

    Rule r{"*.txt", false};
    Options opt;

    auto out = collect_from_rules({r}, opt);
    auto rel = to_rel(out, tmp);

    CHECK(rel == std::vector<std::string>{"a.txt", "c.txt"});
}

TEST_CASE("collector glob: ** recursive") {
    fs::path tmp = fs::temp_directory_path() / "glob_test2";
    fs::remove_all(tmp);

    write(tmp / "a.txt", "A");
    write(tmp / "x/b.txt", "B");
    write(tmp / "x/y/c.txt", "C");

    Rule r{"**", false};
    Options opt;

    auto out = collect_from_rules({r}, opt);
    auto rel = to_rel(out, tmp);

    CHECK(rel == std::vector<std::string>{"a.txt", "x/b.txt", "x/y/c.txt"});
}

TEST_CASE("collector glob: **/*.cpp") {
    fs::path tmp = fs::temp_directory_path() / "glob_test3";
    fs::remove_all(tmp);

    write(tmp / "a.cpp", "A");
    write(tmp / "b.h", "B");
    write(tmp / "x/c.cpp", "C");
    write(tmp / "x/y/z.cpp", "Z");

    Rule r{"**/*.cpp", false};
    Options opt;

    auto out = collect_from_rules({r}, opt);
    auto rel = to_rel(out, tmp);

    CHECK(rel == std::vector<std::string>{
        "a.cpp", "x/c.cpp", "x/y/z.cpp"
    });
}

TEST_CASE("collector glob: foo/*/bar/**/*.txt") {
    fs::path tmp = fs::temp_directory_path() / "glob_test4";
    fs::remove_all(tmp);

    write(tmp / "foo/K/bar/a.txt",     "A");
    write(tmp / "foo/K/bar/x/b.txt",   "B");
    write(tmp / "foo/X/bar/c.bin",     "C");
    write(tmp / "foo/Z/bar/y/z.txt",   "Z");

    Rule r{"foo/*/bar/**/*.txt", false};
    Options opt;

    auto out = collect_from_rules({r}, opt);
    auto rel = to_rel(out, tmp);

    CHECK(rel == std::vector<std::string>{
        "foo/K/bar/a.txt",
        "foo/K/bar/x/b.txt",
        "foo/Z/bar/y/z.txt"
    });
}

TEST_CASE("collector glob: no matches") {
    fs::path tmp = fs::temp_directory_path() / "glob_test5";
    fs::remove_all(tmp);

    write(tmp / "a.cpp", "A");
    Rule r{"**/*.txt", false};
    Options opt;

    auto out = collect_from_rules({r}, opt);
    CHECK(out.empty());
}

TEST_CASE("collector glob: duplicates removed") {
    fs::path tmp = fs::temp_directory_path() / "glob_test6";
    fs::remove_all(tmp);

    write(tmp / "a.txt", "A");
    write(tmp / "d/a.txt", "A2");

    Rule r1{"*.txt", false};
    Rule r2{"**/*.txt", false};
    Options opt;

    auto out = collect_from_rules({r1, r2}, opt);
    auto rel = to_rel(out, tmp);

    CHECK(rel == std::vector<std::string>{
        "a.txt",
        "d/a.txt"
    });
}

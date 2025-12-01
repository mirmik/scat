#include "collector.h"
#include "glob.h"
#include "guard/guard.h"
#include "options.h"
#include "rules.h"


#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

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

// ============================================================================
// glob tests — как раньше, без смены current_path
// ============================================================================

TEST_CASE("glob: basic *")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_a2";
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
    fs::path tmp = fs::temp_directory_path() / "glob_test_b2";
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
    fs::path tmp = fs::temp_directory_path() / "glob_test_c2";
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
    fs::path tmp = fs::temp_directory_path() / "glob_test_d2";
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

TEST_CASE("glob: ** matches zero or more levels before filename")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_f2";
    fs::remove_all(tmp);

    write_file(tmp / "root.txt", "R");
    write_file(tmp / "deep/inner.txt", "D");
    write_file(tmp / "deep/nested/inner.txt", "N");

    auto pat = (tmp / "deep/**/inner.txt").generic_string();
    auto out = expand_glob(pat);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"deep/inner.txt", "deep/nested/inner.txt"});
}

TEST_CASE("glob: no matches")
{
    fs::path tmp = fs::temp_directory_path() / "glob_test_e2";
    fs::remove_all(tmp);

    write_file(tmp / "file.cpp", "A");

    auto out = expand_glob((tmp / "**/*.txt").generic_string());
    CHECK(out.empty());
}

// ============================================================================
// collector tests — тоже на абсолютных путях
// ============================================================================

TEST_CASE("collector: include then exclude subdir")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_1";
    fs::remove_all(tmp);

    write_file(tmp / "src/a.cpp", "A");
    write_file(tmp / "src/b.cpp", "B");
    write_file(tmp / "src/tests/c.cpp", "C");

    std::vector<Rule> rules = {{(tmp / "src/**/*.cpp").generic_string(), false},
                               {(tmp / "src/tests/**").generic_string(), true}};

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"src/a.cpp", "src/b.cpp"});
}

TEST_CASE("collector: exclude *.cpp top level")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_2";
    fs::remove_all(tmp);

    write_file(tmp / "a.cpp", "A");
    write_file(tmp / "b.h", "B");
    write_file(tmp / "sub/c.cpp", "C");

    std::vector<Rule> rules = {
        {(tmp / "**").generic_string(), false},
        {(tmp / "*.cpp").generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"b.h", "sub/c.cpp"});
}

TEST_CASE("collector: exclude all cpp recursively")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_3";
    fs::remove_all(tmp);

    write_file(tmp / "a.cpp", "A");
    write_file(tmp / "x/b.cpp", "B");
    write_file(tmp / "x/y/c.cpp", "C");
    write_file(tmp / "ok.txt", "D");

    std::vector<Rule> rules = {
        {(tmp / "**").generic_string(), false},
        {(tmp / "**/*.cpp").generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"ok.txt"});
}

TEST_CASE("collector: exclude then re-include deeper match")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_4";
    fs::remove_all(tmp);

    write_file(tmp / "keep.txt", "K");
    write_file(tmp / "tmp/skip.txt", "S");
    write_file(tmp / "tmp/keep.txt", "R");

    std::vector<Rule> rules = {
        {(tmp / "**/*.txt").generic_string(), false},
        {(tmp / "tmp/**").generic_string(), true},
        {(tmp / "tmp/keep.txt").generic_string(), false},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"keep.txt", "tmp/keep.txt"});
}

TEST_CASE("collector: exclude-all then include specific later")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_5";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");
    write_file(tmp / "b.txt", "B");

    std::vector<Rule> rules = {
        {(tmp / "**").generic_string(), true},
        {(tmp / "a.txt").generic_string(), false},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.txt"});
}

TEST_CASE("collector: duplicate include patterns are deduped")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_6";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");
    write_file(tmp / "dir/a.txt", "A2");

    std::vector<Rule> rules = {
        {(tmp / "a.txt").generic_string(), false},
        {(tmp / "**/*.txt").generic_string(), false},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.txt", "dir/a.txt"});
}

TEST_CASE("collector: include-exclude-include chain")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_4";
    fs::remove_all(tmp);

    write_file(tmp / "data/keep/a.txt", "A");
    write_file(tmp / "data/keep/b.cpp", "B");
    write_file(tmp / "data/drop/c.txt", "C");

    std::vector<Rule> rules = {
        {(tmp / "data/**").generic_string(), false},
        {(tmp / "data/drop/**").generic_string(), true},
        {(tmp / "data/keep/*.txt").generic_string(), false},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"data/keep/a.txt", "data/keep/b.cpp"});
}

TEST_CASE("collector: empty after exclude")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_5";
    fs::remove_all(tmp);

    write_file(tmp / "a.txt", "A");

    std::vector<Rule> rules = {
        {(tmp / "*.txt").generic_string(), false},
        {(tmp / "*.txt").generic_string(), true},
    };

    Options opt;
    auto out = collect_from_rules(rules, opt);

    CHECK(out.empty());
}

TEST_CASE("collector: complex structure with 5 rules")
{
    fs::path tmp = fs::temp_directory_path() / "collector_complex_test";
    fs::remove_all(tmp);

    // ----------------------------
    // создаём структуру на 15 файлов
    // ----------------------------
    write_file(tmp / "root.txt", "R");
    write_file(tmp / "root.bin", "RB");
    write_file(tmp / "readme.md", "MD");

    write_file(tmp / "src/a.cpp", "A");
    write_file(tmp / "src/b.cpp", "B");
    write_file(tmp / "src/c.log", "CLOG");

    write_file(tmp / "src/tools/t1.txt", "T1");
    write_file(tmp / "src/tools/t2.txt", "T2");
    write_file(tmp / "src/tools/helper.cpp", "HC");

    write_file(tmp / "data/info.txt", "INFO");
    write_file(tmp / "data/raw/raw1.bin", "BIN1");
    write_file(tmp / "data/raw/raw2.bin", "BIN2");

    write_file(tmp / "build/tmp1.log", "TMP1");
    write_file(tmp / "build/tmp2.log", "TMP2");

    write_file(tmp / "misc/x.cpp", "X");
    write_file(tmp / "misc/y.txt", "Y");

    // ----------------------------
    // ПРАВИЛА
    // ----------------------------

    std::vector<Rule> rules = {
        // 1) собрать всё
        {(tmp / "**").generic_string(), false},

        // 2) исключить data/raw/**
        {(tmp / "data/raw/**").generic_string(), true},

        // 3) исключить все *.log
        {(tmp / "**/*.log").generic_string(), true},

        // 4) добавить текстовые файлы только в tools
        {(tmp / "src/tools/*.txt").generic_string(), false},

        // 5) исключить бинарники только в корне (root.bin)
        {(tmp / "*.bin").generic_string(), true}};

    Options opt;
    auto out = collect_from_rules(rules, opt);
    auto rel = to_rel(out, tmp);

    // ----------------------------
    // ОЖИДАЕМЫЙ РЕЗУЛЬТАТ
    // ----------------------------
    //
    // Файлы, которые должны остаться:
    //
    //   root.txt
    //   readme.md
    //   src/a.cpp
    //   src/b.cpp
    //   src/tools/t1.txt
    //   src/tools/t2.txt
    //   src/tools/helper.cpp
    //   data/info.txt
    //   misc/x.cpp
    //   misc/y.txt
    //
    // НЕ должно быть:
    //   data/raw/*        (правило #2)
    //   *.log             (правило #3)
    //   root.bin          (правило #5)
    //
    check_paths(rel,
                {
                    "root.txt",
                    "readme.md",
                    "src/a.cpp",
                    "src/b.cpp",
                    "src/tools/t1.txt",
                    "src/tools/t2.txt",
                    "src/tools/helper.cpp",
                    "data/info.txt",
                    "misc/x.cpp",
                    "misc/y.txt",
                });
}

TEST_CASE("collector: --exclude matches @ semantics (single arg)")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_7";
    fs::remove_all(tmp);

    write_file(tmp / "a.cpp", "A2");
    write_file(tmp / "keep.cpp", "K");
    write_file(tmp / "keep.h", "H");

    std::string glob_all = (tmp / "*").generic_string();
    std::string glob_cpp = (tmp / "*.cpp").generic_string();
    std::string keep_cpp = (tmp / "keep.cpp").generic_string();
    std::string glob_cpp_at = "@" + glob_cpp;

    // Вариант с --exclude
    std::vector<const char *> argv_ex = {"scat",
                                         "-l",
                                         glob_all.c_str(),
                                         "--exclude",
                                         glob_cpp.c_str(),
                                         keep_cpp.c_str()};

    // Вариант с эквивалентным @
    std::vector<const char *> argv_at = {
        "scat", "-l", glob_all.c_str(), glob_cpp_at.c_str(), keep_cpp.c_str()};

    Options opt_ex = parse_options(static_cast<int>(argv_ex.size()),
                                   const_cast<char **>(argv_ex.data()));
    Options opt_at = parse_options(static_cast<int>(argv_at.size()),
                                   const_cast<char **>(argv_at.data()));

    auto out_ex = collect_from_paths_with_excludes(opt_ex.arg_rules, opt_ex);
    auto rel_ex = to_rel(out_ex, tmp);

    auto out_at = collect_from_paths_with_excludes(opt_at.arg_rules, opt_at);
    auto rel_at = to_rel(out_at, tmp);

    check_paths(rel_ex, {"keep.cpp", "keep.h"});
    check_paths(rel_at, {"keep.cpp", "keep.h"});
}

TEST_CASE("collector: --exclude glob after shell expansion with reinclude")
{
    fs::path tmp = fs::temp_directory_path() / "collector_test_8";
    fs::remove_all(tmp);

    write_file(tmp / "a.h", "AH");
    write_file(tmp / "b.h", "BH");
    write_file(tmp / "a.cpp", "AC");
    write_file(tmp / "b.cpp", "BC");

    std::string glob_all = (tmp / "*").generic_string();
    std::string a_cpp = (tmp / "a.cpp").generic_string();
    std::string b_cpp = (tmp / "b.cpp").generic_string();

    // Имитируем argv после раскрутки: include glob, затем все .cpp как
    // аргументы после --exclude, и отдельное явное включение b.cpp.
    std::vector<const char *> argv = {"scat",
                                      "-l",
                                      glob_all.c_str(),
                                      "--exclude",
                                      a_cpp.c_str(),
                                      b_cpp.c_str(),
                                      b_cpp.c_str()};

    Options opt = parse_options(static_cast<int>(argv.size()),
                                const_cast<char **>(argv.data()));

    auto out = collect_from_paths_with_excludes(opt.arg_rules, opt);
    auto rel = to_rel(out, tmp);

    check_paths(rel, {"a.h", "b.h", "b.cpp"});
}

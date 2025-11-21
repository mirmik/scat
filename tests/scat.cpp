#include "doctest/doctest.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// универсальный popen/pclose под Windows и Unix
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

static std::string run_cmd(const std::string& cmd)
{
    std::array<char, 4096> buf{};
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    REQUIRE(pipe != nullptr);

    while (fgets(buf.data(), (int)buf.size(), pipe))
        result += buf.data();

    pclose(pipe);
    return result;
}

TEST_CASE("scat walks example/ correctly")
{
    // путь к бинарнику scat
#ifdef _WIN32
    fs::path exe = fs::current_path() / "Release" / "scat.exe";
#else
    fs::path exe = fs::current_path() / "scat";
#endif

    REQUIRE(fs::exists(exe));

    // путь к example/
    fs::path example = fs::current_path().parent_path() / "example";
    if (fs::exists(example) == false)
        example = fs::current_path() / "example";

    REQUIRE(fs::exists(example));

    // запускаем утилиту
    std::ostringstream cmd;

#ifdef _WIN32
    cmd << "\"" << exe.string() << "\" \"" << example.string() << "\" -r";
#else
    cmd << exe << " \"" << example.string() << "\" -r";
#endif

    std::string out = run_cmd(cmd.str());

    // ожидаемый вывод
    const std::string expected = "===== example/a/b/2.txt =====\n"
                                 "Hello World!\n"
                                 "\n"
                                 "===== example/1.txt =====\n"
                                 "Hi\n"
                                 "\n"
                                 "===== example/c/3.txt =====\n"
                                 "You find me!";

    // убираем финальный \n
    if (!out.empty() && out.back() == '\n')
        out.pop_back();

    CHECK(out == expected);
}

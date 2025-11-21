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

static std::string trim(const std::string& str)
{
    const char* whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

static std::vector<std::string> split_lines(const std::string& str)
{
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;
    while (std::getline(stream, line))
    {
        lines.push_back(line);
    }
    return lines;
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

    int variant = 0;
    // путь к example/
    fs::path example = fs::current_path().parent_path() / "example";
    if (fs::exists(example))
    {
        variant = 1;
    }

    else
    {
        variant = 0;
        example = fs::current_path() / "example";
    }

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
    std::string expected0 = "===== example/a/b/2.txt =====\n"
                            "Hello World!\n"
                            "\n"
                            "===== example/1.txt =====\n"
                            "Hi\n"
                            "\n"
                            "===== example/c/3.txt =====\n"
                            "You find me!";

    std::string expected1 = "===== ../example/a/b/2.txt =====\n"
                            "Hello World!\n"
                            "\n"
                            "===== ../example/1.txt =====\n"
                            "Hi\n"
                            "\n"
                            "===== ../example/c/3.txt =====\n"
                            "You find me!";

    out = trim(out);
    expected0 = trim(expected0);
    expected1 = trim(expected1);

    auto lines_out = split_lines(out);
    auto lines_expected = split_lines(variant == 0 ? expected0 : expected1);

    REQUIRE(lines_out.size() == lines_expected.size());
    for (size_t i = 0; i < lines_expected.size(); ++i)
    {
        REQUIRE(lines_out[i] == lines_expected[i]);
    }
}

#include "doctest/doctest.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

static bool starts_with(const std::string& s, const std::string& prefix)
{
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool ends_with(const std::string& s, const std::string& suffix)
{
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string to_unix_path(const fs::path& p)
{
    std::string s = p.string();
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start])))
        ++start;

    if (start == s.size())
        return "";

    size_t end = s.size() - 1;
    while (end > start && isspace(static_cast<unsigned char>(s[end])))
        --end;

    return s.substr(start, end - start + 1);
}

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

// --- Parsing ---

struct Block
{
    std::string path;
    std::string content;
};

static std::vector<Block> parse_blocks(const std::string& out)
{
    std::vector<Block> result;

    std::istringstream s(out);
    std::string line;
    Block current;

    while (std::getline(s, line))
    {
        if (starts_with(line, "===== ") && ends_with(line, " ====="))
        {
            if (!current.path.empty())
                result.push_back(current);

            current = Block{};
            current.path = line.substr(6, line.size() - 6 - 6); // remove ===== <path> =====
        }
        else
        {
            if (!current.path.empty())
            {
                if (!current.content.empty())
                    current.content += "\n";
                current.content += line;
            }
        }
    }

    if (!current.path.empty())
        result.push_back(current);

    return result;
}

static fs::path find_example_dir(const fs::path& app_dir)
{
    std::vector<fs::path> candidates = {fs::current_path() / "example", fs::current_path().parent_path() / "example",
                                        app_dir / "example", app_dir.parent_path() / "example",
                                        app_dir.parent_path().parent_path() / "example"};

    for (const auto& p : candidates)
    {
        if (fs::exists(p))
            return p;
    }

    return {};
}

std::string get_application_path()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
#else
    char buffer[4096];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
#endif
}

TEST_CASE("scat walks example/ correctly")
{
    auto app_path = get_application_path();
    REQUIRE(!app_path.empty());

    auto app_dir = fs::path(app_path).parent_path();
    fs::path exe = app_dir / "scat";

    if (!fs::exists(exe))
        exe = fs::current_path() / "scat";
    if (!fs::exists(exe))
        exe = fs::current_path() / "Release" / "scat";
    if (!fs::exists(exe))
        exe = fs::current_path() / "Debug" / "scat";
    if (!fs::exists(exe))
        exe = fs::current_path() / "scat.exe";
    if (!fs::exists(exe))
        exe = fs::current_path() / "Release" / "scat.exe";
    if (!fs::exists(exe))
        exe = fs::current_path() / "Debug" / "scat.exe";

    REQUIRE(fs::exists(exe));

    fs::path example = find_example_dir(app_dir);
    REQUIRE(!example.empty());
    REQUIRE(fs::exists(example));

    std::ostringstream cmd;

    std::string exe_path = to_unix_path(exe);
    std::string example_path = to_unix_path(example);
#ifdef _WIN32
    std::ostringstream cmd;
    cmd << "\"" << exe_path << "\" \"" << example_path << "\" -r";
#else
    cmd << exe << " \"" << example.string() << "\" -r";
#endif

    std::string out = run_cmd(cmd.str());

    // Parse output blocks
    auto blocks = parse_blocks(out);

    REQUIRE(blocks.size() == 3);

    // Sort blocks by path
    std::sort(blocks.begin(), blocks.end(), [](auto& a, auto& b) { return a.path < b.path; });

    // Expected blocks
    std::vector<Block> expected = {
        {"example/1.txt", "Hi"}, {"example/a/b/2.txt", "Hello World!"}, {"example/c/3.txt", "You find me!"}};

    // Adjust expected paths for ../example/... case
    if (starts_with(blocks[0].path, "../"))
    {
        for (auto& b : expected)
            b.path = "../" + b.path;
    }

    // Compare
    for (size_t i = 0; i < expected.size(); ++i)
    {
        CHECK(trim(blocks[i].path) == trim(expected[i].path));
        CHECK(trim(blocks[i].content) == trim(expected[i].content));
    }
}

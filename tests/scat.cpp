#include "doctest/doctest.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

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

TEST_CASE("scat walks example/ correctly")
{
#ifdef _WIN32
    fs::path exe = fs::current_path() / "Release" / "scat.exe";
#else
    fs::path exe = fs::current_path() / "scat";
#endif

    REQUIRE(fs::exists(exe));

    fs::path example1 = fs::current_path().parent_path() / "example";
    fs::path example2 = fs::current_path() / "example";

    fs::path example = fs::exists(example1) ? example1 : example2;
    REQUIRE(fs::exists(example));

    std::ostringstream cmd;
#ifdef _WIN32
    cmd << "\"" << exe.string() << "\" \"" << example.string() << "\" -r";
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

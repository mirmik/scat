#include "scat.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static bool g_use_absolute_paths = false;

std::string make_display_path(const fs::path& p)
{
    if (g_use_absolute_paths)
        return fs::absolute(p).string();

    std::error_code ec;
    fs::path rel = fs::relative(p, fs::current_path(), ec);

    if (!ec)
        return rel.string();

    return p.string();
}

void dump_file(const fs::path& p, bool first)
{
    std::ifstream in(p, std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "Cannot open: " << p << "\n";
        return;
    }

    if (!first)
        std::cout << "\n";

    std::cout << "===== " << make_display_path(p) << " =====\n";
    std::cout << in.rdbuf();
}

int scat_main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: scat [-r] [--abs] [-l] <paths...>\n";
        return 1;
    }

    bool recursive = false;
    bool list_only = false;

    std::vector<fs::path> input_paths;
    std::vector<fs::path> collected_files;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-r")
            recursive = true;
        else if (arg == "--abs")
            g_use_absolute_paths = true;
        else if (arg == "-l")
            list_only = true;
        else
            input_paths.emplace_back(arg);
    }

    if (input_paths.empty())
    {
        std::cerr << "No input paths.\n";
        return 1;
    }

    for (const auto& p : input_paths)
    {
        std::error_code ec;

        if (!fs::exists(p, ec))
        {
            std::cerr << "Not found: " << p << "\n";
            continue;
        }

        if (fs::is_regular_file(p, ec))
        {
            collected_files.push_back(fs::canonical(p, ec));
            continue;
        }

        if (fs::is_directory(p, ec))
        {
            if (!recursive)
                continue;

            for (auto& entry : fs::recursive_directory_iterator(p, ec))
            {
                if (entry.is_regular_file())
                    collected_files.push_back(entry.path());
            }
        }
    }

    if (collected_files.empty())
    {
        std::cerr << "No files collected.\n";
        return 0;
    }

    if (list_only)
    {
        for (auto& f : collected_files)
            std::cout << make_display_path(f) << "\n";

        return 0;
    }

    bool first = true;
    for (auto& f : collected_files)
    {
        dump_file(f, first);
        first = false;
    }

    return 0;
}

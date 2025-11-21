#include "scat.h"
#include "collector.h"
#include "options.h"
#include "rules.h"
#include "util.h"
#include <iostream>

bool g_use_absolute_paths = false;

int scat_main(int argc, char** argv)
{
    Options opt = parse_options(argc, argv);

    std::vector<std::filesystem::path> files;

    if (!opt.config_file.empty())
    {
        auto rules = load_rules(opt.config_file);
        files = collect_from_rules(rules, opt);
    }
    else
    {
        files = collect_from_paths(opt.paths, opt);
    }

    if (files.empty())
    {
        std::cerr << "No files collected.\n";
        return 0;
    }

    if (opt.list_only)
    {
        for (auto& f : files)
            std::cout << make_display_path(f) << "\n";
        return 0;
    }

    bool first = true;
    for (auto& f : files)
    {
        dump_file(f, first);
        first = false;
    }

    return 0;
}
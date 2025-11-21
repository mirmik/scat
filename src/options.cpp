#include "options.h"
#include <cstdlib>
#include <iostream>

static void print_help()
{
    std::cout << "Usage: scat [options] [paths...]\n"
                 "\n"
                 "Options:\n"
                 "  -r           Recursive directory processing\n"
                 "  -l           List files only\n"
                 "  --abs        Show absolute paths\n"
                 "  --config F   Read patterns from file F\n"
                 "  -h, --help   Show this help\n"
                 "\n"
                 "If no paths are given, scat reads patterns from scat.txt.\n";
}

Options parse_options(int argc, char** argv)
{
    Options opt;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];

        if (a == "-r")
            opt.recursive = true;
        else if (a == "-l")
            opt.list_only = true;
        else if (a == "--abs")
            opt.abs_paths = true;
        else if (a == "--config")
        {
            if (i + 1 < argc)
                opt.config_file = argv[++i];
            else
            {
                std::cerr << "--config requires file\n";
                std::exit(1);
            }
        }
        else if (a == "-h" || a == "--help")
        {
            print_help();
            std::exit(0);
        }
        else
        {
            opt.paths.push_back(a);
        }
    }

    // Автоматический config mode:
    // если путей нет и конфиг явно не указан
    if (opt.paths.empty() && opt.config_file.empty())
        opt.config_file = "scat.txt";

    return opt;
}

#include "options.h"
#include <cstdlib>
#include <iostream>

static void print_help()
{
    std::cout << "Usage: scat [options] [paths...]\n"
                 "\n"
                 "Options:\n"
                 "  -r            Recursive directory processing\n"
                 "  -l            List files only\n"
                 "  --size        Show file sizes in -l output\n"
                 "  --sorted      Sort list (-l) by size (desc)\n"
                 "  -n            Show line numbers\n"
                 "  --compact     Drop empty lines from file contents\n"
                 "  --abs         Show absolute paths\n"
                 "  --config F    Read patterns from file F\n"
                 "  --server P    Run HTTP server on port P\n"
                 "  --wrap DIR    Wrap collected files as HTML into DIR\n"
                 "  --prefix P    Prepend P before file paths in -l output\n"
                 "  --exclude P   Exclude paths matching glob pattern P "
                 "(argument mode)\n"
                 "                Shorthand: @PATTERN as positional argument\n"
                 "  --git-info    Print git commit hash and remote origin\n"
                 "  --ghmap       List raw.githubusercontent.com URLs for "
                 "current commit\n"
                 "  --copy        Copy all stdout output to system clipboard\n"
                 "  -e NAME       Edit config file NAME in ./.scatconfig/NAME\n"
                 "  --hook-install Install or update .git/hooks/pre-commit for "
                 "scat wrap\n"
                 "  -V, --version Show program version and exit\n"
                 "  -h, --help    Show this help\n"
                 "\n"
                 "If no paths are given, scat reads patterns from scat.txt.\n";
}

Options parse_options(int argc, char **argv)
{
    Options opt;

    auto add_include = [&](const std::string &pat) {
        opt.paths.push_back(pat);
        opt.arg_rules.push_back(Rule{pat, false});
    };

    auto add_exclude = [&](const std::string &pat) {
        opt.exclude_paths.push_back(pat);
        opt.arg_rules.push_back(Rule{pat, true});
    };

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "-r")
        {
            opt.recursive = true;
        }
        else if (a == "-l")
        {
            opt.list_only = true;
        }
        else if (a == "--size")
        {
            opt.show_size = true;
        }
        else if (a == "-n")
        {
            opt.line_numbers = true;
        }
        else if (a == "--compact")
        {
            opt.compact = true;
        }
        else if (a == "--abs")
        {
            opt.abs_paths = true;
        }
        else if (a == "--hook-install")
        {
            opt.hook_install = true;
        }
        else if (a == "--version" || a == "-V")
        {
            opt.show_version = true;
        }
        else if (a == "--sorted")
        {
            opt.sorted = true;
        }
        else if (a.rfind("--prefix=", 0) == 0)
        {
            opt.path_prefix = a.substr(std::string("--prefix=").size());
        }
        else if (a == "--prefix")
        {
            if (i + 1 < argc)
            {
                opt.path_prefix = argv[++i];
            }
            else
            {
                std::cerr << "--prefix requires value\n";
                std::exit(1);
            }
        }
        else if (a == "--server")
        {
            if (i + 1 < argc)
                opt.server_port = std::atoi(argv[++i]);
            else
            {
                std::cerr << "--server requires port\n";
                std::exit(1);
            }
        }
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
        else if (a == "--git-info")
        {
            opt.git_info = true;
        }
        else if (a == "--ghmap")
        {
            opt.gh_map = true;
        }
        else if (a == "--copy")
        {
            opt.copy_out = true;
        }
        else if (a == "-e")
        {
            if (i + 1 < argc)
            {
                opt.edit_config_name = argv[++i];
            }
            else
            {
                std::cerr << "-e requires filename\n";
                std::exit(1);
            }
        }
        else if (a == "-h" || a == "--help")
        {
            print_help();
            std::exit(0);
        }
        else if (a == "--wrap")
        {
            if (i + 1 < argc)
            {
                opt.wrap_root = argv[++i];
            }
            else
            {
                std::cerr << "--wrap requires directory name\n";
                std::exit(1);
            }
        }
        else if (a == "--exclude")
        {
            if (i + 1 < argc)
            {
                add_exclude(argv[++i]);
            }
            else
            {
                std::cerr << "--exclude requires pattern\n";
                std::exit(1);
            }
        }
        else if (a.rfind("--exclude=", 0) == 0)
        {
            add_exclude(a.substr(std::string("--exclude=").size()));
        }
        else if (!a.empty() && a[0] == '@')
        {
            if (a.size() == 1)
            {
                std::cerr << "@ requires pattern\n";
                std::exit(1);
            }
            add_exclude(a.substr(1));
        }
        else
        {
            add_include(a);
        }
    }

    // auto-config mode if no paths and no explicit config
    if (opt.arg_rules.empty() && opt.config_file.empty())
        opt.config_file = "scat.txt";

    return opt;
}

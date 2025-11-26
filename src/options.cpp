#include "options.h"
#include <cstdlib>
#include <iostream>

static void print_help() {
    std::cout <<
        "Usage: scat [options] [paths...]\n"
        "\n"
        "Options:\n"
        "  -r            Recursive directory processing\n"
        "  -l            List files only\n"
        "  --size        Show file sizes in -l output\n"
        "  --sorted      Sort list (-l) by size (desc)\n"
        "  -n            Show line numbers\n"
        "  --abs         Show absolute paths\n"
        "  --config F    Read patterns from file F\n"
        "  --apply F     Apply patch from file F\n"
        "  --apply-stdin Apply patch from stdin\n"
        "  --server P    Run HTTP server on port P\n"
        "  -c, --chunk   Print chunk trailer after output\n"
        "  --chunk-help  Show chunk v2 help\n"
        "  --wrap DIR    Wrap collected files as HTML into DIR\n"
        "  --prefix P    Prepend P before file paths in -l output\n"
        "  -h, --help    Show this help\n"
        "\n"
        "If no paths are given, scat reads patterns from scat.txt.\n";
}


Options parse_options(int argc, char** argv) {
    Options opt;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];

        if (a == "-r") {
            opt.recursive = true;
        } else if (a == "-l") {
            opt.list_only = true;
        } else if (a == "--size") {
            opt.show_size = true;
        } else if (a == "-n") {
            opt.line_numbers = true;
        } else if (a == "--abs") {
            opt.abs_paths = true;
        } else if (a == "--sorted") {
            opt.sorted = true;
        } else if (a.rfind("--prefix=", 0) == 0) {
            opt.path_prefix = a.substr(std::string("--prefix=").size());
        } else if (a == "--prefix") {
            if (i + 1 < argc) {
                opt.path_prefix = argv[++i];
            } else {
                std::cerr << "--prefix requires value\n";
                std::exit(1);
            }
        } else if (a == "--server") {
            if (i + 1 < argc) opt.server_port = std::atoi(argv[++i]);
            else {
                std::cerr << "--server requires port\n";
                std::exit(1);
            }
        } else if (a == "--chunk-help") {
            opt.chunk_help = true;
        } else if (a == "-c" || a == "--chunk") {
            opt.chunk_trailer = true;
        } else if (a == "--apply-stdin") {
            opt.apply_stdin = true;
        } else if (a == "--apply") {
            if (i + 1 < argc) opt.apply_file = argv[++i];
            else {
                std::cerr << "--apply requires file\n";
                std::exit(1);
            }
        } else if (a == "--config") {
            if (i + 1 < argc) opt.config_file = argv[++i];
            else {
                std::cerr << "--config requires file\n";
                std::exit(1);
            }
        } else if (a == "-h" || a == "--help") {
            print_help();
            std::exit(0);
        } else if (a == "--wrap") {
            if (i + 1 < argc) {
                opt.wrap_root = argv[++i];
            } else {
                std::cerr << "--wrap requires directory name\n";
                std::exit(1);
            }
        } else {
            opt.paths.push_back(a);
        }
    }

    // auto-config mode if no paths and no explicit config
    if (opt.paths.empty() && opt.config_file.empty())
        opt.config_file = "scat.txt";

    return opt;
}

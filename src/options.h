#pragma once
#include <string>
#include <vector>

struct Options
{
    bool recursive = false;
    bool list_only = false;
    bool abs_paths = false;
    bool line_numbers = false;
    bool sorted = false; // --sorted: sort list (-l) by size

    bool show_size = false; // --size: show file sizes in -l
    int server_port = 0;    // --server PORT
    std::string config_file; // empty = no config mode

    bool git_info = false; // --git-info: print git meta info
    bool gh_map = false;   // --ghmap: print GitHub raw URLs for current commit
    bool copy_out = false; // --copy: also send stdout to clipboard
    bool hook_install =
        false; // --hook-install: install/update git pre-commit hook

    std::vector<std::string> paths; // positional paths

    std::string wrap_root;   // --wrap
    std::string path_prefix; // --prefix P
};

Options parse_options(int argc, char **argv);

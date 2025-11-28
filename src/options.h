#pragma once
#include "rules.h"
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
    bool compact = false;   // --compact: remove empty lines from file output
    int server_port = 0;    // --server PORT
    std::string config_file; // empty = no config mode

    bool git_info = false; // --git-info: print git meta info
    bool gh_map = false;   // --ghmap: print GitHub raw URLs for current commit
    bool copy_out = false; // --copy: also send stdout to clipboard
    bool hook_install =
        false; // --hook-install: install/update git pre-commit hook
    bool show_version = false; // -V, --version: show version and exit

    std::vector<std::string> paths; // positional paths
    std::vector<std::string>
        exclude_paths; // --exclude P / @PATTERN (argument mode)
    std::string edit_config_name; // -e NAME: edit ./.scatconfig/NAME
    std::vector<Rule> arg_rules; // ordered CLI rules with excludes/includes

    std::string wrap_root;   // --wrap
    std::string path_prefix; // --prefix P
};

Options parse_options(int argc, char **argv);

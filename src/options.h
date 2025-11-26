#pragma once
#include <string>
#include <vector>

struct Options {
    bool chunk_trailer = false;
    bool recursive     = false;
    bool list_only     = false;
    bool abs_paths     = false;
    bool line_numbers  = false;
    bool sorted        = false;  // --sorted: sort list (-l) by size

    bool show_size     = false;  // --size: show file sizes in -l

    int  server_port   = 0;      // --server PORT

    std::string apply_file;   // empty = no apply mode
    bool        apply_stdin = false;

    std::string config_file;  // empty = no config mode
    bool        chunk_help = false;

    std::vector<std::string> paths; // positional paths

    // HTML wrapper root
    std::string wrap_root;   // if not empty, write wrapped files under this directory

    // prefix for -l output
    std::string path_prefix; // --prefix P : prepend P before file paths in -l output
};

Options parse_options(int argc, char** argv);
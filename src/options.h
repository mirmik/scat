#pragma once
#include <string>
#include <vector>

struct Options
{
    bool chunk_trailer = false;
    bool recursive = false;
    bool list_only = false;
    bool abs_paths = false;
    bool line_numbers = false;
    std::string apply_file;   // empty = no apply mode
    bool apply_stdin = false;
    std::string config_file;        // empty = no config mode
bool chunk_help = false;
    std::vector<std::string> paths; // positional paths
};

Options parse_options(int argc, char** argv);

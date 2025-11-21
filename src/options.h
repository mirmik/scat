#pragma once
#include <string>
#include <vector>

struct Options
{
    bool recursive = false;
    bool list_only = false;
    bool abs_paths = false;

    std::string config_file;        // empty = no config mode
    std::vector<std::string> paths; // positional paths
};

Options parse_options(int argc, char** argv);

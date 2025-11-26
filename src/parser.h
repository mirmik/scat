#pragma once
#include "rules.h"
#include <filesystem>
#include <string>
#include <vector>

struct Config
{
    std::vector<Rule> text_rules;
    std::vector<Rule> tree_rules;
    std::string map_format; // новый блок [MAPFORMAT], может быть пустым
};

Config parse_config(const std::filesystem::path &path);

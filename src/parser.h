#pragma once
#include <vector>
#include <filesystem>
#include "rules.h"

struct Config {
    std::vector<Rule> text_rules;
    std::vector<Rule> tree_rules;
};

Config parse_config(const std::filesystem::path& path);

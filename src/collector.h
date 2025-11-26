#pragma once
#include "options.h"
#include "rules.h"
#include <filesystem>
#include <vector>

std::vector<std::filesystem::path>
collect_from_rules(const std::vector<Rule> &rules, const Options &opt);

std::vector<std::filesystem::path>
collect_from_paths(const std::vector<std::string> &paths, const Options &opt);

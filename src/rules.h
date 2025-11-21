#pragma once
#include <string>
#include <vector>

struct Rule
{
    std::string pattern;
    bool exclude = false;
};

std::vector<Rule> load_rules(const std::string& file);

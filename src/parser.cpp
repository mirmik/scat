#include "parser.h"
#include <fstream>
#include <stdexcept>
#include <cctype>
#include "rules.h"

namespace fs = std::filesystem;

static inline void trim(std::string& s)
{
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) { s.clear(); return; }
    size_t e = s.find_last_not_of(" \t\r\n");
    s = s.substr(b, e - b + 1);
}

Config parse_config(const fs::path& path)
{
    Config cfg;

    enum Section { TEXT_RULES, TREE_RULES };
    Section current = TEXT_RULES;

    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Failed to open config file: " + path.string());

    std::string line;
    while (std::getline(in, line))
    {
        trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == "[TREE]")
        {
            current = TREE_RULES;
            continue;
        }

        Rule r = Rule::from_string(line);

        if (current == TEXT_RULES)
            cfg.text_rules.push_back(r);
        else
            cfg.tree_rules.push_back(r);
    }

    return cfg;
}

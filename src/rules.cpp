#include "rules.h"
#include <fstream>
#include <iostream>

std::vector<Rule> load_rules(const std::string& file)
{
    std::vector<Rule> out;

    std::ifstream in(file);
    if (!in.is_open())
    {
        std::cerr << "Config file not found: " << file << "\n";
        return out;
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        Rule r;
        if (line[0] == '!')
        {
            r.exclude = true;
            r.pattern = line.substr(1);
        }
        else
        {
            r.exclude = false;
            r.pattern = line;
        }

        out.push_back(r);
    }

    return out;
}

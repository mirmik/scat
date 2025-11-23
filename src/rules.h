#pragma once
#include <string>

struct Rule
{
    std::string pattern;
    bool exclude = false;

    static Rule from_string(const std::string& line)
    {
        Rule r;
        if (!line.empty() && line[0] == '!')
        {
            r.exclude = true;
            r.pattern = line.substr(1);
        }
        else
        {
            r.exclude = false;
            r.pattern = line;
        }
        return r;
    }
};
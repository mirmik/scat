#include "parser.h"
#include "rules.h"
#include <cctype>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

static inline void trim(std::string &s)
{
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos)
    {
        s.clear();
        return;
    }
    size_t e = s.find_last_not_of(" \t\r\n");
    s = s.substr(b, e - b + 1);
}

Config parse_config(const fs::path &path)
{
    Config cfg;
    enum Section
    {
        TEXT_RULES,
        TREE_RULES,
        MAPFORMAT_TEXT,
        VARIANT_TEXT
    };
    Section current = TEXT_RULES;
    std::string current_variant;

    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Failed to open config file: " +
                                 path.string());

    std::string raw_line;
    while (std::getline(in, raw_line))
    {
        std::string line = raw_line;
        trim(line);

        if (current == MAPFORMAT_TEXT)
        {
            // Всё, что после [MAPFORMAT], идёт как есть
            // (с сохранением пустых строк и #) до конца файла.
            cfg.map_format += raw_line;
            cfg.map_format += "\n";
            continue;
        }

        if (line.empty() || line[0] == '#')
            continue;

        // [TREE] — глобальные правила дерева
        if (line == "[TREE]")
        {
            current = TREE_RULES;
            current_variant.clear();
            continue;
        }

        // [MAPFORMAT] — текстовый шаблон карты, до конца файла
        if (line == "[MAPFORMAT]")
        {
            current = MAPFORMAT_TEXT;
            current_variant.clear();
            continue;
        }

        // [VAR(name)] — отдельный набор правил текстового вывода
        if (line.size() > 6 && line.rfind("[VAR(", 0) == 0 &&
            line.back() == ']')
        {
            std::size_t open = line.find('(');
            std::size_t close = line.rfind(')');
            if (open == std::string::npos ||
                close == std::string::npos || close <= open + 1)
            {
                throw std::runtime_error(
                    "Invalid [VAR(...)] section in config");
            }

            std::string name = line.substr(open + 1, close - open - 1);
            trim(name);
            if (name.empty())
            {
                throw std::runtime_error(
                    "Empty variant name in [VAR()]");
            }

            current = VARIANT_TEXT;
            current_variant = name;
            // гарантируем наличие ключа даже для пустой секции
            cfg.variants[current_variant];
            continue;
        }

        Rule r = Rule::from_string(line);

        switch (current)
        {
        case TEXT_RULES:
            cfg.text_rules.push_back(r);
            break;
        case TREE_RULES:
            cfg.tree_rules.push_back(r);
            break;
        case VARIANT_TEXT:
            cfg.variants[current_variant].push_back(r);
            break;
        case MAPFORMAT_TEXT:
            // сюда не попадём: MAPFORMAT обрабатывается выше
            break;
        }
    }

    return cfg;
}

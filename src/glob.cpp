#include "glob.h"
#include "util.h"
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

namespace fs = std::filesystem;

std::vector<fs::path> expand_glob(const std::string& pattern)
{
    std::vector<fs::path> out;
    std::error_code ec;

    // Новый разбор пути с поддержкой абсолютных путей
    fs::path pat(pattern);
    fs::path root = pat.root_path();      // "/" или "C:\", или "" для относительного
    fs::path rel  = pat.relative_path();  // остальная часть

    // Разбиваем rel на части
    std::vector<std::string> parts;
    {
        std::stringstream ss(rel.generic_string());
        std::string seg;
        while (std::getline(ss, seg, '/'))
            parts.push_back(seg);
    }

    // Рекурсивный обход
    std::function<void(const fs::path&, size_t)> walk =
        [&](const fs::path& base, size_t idx)
    {
        if (idx == parts.size())
        {
            if (fs::is_regular_file(base, ec))
                out.push_back(base);
            return;
        }

        const std::string& seg = parts[idx];

        if (seg == "**")
        {
            // zero levels
            walk(base, idx + 1);

            // one or more
            if (fs::is_directory(base, ec))
            {
                for (auto& e : fs::directory_iterator(base, ec))
                    walk(e.path(), idx);
            }
        }
        else if (seg.find('*') != std::string::npos)
        {
            if (!fs::is_directory(base, ec))
                return;

            for (auto& e : fs::directory_iterator(base, ec))
            {
                if (match_simple(e.path(), seg))
                    walk(e.path(), idx + 1);
            }
        }
        else
        {
            fs::path next = base / seg;
            if (fs::exists(next, ec))
                walk(next, idx + 1);
        }
    };

     // Статический префикс до первого wildcard
    size_t start = 0;

    if (root.empty())
        root = ".";  // относительный путь

    for (; start < parts.size(); ++start)
    {
        const auto& seg = parts[start];
        if (seg == "**" || seg.find('*') != std::string::npos)
            break;

        root /= seg; // корректно: "/" + "tmp" → "/tmp"
    }

    if (!fs::exists(root, ec))
        return out;

    walk(root, start);

    // Дубликаты
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return out;
}

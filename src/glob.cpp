#include "glob.h"
#include "util.h"
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

namespace fs = std::filesystem;

static bool is_wildcard(const std::string& s) {
    return s == "*" || s == "**" || s.find('*') != std::string::npos;
}

// matcit только filename, НЕ директорию
static bool match_name(const fs::path& p, const std::string& seg)
{
    return match_simple(p, seg);
}

std::vector<fs::path> expand_glob(const std::string& pattern)
{
    std::vector<fs::path> out;
    std::error_code ec;

    fs::path pat(pattern);
    fs::path root = pat.root_path();      // "/" или "C:" или ""
    fs::path rel  = pat.relative_path();

    if (root.empty())
        root = ".";

    // Разбиваем на части
    std::vector<std::string> parts;
    {
        std::stringstream ss(rel.generic_string());
        std::string seg;
        while (std::getline(ss, seg, '/'))
            parts.push_back(seg);
    }

    // Определяем статический префикс (до первой звёздочки)
    size_t start = 0;
    for (; start < parts.size(); ++start)
    {
        const auto& seg = parts[start];
        if (is_wildcard(seg))
            break;
        root /= seg;
    }

    if (!fs::exists(root, ec))
        return out;

    // Основной обход
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

        // ---------------------------------------------------------------------
        // "**" → полный рекурсивный обход
        // ---------------------------------------------------------------------
        if (seg == "**")
        {
            // вариант 0 уровней
            walk(base, idx + 1);

            // вариант 1+ уровней
            if (fs::is_directory(base, ec))
            {
                for (auto& e : fs::directory_iterator(base, ec))
                    walk(e.path(), idx); // двигаемся дальше, idx НЕ увеличиваем
            }
            return;
        }

        // ---------------------------------------------------------------------
        // "*" или "ma*sk" → только ОДИН уровень, без рекурсии
        // ---------------------------------------------------------------------
        if (seg.find('*') != std::string::npos)
        {
            if (!fs::is_directory(base, ec))
                return;

            for (auto& e : fs::directory_iterator(base, ec))
            {
                if (!match_name(e.path(), seg))
                    continue;

                // если это последний сегмент — принимаем только файлы
                if (idx + 1 == parts.size())
                {
                    if (e.is_regular_file())
                        out.push_back(e.path());
                }
                else
                {
                    walk(e.path(), idx + 1);
                }
            }
            return;
        }

        // ---------------------------------------------------------------------
        // обычное имя → просто переходим
        // ---------------------------------------------------------------------
        fs::path next = base / seg;
        if (fs::exists(next, ec))
            walk(next, idx + 1);
    };

    walk(root, start);

    // удалить дубликаты
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return out;
}

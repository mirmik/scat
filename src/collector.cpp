#include "collector.h"
#include "util.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <filesystem>
#include <sstream>
#include <functional>

namespace fs = std::filesystem;

// ---------------------------------------------------------------
// Вспомогалки для glob
// ---------------------------------------------------------------

// pattern like:  "src/*"  or  "datas/**"
static bool has_double_star(const std::string& s)
{
    return s.find("**") != std::string::npos;
}

static bool has_single_star(const std::string& s)
{
    return s.find('*') != std::string::npos && !has_double_star(s);
}

// Расширение одного правила
static void expand_rule(const Rule& r, std::vector<fs::path>& out)
{
    const std::string& pat = r.pattern;
    std::error_code ec;

    // ------------------------------------------------------------------
    // NEW REAL GLOB: supports patterns like:
    //   src/**/*.cpp
    //   foo/*/bar/**/test_*.h
    // ------------------------------------------------------------------
    {
        std::vector<std::string> parts;
        {
            std::stringstream ss(pat);
            std::string seg;
            while (std::getline(ss, seg, '/'))
                parts.push_back(seg);
        }

        // recursive matcher for filesystem::path
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
                // 1) match zero levels
                walk(base, idx + 1);

                // 2) match one or more levels
                if (fs::is_directory(base, ec))
                {
                    for (auto& e : fs::directory_iterator(base, ec))
                    {
                        walk(e.path(), idx);
                    }
                }
            }
            else if (seg.find('*') != std::string::npos)
            {
                // wildcard * inside segment
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
                // literal match
                fs::path next = base / seg;
                if (fs::exists(next, ec))
                    walk(next, idx + 1);
            }
        };

        fs::path root;
        size_t start = 0;

        // figure out static prefix before any wildcard
        for (; start < parts.size(); ++start)
        {
            if (parts[start] == "**" || parts[start].find('*') != std::string::npos)
                break;
            root /= parts[start];
        }

        if (root.empty())
            root = ".";

        if (!fs::exists(root, ec))
            return;

        walk(root, start);
        return;
    }

    // ==== * (one level) ====
    if (has_single_star(pat))
    {
        fs::path dir = fs::path(pat).parent_path();
        std::string mask = fs::path(pat).filename().string();

        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))
            return;

        for (auto& e : fs::directory_iterator(dir, ec))
        {
            if (e.is_regular_file() && match_simple(e.path(), mask))
                out.push_back(e.path());
        }
        return;
    }

    // ==== Прямой путь ====
    fs::path p = pat;

    if (fs::is_regular_file(p, ec))
    {
        out.push_back(fs::canonical(p, ec));
        return;
    }

    if (fs::is_directory(p, ec))
    {
        for (auto& e : fs::directory_iterator(p, ec))
            if (e.is_regular_file())
                out.push_back(e.path());
        return;
    }
}

// ---------------------------------------------------------------
// collect_from_rules()
// ---------------------------------------------------------------

std::vector<fs::path> collect_from_rules(const std::vector<Rule>& rules, const Options& opt)
{
    std::vector<fs::path> tmp;
    std::error_code ec;

    // 1. Собираем все include-рулы
    for (const auto& r : rules)
        if (!r.exclude)
            expand_rule(r, tmp);

    // 2. Применяем exclude-рулы
    for (const auto& r : rules)
    {
        if (!r.exclude)
            continue;

        tmp.erase(std::remove_if(tmp.begin(), tmp.end(),
                                 [&](const fs::path& p) {
                                     // исключаем по относительному пути
                                     std::string rel = make_display_path(p);
                                     return rel.rfind(r.pattern, 0) == 0; // starts_with
                                 }),
                  tmp.end());
    }

    // 3. Убираем дубликаты
    std::sort(tmp.begin(), tmp.end());
    tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());

    return tmp;
}

// ---------------------------------------------------------------
// collect_from_paths()
// ---------------------------------------------------------------

std::vector<fs::path> collect_from_paths(const std::vector<std::string>& paths, const Options& opt)
{
    std::vector<fs::path> out;

    for (auto& s : paths)
    {
        fs::path p = s;
        std::error_code ec;

        if (!fs::exists(p, ec))
        {
            std::cerr << "Not found: " << p << "\n";
            continue;
        }

        if (fs::is_regular_file(p, ec))
        {
            out.push_back(fs::canonical(p, ec));
            continue;
        }

        if (fs::is_directory(p, ec))
        {
            if (opt.recursive)
            {
                for (auto& e : fs::recursive_directory_iterator(p, ec))
                    if (e.is_regular_file())
                        out.push_back(e.path());
            }
            else
            {
                for (auto& e : fs::directory_iterator(p, ec))
                    if (e.is_regular_file())
                        out.push_back(e.path());
            }
        }
    }

    // Убираем дубликаты
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return out;
}

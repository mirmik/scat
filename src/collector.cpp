#include "collector.h"
#include "util.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>

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

    // ==== ** (recursive) ====
    if (has_double_star(pat))
    {
        // Берём часть до **
        auto pos = pat.find("**");
        fs::path base = pat.substr(0, pos);

        if (!fs::exists(base, ec) || !fs::is_directory(base, ec))
            return;

        for (auto& e : fs::recursive_directory_iterator(base, ec))
        {
            if (e.is_regular_file())
                out.push_back(e.path());
        }
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

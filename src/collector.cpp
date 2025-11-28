#include "collector.h"
#include "glob.h"
#include "util.h"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <unordered_set>

namespace fs = std::filesystem;

// Строковый ключ для сравнения путей и выбрасывания дублей.
static std::string make_abs_key(const fs::path &p)
{
    std::error_code ec;
    fs::path abs = fs::absolute(p, ec);
    if (!ec)
        return abs.lexically_normal().string();

    return p.lexically_normal().string();
}

// Раскрывает правило из конфигурации (всегда через glob).
static std::vector<fs::path> expand_rule(const Rule &r)
{
    return expand_glob(r.pattern);
}

static void apply_include(std::vector<fs::path> &out,
                          std::unordered_set<std::string> &present,
                          const std::vector<fs::path> &expanded)
{
    for (auto &p : expanded)
    {
        std::string key = make_abs_key(p);
        if (present.insert(key).second)
            out.push_back(p);
    }
}

static void apply_exclude(std::vector<fs::path> &out,
                          std::unordered_set<std::string> &present,
                          const std::vector<fs::path> &expanded)
{
    if (expanded.empty())
        return;

    std::unordered_set<std::string> bad;
    bad.reserve(expanded.size() * 2);

    for (auto &p : expanded)
        bad.insert(make_abs_key(p));

    out.erase(std::remove_if(out.begin(),
                             out.end(),
                             [&](const fs::path &p) {
                                 std::string key = make_abs_key(p);
                                 if (bad.find(key) != bad.end())
                                 {
                                     present.erase(key);
                                     return true;
                                 }
                                 return false;
                             }),
              out.end());
}

static std::vector<fs::path>
collect_with_rules(const std::vector<Rule> &rules,
                   const std::function<std::vector<fs::path>(const Rule &)>
                       &expander)
{
    std::vector<fs::path> out;
    std::unordered_set<std::string> present;
    present.reserve(rules.size() * 4);

    for (const auto &r : rules)
    {
        auto expanded = expander(r);
        if (r.exclude)
            apply_exclude(out, present, expanded);
        else
            apply_include(out, present, expanded);
    }

    return out;
}

// ---------------------------------------------------------------
// collect_from_rules() — config mode
// ---------------------------------------------------------------

std::vector<fs::path> collect_from_rules(const std::vector<Rule> &rules,
                                         const Options &opt)
{
    (void)opt;
    return collect_with_rules(rules, expand_rule);
}

// ---------------------------------------------------------------
// CLI helpers
// ---------------------------------------------------------------

static std::vector<fs::path>
expand_cli_include(const std::string &s, const Options &opt)
{
    std::vector<fs::path> out;
    std::error_code ec;

    // glob pattern
    if (s.find('*') != std::string::npos)
    {
        auto v = expand_glob(s);
        if (v.empty())
        {
            std::cerr << "Not found: " << s << "\n";
            return out;
        }

        out.insert(out.end(), v.begin(), v.end());
        return out;
    }

    fs::path p = s;

    if (!fs::exists(p, ec))
    {
        std::cerr << "Not found: " << p << "\n";
        return out;
    }

    if (fs::is_regular_file(p, ec))
    {
        out.push_back(fs::canonical(p, ec));
        return out;
    }

    if (fs::is_directory(p, ec))
    {
        if (opt.recursive)
        {
            for (auto &e : fs::recursive_directory_iterator(p, ec))
                if (e.is_regular_file())
                    out.push_back(e.path());
        }
        else
        {
            for (auto &e : fs::directory_iterator(p, ec))
                if (e.is_regular_file())
                    out.push_back(e.path());
        }
    }

    return out;
}

// ---------------------------------------------------------------
// collect_from_paths* — CLI mode
// ---------------------------------------------------------------

std::vector<fs::path> collect_from_paths(const std::vector<std::string> &paths,
                                         const Options &opt)
{
    std::vector<Rule> rules;
    rules.reserve(paths.size());
    for (const auto &p : paths)
        rules.push_back(Rule{p, false});

    return collect_from_paths_with_excludes(rules, opt);
}

std::vector<fs::path>
collect_from_paths_with_excludes(const std::vector<Rule> &rules,
                                 const Options &opt)
{
    auto expand_cli_rule = [&](const Rule &r) {
        if (r.exclude)
            return expand_glob(r.pattern);
        return expand_cli_include(r.pattern, opt);
    };

    return collect_with_rules(rules, expand_cli_rule);
}

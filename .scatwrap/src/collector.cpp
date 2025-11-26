<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.cpp</title>
</head>
<body>
<pre><code>
#include &quot;collector.h&quot;
#include &quot;glob.h&quot;
#include &quot;util.h&quot;
#include &lt;algorithm&gt;
#include &lt;filesystem&gt;
#include &lt;functional&gt;
#include &lt;iostream&gt;
#include &lt;sstream&gt;
#include &lt;unordered_set&gt;

namespace fs = std::filesystem;

// ---------------------------------------------------------------
// Вспомогалки для glob
// ---------------------------------------------------------------

// pattern like:  &quot;src/*&quot;  or  &quot;datas/**&quot;
static bool has_double_star(const std::string &amp;s)
{
    return s.find(&quot;**&quot;) != std::string::npos;
}

static bool has_single_star(const std::string &amp;s)
{
    return s.find('*') != std::string::npos &amp;&amp; !has_double_star(s);
}

// Расширение одного правила
static void expand_rule(const Rule &amp;r, std::vector&lt;fs::path&gt; &amp;out)
{
    const std::string &amp;pat = r.pattern;
    std::error_code ec;

    // ------------------------------------------------------------------
    // новый glob — вынесен в glob.cpp
    {
        auto v = expand_glob(pat);
        out.insert(out.end(), v.begin(), v.end());
        return;
    }

    // ==== * (one level) ====
    if (has_single_star(pat))
    {
        fs::path dir = fs::path(pat).parent_path();
        std::string mask = fs::path(pat).filename().string();

        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))
            return;

        for (auto &amp;e : fs::directory_iterator(dir, ec))
        {
            if (e.is_regular_file() &amp;&amp; match_simple(e.path(), mask))
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
        for (auto &amp;e : fs::directory_iterator(p, ec))
            if (e.is_regular_file())
                out.push_back(e.path());
        return;
    }
}

// ---------------------------------------------------------------
// collect_from_rules()
// ---------------------------------------------------------------

std::vector&lt;fs::path&gt; collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules,
                                         const Options &amp;opt)
{
    std::vector&lt;fs::path&gt; tmp;
    std::error_code ec;

    // 1. Собираем все include-рулы
    for (const auto &amp;r : rules)
        if (!r.exclude)
            expand_rule(r, tmp);

    // 2. Применяем exclude-рулы через нормальный glob
    for (const auto &amp;r : rules)
    {
        if (!r.exclude)
            continue;

        auto bad = expand_glob(r.pattern);

        std::unordered_set&lt;std::string&gt; bad_abs;
        bad_abs.reserve(bad.size());

        for (auto &amp;b : bad)
        {
            std::error_code ec;
            auto absb = fs::absolute(b, ec);
            bad_abs.insert(absb.string());
        }

        tmp.erase(std::remove_if(tmp.begin(),
                                 tmp.end(),
                                 [&amp;](const fs::path &amp;p)
                                 {
                                     std::error_code ec;
                                     auto absp = fs::absolute(p, ec);
                                     return bad_abs.find(absp.string()) !=
                                            bad_abs.end();
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

std::vector&lt;fs::path&gt; collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths,
                                         const Options &amp;opt)
{
    std::vector&lt;fs::path&gt; out;

    for (auto &amp;s : paths)
    {
        fs::path p = s;
        std::error_code ec;

        if (!fs::exists(p, ec))
        {
            std::cerr &lt;&lt; &quot;Not found: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;
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
                for (auto &amp;e : fs::recursive_directory_iterator(p, ec))
                    if (e.is_regular_file())
                        out.push_back(e.path());
            }
            else
            {
                for (auto &amp;e : fs::directory_iterator(p, ec))
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

</code></pre>
</body>
</html>

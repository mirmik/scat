<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;collector.h&quot;<br>
#include &quot;glob.h&quot;<br>
#include &quot;util.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;functional&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;unordered_set&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
// ---------------------------------------------------------------<br>
// Вспомогалки для glob<br>
// ---------------------------------------------------------------<br>
<br>
// pattern like:  &quot;src/*&quot;  or  &quot;datas/**&quot;<br>
static bool has_double_star(const std::string &amp;s)<br>
{<br>
    return s.find(&quot;**&quot;) != std::string::npos;<br>
}<br>
<br>
static bool has_single_star(const std::string &amp;s)<br>
{<br>
    return s.find('*') != std::string::npos &amp;&amp; !has_double_star(s);<br>
}<br>
<br>
// Расширение одного правила<br>
static void expand_rule(const Rule &amp;r, std::vector&lt;fs::path&gt; &amp;out)<br>
{<br>
    const std::string &amp;pat = r.pattern;<br>
    std::error_code ec;<br>
<br>
    // ------------------------------------------------------------------<br>
    // новый glob — вынесен в glob.cpp<br>
    {<br>
        auto v = expand_glob(pat);<br>
        out.insert(out.end(), v.begin(), v.end());<br>
        return;<br>
    }<br>
<br>
    // ==== * (one level) ====<br>
    if (has_single_star(pat))<br>
    {<br>
        fs::path dir = fs::path(pat).parent_path();<br>
        std::string mask = fs::path(pat).filename().string();<br>
<br>
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))<br>
            return;<br>
<br>
        for (auto &amp;e : fs::directory_iterator(dir, ec))<br>
        {<br>
            if (e.is_regular_file() &amp;&amp; match_simple(e.path(), mask))<br>
                out.push_back(e.path());<br>
        }<br>
        return;<br>
    }<br>
<br>
    // ==== Прямой путь ====<br>
    fs::path p = pat;<br>
<br>
    if (fs::is_regular_file(p, ec))<br>
    {<br>
        out.push_back(fs::canonical(p, ec));<br>
        return;<br>
    }<br>
<br>
    if (fs::is_directory(p, ec))<br>
    {<br>
        for (auto &amp;e : fs::directory_iterator(p, ec))<br>
            if (e.is_regular_file())<br>
                out.push_back(e.path());<br>
        return;<br>
    }<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_rules()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules,<br>
                                         const Options &amp;opt)<br>
{<br>
    std::vector&lt;fs::path&gt; tmp;<br>
    std::error_code ec;<br>
<br>
    // 1. Собираем все include-рулы<br>
    for (const auto &amp;r : rules)<br>
        if (!r.exclude)<br>
            expand_rule(r, tmp);<br>
<br>
    // 2. Применяем exclude-рулы через нормальный glob<br>
    for (const auto &amp;r : rules)<br>
    {<br>
        if (!r.exclude)<br>
            continue;<br>
<br>
        auto bad = expand_glob(r.pattern);<br>
<br>
        std::unordered_set&lt;std::string&gt; bad_abs;<br>
        bad_abs.reserve(bad.size());<br>
<br>
        for (auto &amp;b : bad)<br>
        {<br>
            std::error_code ec;<br>
            auto absb = fs::absolute(b, ec);<br>
            bad_abs.insert(absb.string());<br>
        }<br>
<br>
        tmp.erase(std::remove_if(tmp.begin(),<br>
                                 tmp.end(),<br>
                                 [&amp;](const fs::path &amp;p)<br>
                                 {<br>
                                     std::error_code ec;<br>
                                     auto absp = fs::absolute(p, ec);<br>
                                     return bad_abs.find(absp.string()) !=<br>
                                            bad_abs.end();<br>
                                 }),<br>
                  tmp.end());<br>
    }<br>
<br>
    // 3. Убираем дубликаты<br>
    std::sort(tmp.begin(), tmp.end());<br>
    tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());<br>
<br>
    return tmp;<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_paths()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths,<br>
                                         const Options &amp;opt)<br>
{<br>
    std::vector&lt;fs::path&gt; out;<br>
<br>
    for (auto &amp;s : paths)<br>
    {<br>
        fs::path p = s;<br>
        std::error_code ec;<br>
<br>
        if (!fs::exists(p, ec))<br>
        {<br>
            std::cerr &lt;&lt; &quot;Not found: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
            continue;<br>
        }<br>
<br>
        if (fs::is_regular_file(p, ec))<br>
        {<br>
            out.push_back(fs::canonical(p, ec));<br>
            continue;<br>
        }<br>
<br>
        if (fs::is_directory(p, ec))<br>
        {<br>
            if (opt.recursive)<br>
            {<br>
                for (auto &amp;e : fs::recursive_directory_iterator(p, ec))<br>
                    if (e.is_regular_file())<br>
                        out.push_back(e.path());<br>
            }<br>
            else<br>
            {<br>
                for (auto &amp;e : fs::directory_iterator(p, ec))<br>
                    if (e.is_regular_file())<br>
                        out.push_back(e.path());<br>
            }<br>
        }<br>
    }<br>
<br>
    // Убираем дубликаты<br>
    std::sort(out.begin(), out.end());<br>
    out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
    return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

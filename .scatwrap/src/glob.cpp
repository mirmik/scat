<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;glob.h&quot;<br>
#include &quot;util.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;functional&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
static bool is_wildcard(const std::string &amp;s)<br>
{<br>
    return s == &quot;*&quot; || s == &quot;**&quot; || s.find('*') != std::string::npos;<br>
}<br>
<br>
// matcit только filename, НЕ директорию<br>
static bool match_name(const fs::path &amp;p, const std::string &amp;seg)<br>
{<br>
    return match_simple(p, seg);<br>
}<br>
<br>
std::vector&lt;fs::path&gt; expand_glob(const std::string &amp;pattern)<br>
{<br>
    std::vector&lt;fs::path&gt; out;<br>
    std::error_code ec;<br>
<br>
    fs::path pat(pattern);<br>
    fs::path root = pat.root_path(); // &quot;/&quot; или &quot;C:&quot; или &quot;&quot;<br>
    fs::path rel = pat.relative_path();<br>
<br>
    if (root.empty())<br>
        root = &quot;.&quot;;<br>
<br>
    // Разбиваем на части<br>
    std::vector&lt;std::string&gt; parts;<br>
    {<br>
        std::stringstream ss(rel.generic_string());<br>
        std::string seg;<br>
        while (std::getline(ss, seg, '/'))<br>
            parts.push_back(seg);<br>
    }<br>
<br>
    // Определяем статический префикс (до первой звёздочки)<br>
    size_t start = 0;<br>
    for (; start &lt; parts.size(); ++start)<br>
    {<br>
        const auto &amp;seg = parts[start];<br>
        if (is_wildcard(seg))<br>
            break;<br>
        root /= seg;<br>
    }<br>
<br>
    if (!fs::exists(root, ec))<br>
        return out;<br>
<br>
    // Основной обход<br>
    std::function&lt;void(const fs::path &amp;, size_t)&gt; walk =<br>
        [&amp;](const fs::path &amp;base, size_t idx)<br>
    {<br>
        if (idx == parts.size())<br>
        {<br>
            if (fs::is_regular_file(base, ec))<br>
                out.push_back(base);<br>
            return;<br>
        }<br>
<br>
        const std::string &amp;seg = parts[idx];<br>
<br>
        // ---------------------------------------------------------------------<br>
        // &quot;**&quot; → полный рекурсивный обход<br>
        // ---------------------------------------------------------------------<br>
        if (seg == &quot;**&quot;)<br>
        {<br>
            // вариант 0 уровней<br>
            walk(base, idx + 1);<br>
<br>
            // вариант 1+ уровней<br>
            if (fs::is_directory(base, ec))<br>
            {<br>
                for (auto &amp;e : fs::directory_iterator(base, ec))<br>
                    walk(e.path(), idx); // двигаемся дальше, idx НЕ увеличиваем<br>
            }<br>
            return;<br>
        }<br>
<br>
        // ---------------------------------------------------------------------<br>
        // &quot;*&quot; или &quot;ma*sk&quot; → только ОДИН уровень, без рекурсии<br>
        // ---------------------------------------------------------------------<br>
        if (seg.find('*') != std::string::npos)<br>
        {<br>
            if (!fs::is_directory(base, ec))<br>
                return;<br>
<br>
            for (auto &amp;e : fs::directory_iterator(base, ec))<br>
            {<br>
                if (!match_name(e.path(), seg))<br>
                    continue;<br>
<br>
                // если это последний сегмент — принимаем только файлы<br>
                if (idx + 1 == parts.size())<br>
                {<br>
                    if (e.is_regular_file())<br>
                        out.push_back(e.path());<br>
                }<br>
                else<br>
                {<br>
                    walk(e.path(), idx + 1);<br>
                }<br>
            }<br>
            return;<br>
        }<br>
<br>
        // ---------------------------------------------------------------------<br>
        // обычное имя → просто переходим<br>
        // ---------------------------------------------------------------------<br>
        fs::path next = base / seg;<br>
        if (fs::exists(next, ec))<br>
            walk(next, idx + 1);<br>
    };<br>
<br>
    walk(root, start);<br>
<br>
    // удалить дубликаты<br>
    std::sort(out.begin(), out.end());<br>
    out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
    return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

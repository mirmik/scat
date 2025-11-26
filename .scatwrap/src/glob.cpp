<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.cpp</title>
</head>
<body>
<pre><code>
#include &quot;glob.h&quot;
#include &quot;util.h&quot;
#include &lt;algorithm&gt;
#include &lt;filesystem&gt;
#include &lt;functional&gt;
#include &lt;sstream&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

namespace fs = std::filesystem;

static bool is_wildcard(const std::string &amp;s)
{
    return s == &quot;*&quot; || s == &quot;**&quot; || s.find('*') != std::string::npos;
}

// matcit только filename, НЕ директорию
static bool match_name(const fs::path &amp;p, const std::string &amp;seg)
{
    return match_simple(p, seg);
}

std::vector&lt;fs::path&gt; expand_glob(const std::string &amp;pattern)
{
    std::vector&lt;fs::path&gt; out;
    std::error_code ec;

    fs::path pat(pattern);
    fs::path root = pat.root_path(); // &quot;/&quot; или &quot;C:&quot; или &quot;&quot;
    fs::path rel = pat.relative_path();

    if (root.empty())
        root = &quot;.&quot;;

    // Разбиваем на части
    std::vector&lt;std::string&gt; parts;
    {
        std::stringstream ss(rel.generic_string());
        std::string seg;
        while (std::getline(ss, seg, '/'))
            parts.push_back(seg);
    }

    // Определяем статический префикс (до первой звёздочки)
    size_t start = 0;
    for (; start &lt; parts.size(); ++start)
    {
        const auto &amp;seg = parts[start];
        if (is_wildcard(seg))
            break;
        root /= seg;
    }

    if (!fs::exists(root, ec))
        return out;

    // Основной обход
    std::function&lt;void(const fs::path &amp;, size_t)&gt; walk =
        [&amp;](const fs::path &amp;base, size_t idx)
    {
        if (idx == parts.size())
        {
            if (fs::is_regular_file(base, ec))
                out.push_back(base);
            return;
        }

        const std::string &amp;seg = parts[idx];

        // ---------------------------------------------------------------------
        // &quot;**&quot; → полный рекурсивный обход
        // ---------------------------------------------------------------------
        if (seg == &quot;**&quot;)
        {
            // вариант 0 уровней
            walk(base, idx + 1);

            // вариант 1+ уровней
            if (fs::is_directory(base, ec))
            {
                for (auto &amp;e : fs::directory_iterator(base, ec))
                    walk(e.path(), idx); // двигаемся дальше, idx НЕ увеличиваем
            }
            return;
        }

        // ---------------------------------------------------------------------
        // &quot;*&quot; или &quot;ma*sk&quot; → только ОДИН уровень, без рекурсии
        // ---------------------------------------------------------------------
        if (seg.find('*') != std::string::npos)
        {
            if (!fs::is_directory(base, ec))
                return;

            for (auto &amp;e : fs::directory_iterator(base, ec))
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

</code></pre>
</body>
</html>

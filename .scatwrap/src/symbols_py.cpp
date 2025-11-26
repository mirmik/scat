<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/symbols_py.cpp</title>
</head>
<body>
<pre><code>
#include &quot;symbols.h&quot;

#include &lt;cctype&gt;

PythonSymbolFinder::PythonSymbolFinder(const std::string&amp; text)
{
    m_lines.clear();
    std::string current;
    current.reserve(80);

    for (char c : text)
    {
        if (c == '\n')
        {
            m_lines.push_back(current);
            current.clear();
        }
        else if (c != '\r')
        {
            current.push_back(c);
        }
    }
    m_lines.push_back(current);
}

int PythonSymbolFinder::calc_indent(const std::string&amp; line)
{
    int indent = 0;
    for (char c : line)
    {
        if (c == ' ')
            ++indent;
        else if (c == '\t')
            indent += 4; // грубая оценка, но устойчиво для сравнения
        else
            break;
    }
    return indent;
}

std::size_t PythonSymbolFinder::first_code_pos(const std::string&amp; line)
{
    std::size_t i = 0;
    while (i &lt; line.size() &amp;&amp; (line[i] == ' ' || line[i] == '\t'))
        ++i;
    return i;
}

bool PythonSymbolFinder::find_class_internal(const std::string&amp; class_name,
                                             Region&amp; out,
                                             int&amp; class_indent) const
{
    const int n = static_cast&lt;int&gt;(m_lines.size());
    for (int i = 0; i &lt; n; ++i)
    {
        const std::string&amp; line = m_lines[static_cast&lt;std::size_t&gt;(i)];
        std::size_t pos = first_code_pos(line);
        if (pos &gt;= line.size())
            continue;

        // комментарии / shebang
        if (line[pos] == '#')
            continue;

        if (line.compare(pos, 5, &quot;class&quot;) != 0)
            continue;

        char after = (pos + 5 &lt; line.size()) ? line[pos + 5] : '\0';
        if (!(after == '\0' || std::isspace(static_cast&lt;unsigned char&gt;(after)) ||
              after == '(' || after == ':'))
            continue;

        std::size_t p = pos + 5;
        while (p &lt; line.size() &amp;&amp; std::isspace(static_cast&lt;unsigned char&gt;(line[p])))
            ++p;

        std::size_t name_start = p;
        while (p &lt; line.size() &amp;&amp;
               (std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) || line[p] == '_'))
            ++p;

        if (name_start == p)
            continue;

        std::string name = line.substr(name_start, p - name_start);
        if (name != class_name)
            continue;

        class_indent = calc_indent(line);
        int last_body = i;

        for (int k = i + 1; k &lt; n; ++k)
        {
            const std::string&amp; l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];
            std::size_t pos2 = first_code_pos(l2);
            if (pos2 &gt;= l2.size())
                continue; // пустая строка в теле

            int ind2 = calc_indent(l2);
            if (ind2 &lt;= class_indent)
                break; // dedent — выходим из класса

            last_body = k;
        }

        out.start_line = i;
        out.end_line = last_body;
        return true;
    }

    return false;
}

bool PythonSymbolFinder::find_class(const std::string&amp; class_name, Region&amp; out) const
{
    int indent = 0;
    return find_class_internal(class_name, out, indent);
}

bool PythonSymbolFinder::find_method(const std::string&amp; class_name,
                                     const std::string&amp; method_name,
                                     Region&amp; out) const
{
    Region class_region;
    int class_indent = 0;
    if (!find_class_internal(class_name, class_region, class_indent))
        return false;

    const int start = class_region.start_line;
    const int end = class_region.end_line;

    // Определяем базовый уровень отступа для членов класса
    int member_indent = -1;
    for (int i = start + 1; i &lt;= end; ++i)
    {
        const std::string&amp; line = m_lines[static_cast&lt;std::size_t&gt;(i)];
        std::size_t pos = first_code_pos(line);
        if (pos &gt;= line.size())
            continue;
        if (line[pos] == '#')
            continue;

        int ind = calc_indent(line);
        if (ind &lt;= class_indent)
            continue;

        member_indent = ind;
        break;
    }

    if (member_indent &lt; 0)
        return false; // пустой класс

    const int n = static_cast&lt;int&gt;(m_lines.size());

    for (int i = start + 1; i &lt;= end &amp;&amp; i &lt; n; ++i)
    {
        const std::string&amp; line = m_lines[static_cast&lt;std::size_t&gt;(i)];
        std::size_t pos = first_code_pos(line);
        if (pos &gt;= line.size())
            continue;

        int ind = calc_indent(line);
        if (ind != member_indent)
            continue;

        if (line[pos] == '#')
            continue;

        // def / async def
        std::size_t p = pos;
        bool is_async = false;

        if (line.compare(p, 5, &quot;async&quot;) == 0 &amp;&amp;
            (p + 5 &gt;= line.size() || std::isspace(static_cast&lt;unsigned char&gt;(line[p + 5]))))
        {
            is_async = true;
            p += 5;
            while (p &lt; line.size() &amp;&amp; std::isspace(static_cast&lt;unsigned char&gt;(line[p])))
                ++p;
        }

        if (line.compare(p, 3, &quot;def&quot;) != 0 ||
            (p + 3 &lt; line.size() &amp;&amp;
             !std::isspace(static_cast&lt;unsigned char&gt;(line[p + 3]))))
        {
            continue;
        }

        p += 3;
        while (p &lt; line.size() &amp;&amp; std::isspace(static_cast&lt;unsigned char&gt;(line[p])))
            ++p;

        std::size_t name_start = p;
        while (p &lt; line.size() &amp;&amp;
               (std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) || line[p] == '_'))
            ++p;

        if (name_start == p)
            continue;

        std::string name = line.substr(name_start, p - name_start);
        if (name != method_name)
            continue;

        // Нашли нужный метод
        int decl_start = i;

        // Захватываем декораторы над методом (тем же отступом)
        for (int j = i - 1; j &gt; start; --j)
        {
            const std::string&amp; pline = m_lines[static_cast&lt;std::size_t&gt;(j)];
            std::size_t ppos = first_code_pos(pline);
            if (ppos &gt;= pline.size())
                break;

            int pind = calc_indent(pline);
            if (pind != member_indent)
                break;
            if (pline[ppos] != '@')
                break;

            decl_start = j;
        }

        int last_body = i;
        for (int k = i + 1; k &lt;= end &amp;&amp; k &lt; n; ++k)
        {
            const std::string&amp; l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];
            std::size_t pos2 = first_code_pos(l2);
            if (pos2 &gt;= l2.size())
                continue;

            int ind2 = calc_indent(l2);

            if (ind2 &lt;= class_indent)
                break; // вышли из класса
            if (ind2 &lt; member_indent)
                break; // вышли из метода

            // Новый метод / класс на том же уровне — заканчиваем текущий
            if (ind2 == member_indent)
            {
                bool is_new_block = false;

                if (l2.compare(pos2, 5, &quot;class&quot;) == 0 &amp;&amp;
                    (pos2 + 5 &gt;= l2.size() ||
                     std::isspace(static_cast&lt;unsigned char&gt;(l2[pos2 + 5]))))
                {
                    is_new_block = true;
                }
                else if (l2.compare(pos2, 3, &quot;def&quot;) == 0 &amp;&amp;
                         (pos2 + 3 &gt;= l2.size() ||
                          std::isspace(static_cast&lt;unsigned char&gt;(l2[pos2 + 3]))))
                {
                    is_new_block = true;
                }
                else if (l2.compare(pos2, 5, &quot;async&quot;) == 0)
                {
                    std::size_t q = pos2 + 5;
                    while (q &lt; l2.size() &amp;&amp;
                           std::isspace(static_cast&lt;unsigned char&gt;(l2[q])))
                        ++q;
                    if (l2.compare(q, 3, &quot;def&quot;) == 0 &amp;&amp;
                        (q + 3 &gt;= l2.size() ||
                         std::isspace(static_cast&lt;unsigned char&gt;(l2[q + 3]))))
                    {
                        is_new_block = true;
                    }
                }

                if (is_new_block)
                    break;
            }

            last_body = k;
        }

        out.start_line = decl_start;
        out.end_line = last_body;
        return true;
    }

    return false;
}

</code></pre>
</body>
</html>

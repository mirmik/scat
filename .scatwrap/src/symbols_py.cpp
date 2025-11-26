<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/symbols_py.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;symbols.h&quot;<br>
<br>
#include &lt;cctype&gt;<br>
<br>
PythonSymbolFinder::PythonSymbolFinder(const std::string &amp;text)<br>
{<br>
    m_lines.clear();<br>
    std::string current;<br>
    current.reserve(80);<br>
<br>
    for (char c : text)<br>
    {<br>
        if (c == '\n')<br>
        {<br>
            m_lines.push_back(current);<br>
            current.clear();<br>
        }<br>
        else if (c != '\r')<br>
        {<br>
            current.push_back(c);<br>
        }<br>
    }<br>
    m_lines.push_back(current);<br>
}<br>
<br>
int PythonSymbolFinder::calc_indent(const std::string &amp;line)<br>
{<br>
    int indent = 0;<br>
    for (char c : line)<br>
    {<br>
        if (c == ' ')<br>
            ++indent;<br>
        else if (c == '\t')<br>
            indent += 4; // грубая оценка, но устойчиво для сравнения<br>
        else<br>
            break;<br>
    }<br>
    return indent;<br>
}<br>
<br>
std::size_t PythonSymbolFinder::first_code_pos(const std::string &amp;line)<br>
{<br>
    std::size_t i = 0;<br>
    while (i &lt; line.size() &amp;&amp; (line[i] == ' ' || line[i] == '\t'))<br>
        ++i;<br>
    return i;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
                                             Region &amp;out,<br>
                                             int &amp;class_indent) const<br>
{<br>
    const int n = static_cast&lt;int&gt;(m_lines.size());<br>
    for (int i = 0; i &lt; n; ++i)<br>
    {<br>
        const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
        std::size_t pos = first_code_pos(line);<br>
        if (pos &gt;= line.size())<br>
            continue;<br>
<br>
        // комментарии / shebang<br>
        if (line[pos] == '#')<br>
            continue;<br>
<br>
        if (line.compare(pos, 5, &quot;class&quot;) != 0)<br>
            continue;<br>
<br>
        char after = (pos + 5 &lt; line.size()) ? line[pos + 5] : '\0';<br>
        if (!(after == '\0' ||<br>
              std::isspace(static_cast&lt;unsigned char&gt;(after)) || after == '(' ||<br>
              after == ':'))<br>
            continue;<br>
<br>
        std::size_t p = pos + 5;<br>
        while (p &lt; line.size() &amp;&amp;<br>
               std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
            ++p;<br>
<br>
        std::size_t name_start = p;<br>
        while (p &lt; line.size() &amp;&amp;<br>
               (std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
                line[p] == '_'))<br>
            ++p;<br>
<br>
        if (name_start == p)<br>
            continue;<br>
<br>
        std::string name = line.substr(name_start, p - name_start);<br>
        if (name != class_name)<br>
            continue;<br>
<br>
        class_indent = calc_indent(line);<br>
        int last_body = i;<br>
<br>
        for (int k = i + 1; k &lt; n; ++k)<br>
        {<br>
            const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
            std::size_t pos2 = first_code_pos(l2);<br>
            if (pos2 &gt;= l2.size())<br>
                continue; // пустая строка в теле<br>
<br>
            int ind2 = calc_indent(l2);<br>
            if (ind2 &lt;= class_indent)<br>
                break; // dedent — выходим из класса<br>
<br>
            last_body = k;<br>
        }<br>
<br>
        out.start_line = i;<br>
        out.end_line = last_body;<br>
        return true;<br>
    }<br>
<br>
    return false;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class(const std::string &amp;class_name,<br>
                                    Region &amp;out) const<br>
{<br>
    int indent = 0;<br>
    return find_class_internal(class_name, out, indent);<br>
}<br>
<br>
bool PythonSymbolFinder::find_method(const std::string &amp;class_name,<br>
                                     const std::string &amp;method_name,<br>
                                     Region &amp;out) const<br>
{<br>
    Region class_region;<br>
    int class_indent = 0;<br>
    if (!find_class_internal(class_name, class_region, class_indent))<br>
        return false;<br>
<br>
    const int start = class_region.start_line;<br>
    const int end = class_region.end_line;<br>
<br>
    // Определяем базовый уровень отступа для членов класса<br>
    int member_indent = -1;<br>
    for (int i = start + 1; i &lt;= end; ++i)<br>
    {<br>
        const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
        std::size_t pos = first_code_pos(line);<br>
        if (pos &gt;= line.size())<br>
            continue;<br>
        if (line[pos] == '#')<br>
            continue;<br>
<br>
        int ind = calc_indent(line);<br>
        if (ind &lt;= class_indent)<br>
            continue;<br>
<br>
        member_indent = ind;<br>
        break;<br>
    }<br>
<br>
    if (member_indent &lt; 0)<br>
        return false; // пустой класс<br>
<br>
    const int n = static_cast&lt;int&gt;(m_lines.size());<br>
<br>
    for (int i = start + 1; i &lt;= end &amp;&amp; i &lt; n; ++i)<br>
    {<br>
        const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
        std::size_t pos = first_code_pos(line);<br>
        if (pos &gt;= line.size())<br>
            continue;<br>
<br>
        int ind = calc_indent(line);<br>
        if (ind != member_indent)<br>
            continue;<br>
<br>
        if (line[pos] == '#')<br>
            continue;<br>
<br>
        // def / async def<br>
        std::size_t p = pos;<br>
        bool is_async = false;<br>
<br>
        if (line.compare(p, 5, &quot;async&quot;) == 0 &amp;&amp;<br>
            (p + 5 &gt;= line.size() ||<br>
             std::isspace(static_cast&lt;unsigned char&gt;(line[p + 5]))))<br>
        {<br>
            is_async = true;<br>
            p += 5;<br>
            while (p &lt; line.size() &amp;&amp;<br>
                   std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
                ++p;<br>
        }<br>
<br>
        if (line.compare(p, 3, &quot;def&quot;) != 0 ||<br>
            (p + 3 &lt; line.size() &amp;&amp;<br>
             !std::isspace(static_cast&lt;unsigned char&gt;(line[p + 3]))))<br>
        {<br>
            continue;<br>
        }<br>
<br>
        p += 3;<br>
        while (p &lt; line.size() &amp;&amp;<br>
               std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
            ++p;<br>
<br>
        std::size_t name_start = p;<br>
        while (p &lt; line.size() &amp;&amp;<br>
               (std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
                line[p] == '_'))<br>
            ++p;<br>
<br>
        if (name_start == p)<br>
            continue;<br>
<br>
        std::string name = line.substr(name_start, p - name_start);<br>
        if (name != method_name)<br>
            continue;<br>
<br>
        // Нашли нужный метод<br>
        int decl_start = i;<br>
<br>
        // Захватываем декораторы над методом (тем же отступом)<br>
        for (int j = i - 1; j &gt; start; --j)<br>
        {<br>
            const std::string &amp;pline = m_lines[static_cast&lt;std::size_t&gt;(j)];<br>
            std::size_t ppos = first_code_pos(pline);<br>
            if (ppos &gt;= pline.size())<br>
                break;<br>
<br>
            int pind = calc_indent(pline);<br>
            if (pind != member_indent)<br>
                break;<br>
            if (pline[ppos] != '@')<br>
                break;<br>
<br>
            decl_start = j;<br>
        }<br>
<br>
        int last_body = i;<br>
        for (int k = i + 1; k &lt;= end &amp;&amp; k &lt; n; ++k)<br>
        {<br>
            const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
            std::size_t pos2 = first_code_pos(l2);<br>
            if (pos2 &gt;= l2.size())<br>
                continue;<br>
<br>
            int ind2 = calc_indent(l2);<br>
<br>
            if (ind2 &lt;= class_indent)<br>
                break; // вышли из класса<br>
            if (ind2 &lt; member_indent)<br>
                break; // вышли из метода<br>
<br>
            // Новый метод / класс на том же уровне — заканчиваем текущий<br>
            if (ind2 == member_indent)<br>
            {<br>
                bool is_new_block = false;<br>
<br>
                if (l2.compare(pos2, 5, &quot;class&quot;) == 0 &amp;&amp;<br>
                    (pos2 + 5 &gt;= l2.size() ||<br>
                     std::isspace(static_cast&lt;unsigned char&gt;(l2[pos2 + 5]))))<br>
                {<br>
                    is_new_block = true;<br>
                }<br>
                else if (l2.compare(pos2, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
                         (pos2 + 3 &gt;= l2.size() ||<br>
                          std::isspace(<br>
                              static_cast&lt;unsigned char&gt;(l2[pos2 + 3]))))<br>
                {<br>
                    is_new_block = true;<br>
                }<br>
                else if (l2.compare(pos2, 5, &quot;async&quot;) == 0)<br>
                {<br>
                    std::size_t q = pos2 + 5;<br>
                    while (q &lt; l2.size() &amp;&amp;<br>
                           std::isspace(static_cast&lt;unsigned char&gt;(l2[q])))<br>
                        ++q;<br>
                    if (l2.compare(q, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
                        (q + 3 &gt;= l2.size() ||<br>
                         std::isspace(static_cast&lt;unsigned char&gt;(l2[q + 3]))))<br>
                    {<br>
                        is_new_block = true;<br>
                    }<br>
                }<br>
<br>
                if (is_new_block)<br>
                    break;<br>
            }<br>
<br>
            last_body = k;<br>
        }<br>
<br>
        out.start_line = decl_start;<br>
        out.end_line = last_body;<br>
        return true;<br>
    }<br>
<br>
    return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

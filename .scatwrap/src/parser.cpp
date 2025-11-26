<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;parser.h&quot;<br>
#include &quot;rules.h&quot;<br>
#include &lt;cctype&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;stdexcept&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
static inline void trim(std::string &amp;s)<br>
{<br>
    size_t b = s.find_first_not_of(&quot; \t\r\n&quot;);<br>
    if (b == std::string::npos)<br>
    {<br>
        s.clear();<br>
        return;<br>
    }<br>
    size_t e = s.find_last_not_of(&quot; \t\r\n&quot;);<br>
    s = s.substr(b, e - b + 1);<br>
}<br>
<br>
Config parse_config(const fs::path &amp;path)<br>
{<br>
    Config cfg;<br>
    enum Section<br>
    {<br>
        TEXT_RULES,<br>
        TREE_RULES,<br>
        MAPFORMAT_TEXT<br>
    };<br>
    Section current = TEXT_RULES;<br>
<br>
    std::ifstream in(path);<br>
    if (!in.is_open())<br>
        throw std::runtime_error(&quot;Failed to open config file: &quot; +<br>
                                 path.string());<br>
<br>
    std::string raw_line;<br>
    while (std::getline(in, raw_line))<br>
    {<br>
        std::string line = raw_line;<br>
        trim(line);<br>
<br>
        if (current == MAPFORMAT_TEXT)<br>
        {<br>
            // Всё, что после [MAPFORMAT], идёт как есть (с сохранением пустых<br>
            // строк и #) до конца файла (пока новых секций у нас нет).<br>
            cfg.map_format += raw_line;<br>
            cfg.map_format += &quot;\n&quot;;<br>
            continue;<br>
        }<br>
<br>
        // вне MAPFORMAT — старая логика<br>
        if (line.empty() || line[0] == '#')<br>
            continue;<br>
<br>
        if (line == &quot;[TREE]&quot;)<br>
        {<br>
            current = TREE_RULES;<br>
            continue;<br>
        }<br>
        if (line == &quot;[MAPFORMAT]&quot;)<br>
        {<br>
            current = MAPFORMAT_TEXT;<br>
            continue;<br>
        }<br>
<br>
        Rule r = Rule::from_string(line);<br>
<br>
        if (current == TEXT_RULES)<br>
            cfg.text_rules.push_back(r);<br>
        else<br>
            cfg.tree_rules.push_back(r);<br>
    }<br>
<br>
    return cfg;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

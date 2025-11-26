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
&#9;size_t b = s.find_first_not_of(&quot; \t\r\n&quot;);<br>
&#9;if (b == std::string::npos)<br>
&#9;{<br>
&#9;&#9;s.clear();<br>
&#9;&#9;return;<br>
&#9;}<br>
&#9;size_t e = s.find_last_not_of(&quot; \t\r\n&quot;);<br>
&#9;s = s.substr(b, e - b + 1);<br>
}<br>
<br>
Config parse_config(const fs::path &amp;path)<br>
{<br>
&#9;Config cfg;<br>
&#9;enum Section<br>
&#9;{<br>
&#9;&#9;TEXT_RULES,<br>
&#9;&#9;TREE_RULES,<br>
&#9;&#9;MAPFORMAT_TEXT<br>
&#9;};<br>
&#9;Section current = TEXT_RULES;<br>
<br>
&#9;std::ifstream in(path);<br>
&#9;if (!in.is_open())<br>
&#9;&#9;throw std::runtime_error(&quot;Failed to open config file: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;path.string());<br>
<br>
&#9;std::string raw_line;<br>
&#9;while (std::getline(in, raw_line))<br>
&#9;{<br>
&#9;&#9;std::string line = raw_line;<br>
&#9;&#9;trim(line);<br>
<br>
&#9;&#9;if (current == MAPFORMAT_TEXT)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// Всё, что после [MAPFORMAT], идёт как есть (с сохранением пустых<br>
&#9;&#9;&#9;// строк и #) до конца файла (пока новых секций у нас нет).<br>
&#9;&#9;&#9;cfg.map_format += raw_line;<br>
&#9;&#9;&#9;cfg.map_format += &quot;\n&quot;;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// вне MAPFORMAT — старая логика<br>
&#9;&#9;if (line.empty() || line[0] == '#')<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;if (line == &quot;[TREE]&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;current = TREE_RULES;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
&#9;&#9;if (line == &quot;[MAPFORMAT]&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;current = MAPFORMAT_TEXT;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;Rule r = Rule::from_string(line);<br>
<br>
&#9;&#9;if (current == TEXT_RULES)<br>
&#9;&#9;&#9;cfg.text_rules.push_back(r);<br>
&#9;&#9;else<br>
&#9;&#9;&#9;cfg.tree_rules.push_back(r);<br>
&#9;}<br>
<br>
&#9;return cfg;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

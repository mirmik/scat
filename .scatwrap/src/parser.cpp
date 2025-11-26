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
&emsp;size_t b = s.find_first_not_of(&quot; \t\r\n&quot;);<br>
&emsp;if (b == std::string::npos)<br>
&emsp;{<br>
&emsp;&emsp;s.clear();<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
&emsp;size_t e = s.find_last_not_of(&quot; \t\r\n&quot;);<br>
&emsp;s = s.substr(b, e - b + 1);<br>
}<br>
<br>
Config parse_config(const fs::path &amp;path)<br>
{<br>
&emsp;Config cfg;<br>
&emsp;enum Section<br>
&emsp;{<br>
&emsp;&emsp;TEXT_RULES,<br>
&emsp;&emsp;TREE_RULES,<br>
&emsp;&emsp;MAPFORMAT_TEXT<br>
&emsp;};<br>
&emsp;Section current = TEXT_RULES;<br>
<br>
&emsp;std::ifstream in(path);<br>
&emsp;if (!in.is_open())<br>
&emsp;&emsp;throw std::runtime_error(&quot;Failed to open config file: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;path.string());<br>
<br>
&emsp;std::string raw_line;<br>
&emsp;while (std::getline(in, raw_line))<br>
&emsp;{<br>
&emsp;&emsp;std::string line = raw_line;<br>
&emsp;&emsp;trim(line);<br>
<br>
&emsp;&emsp;if (current == MAPFORMAT_TEXT)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// Всё, что после [MAPFORMAT], идёт как есть (с сохранением пустых<br>
&emsp;&emsp;&emsp;// строк и #) до конца файла (пока новых секций у нас нет).<br>
&emsp;&emsp;&emsp;cfg.map_format += raw_line;<br>
&emsp;&emsp;&emsp;cfg.map_format += &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// вне MAPFORMAT — старая логика<br>
&emsp;&emsp;if (line.empty() || line[0] == '#')<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;if (line == &quot;[TREE]&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;current = TREE_RULES;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;if (line == &quot;[MAPFORMAT]&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;current = MAPFORMAT_TEXT;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;Rule r = Rule::from_string(line);<br>
<br>
&emsp;&emsp;if (current == TEXT_RULES)<br>
&emsp;&emsp;&emsp;cfg.text_rules.push_back(r);<br>
&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;cfg.tree_rules.push_back(r);<br>
&emsp;}<br>
<br>
&emsp;return cfg;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

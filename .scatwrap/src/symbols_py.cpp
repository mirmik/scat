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
&#9;m_lines.clear();<br>
&#9;std::string current;<br>
&#9;current.reserve(80);<br>
<br>
&#9;for (char c : text)<br>
&#9;{<br>
&#9;&#9;if (c == '\n')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;m_lines.push_back(current);<br>
&#9;&#9;&#9;current.clear();<br>
&#9;&#9;}<br>
&#9;&#9;else if (c != '\r')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;current.push_back(c);<br>
&#9;&#9;}<br>
&#9;}<br>
&#9;m_lines.push_back(current);<br>
}<br>
<br>
int PythonSymbolFinder::calc_indent(const std::string &amp;line)<br>
{<br>
&#9;int indent = 0;<br>
&#9;for (char c : line)<br>
&#9;{<br>
&#9;&#9;if (c == ' ')<br>
&#9;&#9;&#9;++indent;<br>
&#9;&#9;else if (c == '\t')<br>
&#9;&#9;&#9;indent += 4; // грубая оценка, но устойчиво для сравнения<br>
&#9;&#9;else<br>
&#9;&#9;&#9;break;<br>
&#9;}<br>
&#9;return indent;<br>
}<br>
<br>
std::size_t PythonSymbolFinder::first_code_pos(const std::string &amp;line)<br>
{<br>
&#9;std::size_t i = 0;<br>
&#9;while (i &lt; line.size() &amp;&amp; (line[i] == ' ' || line[i] == '\t'))<br>
&#9;&#9;++i;<br>
&#9;return i;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;int &amp;class_indent) const<br>
{<br>
&#9;const int n = static_cast&lt;int&gt;(m_lines.size());<br>
&#9;for (int i = 0; i &lt; n; ++i)<br>
&#9;{<br>
&#9;&#9;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&#9;&#9;std::size_t pos = first_code_pos(line);<br>
&#9;&#9;if (pos &gt;= line.size())<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// комментарии / shebang<br>
&#9;&#9;if (line[pos] == '#')<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;if (line.compare(pos, 5, &quot;class&quot;) != 0)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;char after = (pos + 5 &lt; line.size()) ? line[pos + 5] : '\0';<br>
&#9;&#9;if (!(after == '\0' ||<br>
&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(after)) || after == '(' ||<br>
&#9;&#9;&#9;after == ':'))<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;std::size_t p = pos + 5;<br>
&#9;&#9;while (p &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&#9;&#9;&#9;++p;<br>
<br>
&#9;&#9;std::size_t name_start = p;<br>
&#9;&#9;while (p &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;(std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
&#9;&#9;&#9;&#9;line[p] == '_'))<br>
&#9;&#9;&#9;++p;<br>
<br>
&#9;&#9;if (name_start == p)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;std::string name = line.substr(name_start, p - name_start);<br>
&#9;&#9;if (name != class_name)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;class_indent = calc_indent(line);<br>
&#9;&#9;int last_body = i;<br>
<br>
&#9;&#9;for (int k = i + 1; k &lt; n; ++k)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
&#9;&#9;&#9;std::size_t pos2 = first_code_pos(l2);<br>
&#9;&#9;&#9;if (pos2 &gt;= l2.size())<br>
&#9;&#9;&#9;&#9;continue; // пустая строка в теле<br>
<br>
&#9;&#9;&#9;int ind2 = calc_indent(l2);<br>
&#9;&#9;&#9;if (ind2 &lt;= class_indent)<br>
&#9;&#9;&#9;&#9;break; // dedent — выходим из класса<br>
<br>
&#9;&#9;&#9;last_body = k;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;out.start_line = i;<br>
&#9;&#9;out.end_line = last_body;<br>
&#9;&#9;return true;<br>
&#9;}<br>
<br>
&#9;return false;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out) const<br>
{<br>
&#9;int indent = 0;<br>
&#9;return find_class_internal(class_name, out, indent);<br>
}<br>
<br>
bool PythonSymbolFinder::find_method(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::string &amp;method_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out) const<br>
{<br>
&#9;Region class_region;<br>
&#9;int class_indent = 0;<br>
&#9;if (!find_class_internal(class_name, class_region, class_indent))<br>
&#9;&#9;return false;<br>
<br>
&#9;const int start = class_region.start_line;<br>
&#9;const int end = class_region.end_line;<br>
<br>
&#9;// Определяем базовый уровень отступа для членов класса<br>
&#9;int member_indent = -1;<br>
&#9;for (int i = start + 1; i &lt;= end; ++i)<br>
&#9;{<br>
&#9;&#9;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&#9;&#9;std::size_t pos = first_code_pos(line);<br>
&#9;&#9;if (pos &gt;= line.size())<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;if (line[pos] == '#')<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;int ind = calc_indent(line);<br>
&#9;&#9;if (ind &lt;= class_indent)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;member_indent = ind;<br>
&#9;&#9;break;<br>
&#9;}<br>
<br>
&#9;if (member_indent &lt; 0)<br>
&#9;&#9;return false; // пустой класс<br>
<br>
&#9;const int n = static_cast&lt;int&gt;(m_lines.size());<br>
<br>
&#9;for (int i = start + 1; i &lt;= end &amp;&amp; i &lt; n; ++i)<br>
&#9;{<br>
&#9;&#9;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&#9;&#9;std::size_t pos = first_code_pos(line);<br>
&#9;&#9;if (pos &gt;= line.size())<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;int ind = calc_indent(line);<br>
&#9;&#9;if (ind != member_indent)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;if (line[pos] == '#')<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// def / async def<br>
&#9;&#9;std::size_t p = pos;<br>
&#9;&#9;bool is_async = false;<br>
<br>
&#9;&#9;if (line.compare(p, 5, &quot;async&quot;) == 0 &amp;&amp;<br>
&#9;&#9;&#9;(p + 5 &gt;= line.size() ||<br>
&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(line[p + 5]))))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;is_async = true;<br>
&#9;&#9;&#9;p += 5;<br>
&#9;&#9;&#9;while (p &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&#9;&#9;&#9;&#9;++p;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (line.compare(p, 3, &quot;def&quot;) != 0 ||<br>
&#9;&#9;&#9;(p + 3 &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;!std::isspace(static_cast&lt;unsigned char&gt;(line[p + 3]))))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;p += 3;<br>
&#9;&#9;while (p &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&#9;&#9;&#9;++p;<br>
<br>
&#9;&#9;std::size_t name_start = p;<br>
&#9;&#9;while (p &lt; line.size() &amp;&amp;<br>
&#9;&#9;&#9;(std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
&#9;&#9;&#9;&#9;line[p] == '_'))<br>
&#9;&#9;&#9;++p;<br>
<br>
&#9;&#9;if (name_start == p)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;std::string name = line.substr(name_start, p - name_start);<br>
&#9;&#9;if (name != method_name)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// Нашли нужный метод<br>
&#9;&#9;int decl_start = i;<br>
<br>
&#9;&#9;// Захватываем декораторы над методом (тем же отступом)<br>
&#9;&#9;for (int j = i - 1; j &gt; start; --j)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;pline = m_lines[static_cast&lt;std::size_t&gt;(j)];<br>
&#9;&#9;&#9;std::size_t ppos = first_code_pos(pline);<br>
&#9;&#9;&#9;if (ppos &gt;= pline.size())<br>
&#9;&#9;&#9;&#9;break;<br>
<br>
&#9;&#9;&#9;int pind = calc_indent(pline);<br>
&#9;&#9;&#9;if (pind != member_indent)<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;if (pline[ppos] != '@')<br>
&#9;&#9;&#9;&#9;break;<br>
<br>
&#9;&#9;&#9;decl_start = j;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;int last_body = i;<br>
&#9;&#9;for (int k = i + 1; k &lt;= end &amp;&amp; k &lt; n; ++k)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
&#9;&#9;&#9;std::size_t pos2 = first_code_pos(l2);<br>
&#9;&#9;&#9;if (pos2 &gt;= l2.size())<br>
&#9;&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;&#9;int ind2 = calc_indent(l2);<br>
<br>
&#9;&#9;&#9;if (ind2 &lt;= class_indent)<br>
&#9;&#9;&#9;&#9;break; // вышли из класса<br>
&#9;&#9;&#9;if (ind2 &lt; member_indent)<br>
&#9;&#9;&#9;&#9;break; // вышли из метода<br>
<br>
&#9;&#9;&#9;// Новый метод / класс на том же уровне — заканчиваем текущий<br>
&#9;&#9;&#9;if (ind2 == member_indent)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;bool is_new_block = false;<br>
<br>
&#9;&#9;&#9;&#9;if (l2.compare(pos2, 5, &quot;class&quot;) == 0 &amp;&amp;<br>
&#9;&#9;&#9;&#9;&#9;(pos2 + 5 &gt;= l2.size() ||<br>
&#9;&#9;&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(l2[pos2 + 5]))))<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;is_new_block = true;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else if (l2.compare(pos2, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
&#9;&#9;&#9;&#9;&#9;&#9;(pos2 + 3 &gt;= l2.size() ||<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::isspace(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;static_cast&lt;unsigned char&gt;(l2[pos2 + 3]))))<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;is_new_block = true;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else if (l2.compare(pos2, 5, &quot;async&quot;) == 0)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;std::size_t q = pos2 + 5;<br>
&#9;&#9;&#9;&#9;&#9;while (q &lt; l2.size() &amp;&amp;<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(l2[q])))<br>
&#9;&#9;&#9;&#9;&#9;&#9;++q;<br>
&#9;&#9;&#9;&#9;&#9;if (l2.compare(q, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
&#9;&#9;&#9;&#9;&#9;&#9;(q + 3 &gt;= l2.size() ||<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::isspace(static_cast&lt;unsigned char&gt;(l2[q + 3]))))<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;is_new_block = true;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;&#9;if (is_new_block)<br>
&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;last_body = k;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;out.start_line = decl_start;<br>
&#9;&#9;out.end_line = last_body;<br>
&#9;&#9;return true;<br>
&#9;}<br>
<br>
&#9;return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

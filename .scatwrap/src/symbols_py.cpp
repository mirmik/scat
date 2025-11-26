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
&emsp;m_lines.clear();<br>
&emsp;std::string current;<br>
&emsp;current.reserve(80);<br>
<br>
&emsp;for (char c : text)<br>
&emsp;{<br>
&emsp;&emsp;if (c == '\n')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;m_lines.push_back(current);<br>
&emsp;&emsp;&emsp;current.clear();<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (c != '\r')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;current.push_back(c);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
&emsp;m_lines.push_back(current);<br>
}<br>
<br>
int PythonSymbolFinder::calc_indent(const std::string &amp;line)<br>
{<br>
&emsp;int indent = 0;<br>
&emsp;for (char c : line)<br>
&emsp;{<br>
&emsp;&emsp;if (c == ' ')<br>
&emsp;&emsp;&emsp;++indent;<br>
&emsp;&emsp;else if (c == '\t')<br>
&emsp;&emsp;&emsp;indent += 4; // грубая оценка, но устойчиво для сравнения<br>
&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;}<br>
&emsp;return indent;<br>
}<br>
<br>
std::size_t PythonSymbolFinder::first_code_pos(const std::string &amp;line)<br>
{<br>
&emsp;std::size_t i = 0;<br>
&emsp;while (i &lt; line.size() &amp;&amp; (line[i] == ' ' || line[i] == '\t'))<br>
&emsp;&emsp;++i;<br>
&emsp;return i;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;int &amp;class_indent) const<br>
{<br>
&emsp;const int n = static_cast&lt;int&gt;(m_lines.size());<br>
&emsp;for (int i = 0; i &lt; n; ++i)<br>
&emsp;{<br>
&emsp;&emsp;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&emsp;&emsp;std::size_t pos = first_code_pos(line);<br>
&emsp;&emsp;if (pos &gt;= line.size())<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// комментарии / shebang<br>
&emsp;&emsp;if (line[pos] == '#')<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;if (line.compare(pos, 5, &quot;class&quot;) != 0)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;char after = (pos + 5 &lt; line.size()) ? line[pos + 5] : '\0';<br>
&emsp;&emsp;if (!(after == '\0' ||<br>
&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(after)) || after == '(' ||<br>
&emsp;&emsp;&emsp;after == ':'))<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;std::size_t p = pos + 5;<br>
&emsp;&emsp;while (p &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&emsp;&emsp;&emsp;++p;<br>
<br>
&emsp;&emsp;std::size_t name_start = p;<br>
&emsp;&emsp;while (p &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;(std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
&emsp;&emsp;&emsp;&emsp;line[p] == '_'))<br>
&emsp;&emsp;&emsp;++p;<br>
<br>
&emsp;&emsp;if (name_start == p)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;std::string name = line.substr(name_start, p - name_start);<br>
&emsp;&emsp;if (name != class_name)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;class_indent = calc_indent(line);<br>
&emsp;&emsp;int last_body = i;<br>
<br>
&emsp;&emsp;for (int k = i + 1; k &lt; n; ++k)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
&emsp;&emsp;&emsp;std::size_t pos2 = first_code_pos(l2);<br>
&emsp;&emsp;&emsp;if (pos2 &gt;= l2.size())<br>
&emsp;&emsp;&emsp;&emsp;continue; // пустая строка в теле<br>
<br>
&emsp;&emsp;&emsp;int ind2 = calc_indent(l2);<br>
&emsp;&emsp;&emsp;if (ind2 &lt;= class_indent)<br>
&emsp;&emsp;&emsp;&emsp;break; // dedent — выходим из класса<br>
<br>
&emsp;&emsp;&emsp;last_body = k;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;out.start_line = i;<br>
&emsp;&emsp;out.end_line = last_body;<br>
&emsp;&emsp;return true;<br>
&emsp;}<br>
<br>
&emsp;return false;<br>
}<br>
<br>
bool PythonSymbolFinder::find_class(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const<br>
{<br>
&emsp;int indent = 0;<br>
&emsp;return find_class_internal(class_name, out, indent);<br>
}<br>
<br>
bool PythonSymbolFinder::find_method(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;method_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const<br>
{<br>
&emsp;Region class_region;<br>
&emsp;int class_indent = 0;<br>
&emsp;if (!find_class_internal(class_name, class_region, class_indent))<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;const int start = class_region.start_line;<br>
&emsp;const int end = class_region.end_line;<br>
<br>
&emsp;// Определяем базовый уровень отступа для членов класса<br>
&emsp;int member_indent = -1;<br>
&emsp;for (int i = start + 1; i &lt;= end; ++i)<br>
&emsp;{<br>
&emsp;&emsp;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&emsp;&emsp;std::size_t pos = first_code_pos(line);<br>
&emsp;&emsp;if (pos &gt;= line.size())<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;if (line[pos] == '#')<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;int ind = calc_indent(line);<br>
&emsp;&emsp;if (ind &lt;= class_indent)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;member_indent = ind;<br>
&emsp;&emsp;break;<br>
&emsp;}<br>
<br>
&emsp;if (member_indent &lt; 0)<br>
&emsp;&emsp;return false; // пустой класс<br>
<br>
&emsp;const int n = static_cast&lt;int&gt;(m_lines.size());<br>
<br>
&emsp;for (int i = start + 1; i &lt;= end &amp;&amp; i &lt; n; ++i)<br>
&emsp;{<br>
&emsp;&emsp;const std::string &amp;line = m_lines[static_cast&lt;std::size_t&gt;(i)];<br>
&emsp;&emsp;std::size_t pos = first_code_pos(line);<br>
&emsp;&emsp;if (pos &gt;= line.size())<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;int ind = calc_indent(line);<br>
&emsp;&emsp;if (ind != member_indent)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;if (line[pos] == '#')<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// def / async def<br>
&emsp;&emsp;std::size_t p = pos;<br>
&emsp;&emsp;bool is_async = false;<br>
<br>
&emsp;&emsp;if (line.compare(p, 5, &quot;async&quot;) == 0 &amp;&amp;<br>
&emsp;&emsp;&emsp;(p + 5 &gt;= line.size() ||<br>
&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(line[p + 5]))))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;is_async = true;<br>
&emsp;&emsp;&emsp;p += 5;<br>
&emsp;&emsp;&emsp;while (p &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&emsp;&emsp;&emsp;&emsp;++p;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (line.compare(p, 3, &quot;def&quot;) != 0 ||<br>
&emsp;&emsp;&emsp;(p + 3 &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;!std::isspace(static_cast&lt;unsigned char&gt;(line[p + 3]))))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;p += 3;<br>
&emsp;&emsp;while (p &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(line[p])))<br>
&emsp;&emsp;&emsp;++p;<br>
<br>
&emsp;&emsp;std::size_t name_start = p;<br>
&emsp;&emsp;while (p &lt; line.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;(std::isalnum(static_cast&lt;unsigned char&gt;(line[p])) ||<br>
&emsp;&emsp;&emsp;&emsp;line[p] == '_'))<br>
&emsp;&emsp;&emsp;++p;<br>
<br>
&emsp;&emsp;if (name_start == p)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;std::string name = line.substr(name_start, p - name_start);<br>
&emsp;&emsp;if (name != method_name)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// Нашли нужный метод<br>
&emsp;&emsp;int decl_start = i;<br>
<br>
&emsp;&emsp;// Захватываем декораторы над методом (тем же отступом)<br>
&emsp;&emsp;for (int j = i - 1; j &gt; start; --j)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;pline = m_lines[static_cast&lt;std::size_t&gt;(j)];<br>
&emsp;&emsp;&emsp;std::size_t ppos = first_code_pos(pline);<br>
&emsp;&emsp;&emsp;if (ppos &gt;= pline.size())<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
<br>
&emsp;&emsp;&emsp;int pind = calc_indent(pline);<br>
&emsp;&emsp;&emsp;if (pind != member_indent)<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;if (pline[ppos] != '@')<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
<br>
&emsp;&emsp;&emsp;decl_start = j;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;int last_body = i;<br>
&emsp;&emsp;for (int k = i + 1; k &lt;= end &amp;&amp; k &lt; n; ++k)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;l2 = m_lines[static_cast&lt;std::size_t&gt;(k)];<br>
&emsp;&emsp;&emsp;std::size_t pos2 = first_code_pos(l2);<br>
&emsp;&emsp;&emsp;if (pos2 &gt;= l2.size())<br>
&emsp;&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;&emsp;int ind2 = calc_indent(l2);<br>
<br>
&emsp;&emsp;&emsp;if (ind2 &lt;= class_indent)<br>
&emsp;&emsp;&emsp;&emsp;break; // вышли из класса<br>
&emsp;&emsp;&emsp;if (ind2 &lt; member_indent)<br>
&emsp;&emsp;&emsp;&emsp;break; // вышли из метода<br>
<br>
&emsp;&emsp;&emsp;// Новый метод / класс на том же уровне — заканчиваем текущий<br>
&emsp;&emsp;&emsp;if (ind2 == member_indent)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;bool is_new_block = false;<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (l2.compare(pos2, 5, &quot;class&quot;) == 0 &amp;&amp;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;(pos2 + 5 &gt;= l2.size() ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(l2[pos2 + 5]))))<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;is_new_block = true;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else if (l2.compare(pos2, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;(pos2 + 3 &gt;= l2.size() ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::isspace(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;static_cast&lt;unsigned char&gt;(l2[pos2 + 3]))))<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;is_new_block = true;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else if (l2.compare(pos2, 5, &quot;async&quot;) == 0)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;std::size_t q = pos2 + 5;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;while (q &lt; l2.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(l2[q])))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;++q;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (l2.compare(q, 3, &quot;def&quot;) == 0 &amp;&amp;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;(q + 3 &gt;= l2.size() ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::isspace(static_cast&lt;unsigned char&gt;(l2[q + 3]))))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;is_new_block = true;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (is_new_block)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;last_body = k;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;out.start_line = decl_start;<br>
&emsp;&emsp;out.end_line = last_body;<br>
&emsp;&emsp;return true;<br>
&emsp;}<br>
<br>
&emsp;return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

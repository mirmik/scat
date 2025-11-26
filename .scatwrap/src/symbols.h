<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/symbols.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
<br>
#include &lt;cstddef&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
// Диапазон строк в файле (0-based, включительно).<br>
struct Region<br>
{<br>
&#9;int start_line = -1; // inclusive<br>
&#9;int end_line = -1;   // inclusive<br>
};<br>
<br>
// Простейший поисковик C++-символов.<br>
// Умеет:<br>
//   * находить определение класса (class / struct)<br>
//   * искать методы внутри класса (по имени)<br>
class CppSymbolFinder<br>
{<br>
public:<br>
&#9;// text — полный текст файла.<br>
&#9;explicit CppSymbolFinder(const std::string &amp;text);<br>
<br>
&#9;const std::vector&lt;std::string&gt; &amp;lines() const<br>
&#9;{<br>
&#9;&#9;return m_lines;<br>
&#9;}<br>
<br>
&#9;// Находит определение класса (НЕ forward-declaration).<br>
&#9;// Возвращает true, если класс найден.<br>
&#9;bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
&#9;// Находит метод внутри класса (по имени).<br>
&#9;// Ищет только внутри тела class/struct class_name.<br>
&#9;// Возвращает диапазон строк от начала объявления/определения<br>
&#9;// до ';' или закрывающей '}'.<br>
&#9;bool find_method(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;method_name,<br>
&#9;&#9;&#9;&#9;&#9;Region &amp;out) const;<br>
<br>
private:<br>
&#9;struct Token<br>
&#9;{<br>
&#9;&#9;enum Kind<br>
&#9;&#9;{<br>
&#9;&#9;&#9;Identifier,<br>
&#9;&#9;&#9;Keyword,<br>
&#9;&#9;&#9;Symbol<br>
&#9;&#9;} kind;<br>
&#9;&#9;std::string text;<br>
&#9;&#9;int line = 0; // 0-based<br>
&#9;};<br>
<br>
&#9;struct ClassRange<br>
&#9;{<br>
&#9;&#9;Region region;<br>
&#9;&#9;std::size_t body_start_token = 0; // индекс '{'<br>
&#9;&#9;std::size_t body_end_token = 0;   // индекс '}' (или '};')<br>
&#9;};<br>
<br>
&#9;std::vector&lt;std::string&gt; m_lines;<br>
&#9;std::vector&lt;Token&gt; m_tokens;<br>
<br>
&#9;void tokenize(const std::string &amp;text);<br>
<br>
&#9;static bool is_ident_start(char c);<br>
&#9;static bool is_ident_char(char c);<br>
<br>
&#9;bool find_class_internal(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;ClassRange &amp;out) const;<br>
};<br>
<br>
// Простейший поисковик Python-символов.<br>
// Умеет:<br>
//   * находить определение класса (class Foo:)<br>
//   * искать методы внутри класса (def bar(self, ...))<br>
class PythonSymbolFinder<br>
{<br>
public:<br>
&#9;explicit PythonSymbolFinder(const std::string &amp;text);<br>
<br>
&#9;const std::vector&lt;std::string&gt; &amp;lines() const<br>
&#9;{<br>
&#9;&#9;return m_lines;<br>
&#9;}<br>
<br>
&#9;// Находит определение класса (первое вхождение с таким именем).<br>
&#9;// Region покрывает строку 'class ...' и всё тело класса до dedent.<br>
&#9;bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
&#9;// Находит метод внутри класса.<br>
&#9;// Ищет def / async def с именем method_name,<br>
&#9;// являющийся &quot;первым уровнем&quot; внутри тела class_name.<br>
&#9;// Region покрывает строку def (включая декораторы над ней) и всё тело до<br>
&#9;// dedent.<br>
&#9;bool find_method(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;method_name,<br>
&#9;&#9;&#9;&#9;&#9;Region &amp;out) const;<br>
<br>
private:<br>
&#9;std::vector&lt;std::string&gt; m_lines;<br>
<br>
&#9;static int calc_indent(const std::string &amp;line);<br>
&#9;static std::size_t first_code_pos(const std::string &amp;line);<br>
<br>
&#9;bool find_class_internal(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;int &amp;class_indent) const;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

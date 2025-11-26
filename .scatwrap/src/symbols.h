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
&emsp;int start_line = -1; // inclusive<br>
&emsp;int end_line = -1;   // inclusive<br>
};<br>
<br>
// Простейший поисковик C++-символов.<br>
// Умеет:<br>
//   * находить определение класса (class / struct)<br>
//   * искать методы внутри класса (по имени)<br>
class CppSymbolFinder<br>
{<br>
public:<br>
&emsp;// text — полный текст файла.<br>
&emsp;explicit CppSymbolFinder(const std::string &amp;text);<br>
<br>
&emsp;const std::vector&lt;std::string&gt; &amp;lines() const<br>
&emsp;{<br>
&emsp;&emsp;return m_lines;<br>
&emsp;}<br>
<br>
&emsp;// Находит определение класса (НЕ forward-declaration).<br>
&emsp;// Возвращает true, если класс найден.<br>
&emsp;bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
&emsp;// Находит метод внутри класса (по имени).<br>
&emsp;// Ищет только внутри тела class/struct class_name.<br>
&emsp;// Возвращает диапазон строк от начала объявления/определения<br>
&emsp;// до ';' или закрывающей '}'.<br>
&emsp;bool find_method(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;method_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const;<br>
<br>
private:<br>
&emsp;struct Token<br>
&emsp;{<br>
&emsp;&emsp;enum Kind<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;Identifier,<br>
&emsp;&emsp;&emsp;Keyword,<br>
&emsp;&emsp;&emsp;Symbol<br>
&emsp;&emsp;} kind;<br>
&emsp;&emsp;std::string text;<br>
&emsp;&emsp;int line = 0; // 0-based<br>
&emsp;};<br>
<br>
&emsp;struct ClassRange<br>
&emsp;{<br>
&emsp;&emsp;Region region;<br>
&emsp;&emsp;std::size_t body_start_token = 0; // индекс '{'<br>
&emsp;&emsp;std::size_t body_end_token = 0;   // индекс '}' (или '};')<br>
&emsp;};<br>
<br>
&emsp;std::vector&lt;std::string&gt; m_lines;<br>
&emsp;std::vector&lt;Token&gt; m_tokens;<br>
<br>
&emsp;void tokenize(const std::string &amp;text);<br>
<br>
&emsp;static bool is_ident_start(char c);<br>
&emsp;static bool is_ident_char(char c);<br>
<br>
&emsp;bool find_class_internal(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;ClassRange &amp;out) const;<br>
};<br>
<br>
// Простейший поисковик Python-символов.<br>
// Умеет:<br>
//   * находить определение класса (class Foo:)<br>
//   * искать методы внутри класса (def bar(self, ...))<br>
class PythonSymbolFinder<br>
{<br>
public:<br>
&emsp;explicit PythonSymbolFinder(const std::string &amp;text);<br>
<br>
&emsp;const std::vector&lt;std::string&gt; &amp;lines() const<br>
&emsp;{<br>
&emsp;&emsp;return m_lines;<br>
&emsp;}<br>
<br>
&emsp;// Находит определение класса (первое вхождение с таким именем).<br>
&emsp;// Region покрывает строку 'class ...' и всё тело класса до dedent.<br>
&emsp;bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
&emsp;// Находит метод внутри класса.<br>
&emsp;// Ищет def / async def с именем method_name,<br>
&emsp;// являющийся &quot;первым уровнем&quot; внутри тела class_name.<br>
&emsp;// Region покрывает строку def (включая декораторы над ней) и всё тело до<br>
&emsp;// dedent.<br>
&emsp;bool find_method(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;method_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const;<br>
<br>
private:<br>
&emsp;std::vector&lt;std::string&gt; m_lines;<br>
<br>
&emsp;static int calc_indent(const std::string &amp;line);<br>
&emsp;static std::size_t first_code_pos(const std::string &amp;line);<br>
<br>
&emsp;bool find_class_internal(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;int &amp;class_indent) const;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

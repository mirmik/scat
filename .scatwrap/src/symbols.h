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
    int start_line = -1; // inclusive<br>
    int end_line = -1;   // inclusive<br>
};<br>
<br>
// Простейший поисковик C++-символов.<br>
// Умеет:<br>
//   * находить определение класса (class / struct)<br>
//   * искать методы внутри класса (по имени)<br>
class CppSymbolFinder<br>
{<br>
public:<br>
    // text — полный текст файла.<br>
    explicit CppSymbolFinder(const std::string &amp;text);<br>
<br>
    const std::vector&lt;std::string&gt; &amp;lines() const<br>
    {<br>
        return m_lines;<br>
    }<br>
<br>
    // Находит определение класса (НЕ forward-declaration).<br>
    // Возвращает true, если класс найден.<br>
    bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
    // Находит метод внутри класса (по имени).<br>
    // Ищет только внутри тела class/struct class_name.<br>
    // Возвращает диапазон строк от начала объявления/определения<br>
    // до ';' или закрывающей '}'.<br>
    bool find_method(const std::string &amp;class_name,<br>
                     const std::string &amp;method_name,<br>
                     Region &amp;out) const;<br>
<br>
private:<br>
    struct Token<br>
    {<br>
        enum Kind<br>
        {<br>
            Identifier,<br>
            Keyword,<br>
            Symbol<br>
        } kind;<br>
        std::string text;<br>
        int line = 0; // 0-based<br>
    };<br>
<br>
    struct ClassRange<br>
    {<br>
        Region region;<br>
        std::size_t body_start_token = 0; // индекс '{'<br>
        std::size_t body_end_token = 0;   // индекс '}' (или '};')<br>
    };<br>
<br>
    std::vector&lt;std::string&gt; m_lines;<br>
    std::vector&lt;Token&gt; m_tokens;<br>
<br>
    void tokenize(const std::string &amp;text);<br>
<br>
    static bool is_ident_start(char c);<br>
    static bool is_ident_char(char c);<br>
<br>
    bool find_class_internal(const std::string &amp;class_name,<br>
                             ClassRange &amp;out) const;<br>
};<br>
<br>
// Простейший поисковик Python-символов.<br>
// Умеет:<br>
//   * находить определение класса (class Foo:)<br>
//   * искать методы внутри класса (def bar(self, ...))<br>
class PythonSymbolFinder<br>
{<br>
public:<br>
    explicit PythonSymbolFinder(const std::string &amp;text);<br>
<br>
    const std::vector&lt;std::string&gt; &amp;lines() const<br>
    {<br>
        return m_lines;<br>
    }<br>
<br>
    // Находит определение класса (первое вхождение с таким именем).<br>
    // Region покрывает строку 'class ...' и всё тело класса до dedent.<br>
    bool find_class(const std::string &amp;class_name, Region &amp;out) const;<br>
<br>
    // Находит метод внутри класса.<br>
    // Ищет def / async def с именем method_name,<br>
    // являющийся &quot;первым уровнем&quot; внутри тела class_name.<br>
    // Region покрывает строку def (включая декораторы над ней) и всё тело до<br>
    // dedent.<br>
    bool find_method(const std::string &amp;class_name,<br>
                     const std::string &amp;method_name,<br>
                     Region &amp;out) const;<br>
<br>
private:<br>
    std::vector&lt;std::string&gt; m_lines;<br>
<br>
    static int calc_indent(const std::string &amp;line);<br>
    static std::size_t first_code_pos(const std::string &amp;line);<br>
<br>
    bool find_class_internal(const std::string &amp;class_name,<br>
                             Region &amp;out,<br>
                             int &amp;class_indent) const;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

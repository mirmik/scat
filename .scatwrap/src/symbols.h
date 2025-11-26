<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/symbols.h</title>
</head>
<body>
<pre><code>
#pragma once

#include &lt;cstddef&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

// Диапазон строк в файле (0-based, включительно).
struct Region
{
    int start_line = -1; // inclusive
    int end_line = -1;   // inclusive
};

// Простейший поисковик C++-символов.
// Умеет:
//   * находить определение класса (class / struct)
//   * искать методы внутри класса (по имени)
class CppSymbolFinder
{
public:
    // text — полный текст файла.
    explicit CppSymbolFinder(const std::string &amp;text);

    const std::vector&lt;std::string&gt; &amp;lines() const
    {
        return m_lines;
    }

    // Находит определение класса (НЕ forward-declaration).
    // Возвращает true, если класс найден.
    bool find_class(const std::string &amp;class_name, Region &amp;out) const;

    // Находит метод внутри класса (по имени).
    // Ищет только внутри тела class/struct class_name.
    // Возвращает диапазон строк от начала объявления/определения
    // до ';' или закрывающей '}'.
    bool find_method(const std::string &amp;class_name,
                     const std::string &amp;method_name,
                     Region &amp;out) const;

private:
    struct Token
    {
        enum Kind
        {
            Identifier,
            Keyword,
            Symbol
        } kind;
        std::string text;
        int line = 0; // 0-based
    };

    struct ClassRange
    {
        Region region;
        std::size_t body_start_token = 0; // индекс '{'
        std::size_t body_end_token = 0;   // индекс '}' (или '};')
    };

    std::vector&lt;std::string&gt; m_lines;
    std::vector&lt;Token&gt; m_tokens;

    void tokenize(const std::string &amp;text);

    static bool is_ident_start(char c);
    static bool is_ident_char(char c);

    bool find_class_internal(const std::string &amp;class_name,
                             ClassRange &amp;out) const;
};

// Простейший поисковик Python-символов.
// Умеет:
//   * находить определение класса (class Foo:)
//   * искать методы внутри класса (def bar(self, ...))
class PythonSymbolFinder
{
public:
    explicit PythonSymbolFinder(const std::string &amp;text);

    const std::vector&lt;std::string&gt; &amp;lines() const
    {
        return m_lines;
    }

    // Находит определение класса (первое вхождение с таким именем).
    // Region покрывает строку 'class ...' и всё тело класса до dedent.
    bool find_class(const std::string &amp;class_name, Region &amp;out) const;

    // Находит метод внутри класса.
    // Ищет def / async def с именем method_name,
    // являющийся &quot;первым уровнем&quot; внутри тела class_name.
    // Region покрывает строку def (включая декораторы над ней) и всё тело до
    // dedent.
    bool find_method(const std::string &amp;class_name,
                     const std::string &amp;method_name,
                     Region &amp;out) const;

private:
    std::vector&lt;std::string&gt; m_lines;

    static int calc_indent(const std::string &amp;line);
    static std::size_t first_code_pos(const std::string &amp;line);

    bool find_class_internal(const std::string &amp;class_name,
                             Region &amp;out,
                             int &amp;class_indent) const;
};
</code></pre>
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/apply_chunk_v2.cpp</title>
</head>
<body>
<pre><code>
#include &quot;symbols.h&quot;
#include &lt;algorithm&gt;
#include &lt;cctype&gt;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;sstream&gt;
#include &lt;stdexcept&gt;
#include &lt;string&gt;
#include &lt;string_view&gt;
#include &lt;vector&gt;

namespace fs = std::filesystem;

struct Section
{
    std::string filepath;
    std::string command;
    int a = -1;
    int b = -1;
    std::vector&lt;std::string&gt; payload;
    std::vector&lt;std::string&gt; marker;
    std::vector&lt;std::string&gt; before; // контекст до маркера (BEFORE:)
    std::vector&lt;std::string&gt; after; // контекст после маркера (AFTER:)
    int seq = 0;
    std::string arg1; // доп. аргументы команды (например, имя класса)
    std::string arg2; // второй аргумент (например, имя метода)
};

static std::vector&lt;std::string&gt; read_file_lines(const fs::path &amp;p)
{
    std::ifstream in(p);
    if (!in)
        throw std::runtime_error(&quot;cannot open file: &quot; + p.string());

    std::vector&lt;std::string&gt; out;
    std::string line;
    while (std::getline(in, line))
        out.push_back(line);

    return out;
}

static void write_file_lines(const fs::path &amp;p,
                             const std::vector&lt;std::string&gt; &amp;lines)
{
    std::ofstream out(p, std::ios::trunc);
    if (!out)
        throw std::runtime_error(&quot;cannot write file: &quot; + p.string());

    for (const auto &amp;s : lines)
        out &lt;&lt; s &lt;&lt; &quot;\n&quot;;
}

std::string trim(const std::string_view &amp;view)
{
    if (view.size() == 0)
        return &quot;&quot;;

    const char *left = view.data();
    const char *right = view.data() + view.size() - 1;
    const char *end = view.data() + view.size();

    while (left != end &amp;&amp;
           (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
        ++left;

    if (left == end)
        return &quot;&quot;;

    while (left != right &amp;&amp; (*right == ' ' || *right == '\n' ||
                             *right == '\r' || *right == '\t'))
        --right;

    return std::string(left, (right - left) + 1);
}

static bool is_text_command(const std::string &amp;cmd)
{
    return cmd == &quot;insert-after-text&quot; || cmd == &quot;insert-before-text&quot; ||
           cmd == &quot;replace-text&quot; || cmd == &quot;delete-text&quot;;
}

static bool is_symbol_command(const std::string &amp;cmd)
{
    return cmd == &quot;replace-cpp-method&quot; || cmd == &quot;replace-cpp-class&quot; ||
           cmd == &quot;replace-py-method&quot; || cmd == &quot;replace-py-class&quot;;
}

static int find_subsequence(const std::vector&lt;std::string&gt; &amp;haystack,
                            const std::vector&lt;std::string&gt; &amp;needle)
{
    if (needle.empty() || needle.size() &gt; haystack.size())
        return -1;

    const std::size_t n = haystack.size();
    const std::size_t m = needle.size();

    for (std::size_t i = 0; i + m &lt;= n; ++i)
    {
        bool ok = true;
        for (std::size_t j = 0; j &lt; m; ++j)
        {
            std::string h = trim(haystack[i + j]);
            std::string nn = trim(needle[j]);
            if (h != nn)
            {
                ok = false;
                break;
            }
        }
        if (ok)
            return static_cast&lt;int&gt;(i);
    }

    return -1;
}

// Строгий выбор позиции маркера с учётом BEFORE/AFTER.
// Никакого fuzzy, только точное позиционное совпадение.
static int find_best_marker_match(const std::vector&lt;std::string&gt; &amp;lines,
                                  const Section *s,
                                  const std::vector&lt;int&gt; &amp;candidates)
{
    if (candidates.empty())
        return -1;

    // Нет дополнительного контекста — ведём себя как раньше.
    if (s-&gt;before.empty() &amp;&amp; s-&gt;after.empty())
        return candidates.front();

    auto trim_eq = [&amp;](const std::string &amp;a, const std::string &amp;b)
    { return trim(a) == trim(b); };

    std::vector&lt;int&gt; strict;

    for (int pos : candidates)
    {
        bool ok = true;

        // BEFORE: строки сразу над маркером
        if (!s-&gt;before.empty())
        {
            int need = static_cast&lt;int&gt;(s-&gt;before.size());
            if (pos &lt; need)
            {
                ok = false;
            }
            else
            {
                // Последняя строка BEFORE должна быть непосредственно над
                // первой строкой маркера.
                for (int i = 0; i &lt; need; ++i)
                {
                    const std::string &amp;want =
                        s-&gt;before[static_cast&lt;std::size_t&gt;(need - 1 - i)];
                    const std::string &amp;got =
                        lines[static_cast&lt;std::size_t&gt;(pos - 1 - i)];
                    if (!trim_eq(got, want))
                    {
                        ok = false;
                        break;
                    }
                }
            }
        }

        // AFTER: строки сразу под маркером
        if (ok &amp;&amp; !s-&gt;after.empty())
        {
            int start = pos + static_cast&lt;int&gt;(s-&gt;marker.size());
            int need = static_cast&lt;int&gt;(s-&gt;after.size());

            if (start &lt; 0 || start + need &gt; static_cast&lt;int&gt;(lines.size()))
            {
                ok = false;
            }
            else
            {
                for (int i = 0; i &lt; need; ++i)
                {
                    const std::string &amp;want =
                        s-&gt;after[static_cast&lt;std::size_t&gt;(i)];
                    const std::string &amp;got =
                        lines[static_cast&lt;std::size_t&gt;(start + i)];
                    if (!trim_eq(got, want))
                    {
                        ok = false;
                        break;
                    }
                }
            }
        }

        if (ok)
            strict.push_back(pos);
    }

    if (strict.empty())
        throw std::runtime_error(&quot;strict marker context not found&quot;);

    if (strict.size() &gt; 1)
        throw std::runtime_error(&quot;strict marker match is ambiguous&quot;);

    return strict.front();
}

static void apply_text_commands(const std::string &amp;filepath,
                                std::vector&lt;std::string&gt; &amp;lines,
                                const std::vector&lt;const Section *&gt; &amp;sections)
{
    for (const Section *s : sections)
    {
        if (s-&gt;marker.empty())
            throw std::runtime_error(&quot;empty marker in text command for file: &quot; +
                                     filepath);

        // Собираем все вхождения маркера
        std::vector&lt;int&gt; candidates;
        {
            int base = 0;
            while (base &lt; static_cast&lt;int&gt;(lines.size()))
            {
                std::vector&lt;std::string&gt; sub(lines.begin() + base, lines.end());
                int idx = find_subsequence(sub, s-&gt;marker);
                if (idx &lt; 0)
                    break;
                candidates.push_back(base + idx);
                base += idx + 1;
            }
        }

        if (candidates.empty())
            throw std::runtime_error(
                &quot;text marker not found for file: &quot; + filepath +
                &quot;\ncommand: &quot; + s-&gt;command + &quot;\n&quot;);

        int idx = find_best_marker_match(lines, s, candidates);
        if (idx &lt; 0)
            throw std::runtime_error(&quot;cannot locate marker uniquely&quot;);

        std::size_t pos = static_cast&lt;std::size_t&gt;(idx);

        if (s-&gt;command == &quot;insert-after-text&quot;)
        {
            pos += s-&gt;marker.size();
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),
                         s-&gt;payload.begin(),
                         s-&gt;payload.end());
        }
        else if (s-&gt;command == &quot;insert-before-text&quot;)
        {
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),
                         s-&gt;payload.begin(),
                         s-&gt;payload.end());
        }
        else if (s-&gt;command == &quot;replace-text&quot;)
        {
            auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);
            auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());
            lines.erase(begin, end);
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),
                         s-&gt;payload.begin(),
                         s-&gt;payload.end());
        }
        else if (s-&gt;command == &quot;delete-text&quot;)
        {
            auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);
            auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());
            lines.erase(begin, end);
        }
        else
        {
            throw std::runtime_error(&quot;unknown text command: &quot; + s-&gt;command);
        }
    }
}

static std::string join_lines(const std::vector&lt;std::string&gt; &amp;lines)
{
    if (lines.empty())
        return std::string();

    std::string text;
    std::size_t total = 0;
    for (const auto &amp;s : lines)
        total += s.size() + 1;
    text.reserve(total);

    for (std::size_t i = 0; i &lt; lines.size(); ++i)
    {
        text += lines[i];
        if (i + 1 &lt; lines.size())
            text += '\n';
    }

    return text;
}

static void apply_symbol_commands(const std::string &amp;filepath,
                                  std::vector&lt;std::string&gt; &amp;lines,
                                  const std::vector&lt;const Section *&gt; &amp;sections)
{
    for (const Section *s : sections)
    {
        // Всегда работаем с текущей версией файла
        std::string text = join_lines(lines);

        if (s-&gt;command == &quot;replace-cpp-class&quot; ||
            s-&gt;command == &quot;replace-cpp-method&quot;)
        {
            CppSymbolFinder finder(text);

            if (s-&gt;command == &quot;replace-cpp-class&quot;)
            {
                if (s-&gt;arg1.empty())
                    throw std::runtime_error(
                        &quot;replace-cpp-class: missing class name for file: &quot; +
                        filepath);

                Region r;
                if (!finder.find_class(s-&gt;arg1, r))
                    throw std::runtime_error(
                        &quot;replace-cpp-class: class not found: &quot; + s-&gt;arg1 +
                        &quot; in file: &quot; + filepath);

                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))
                    throw std::runtime_error(
                        &quot;replace-cpp-class: invalid region&quot;);

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s-&gt;payload.begin(),
                             s-&gt;payload.end());
            }
            else // replace-cpp-method
            {
                std::string cls;
                std::string method;

                if (!s-&gt;arg2.empty())
                {
                    cls = s-&gt;arg1;
                    method = s-&gt;arg2;
                }
                else
                {
                    auto pos = s-&gt;arg1.find(&quot;::&quot;);
                    if (pos == std::string::npos)
                        throw std::runtime_error(
                            &quot;replace-cpp-method: expected 'Class::method' or &quot;
                            &quot;'Class method'&quot;);

                    cls = s-&gt;arg1.substr(0, pos);
                    method = s-&gt;arg1.substr(pos + 2);
                }

                if (cls.empty() || method.empty())
                    throw std::runtime_error(
                        &quot;replace-cpp-method: empty class or method name&quot;);

                Region r;
                if (!finder.find_method(cls, method, r))
                    throw std::runtime_error(
                        &quot;replace-cpp-method: method not found: &quot; + cls +
                        &quot;::&quot; + method + &quot; in file: &quot; + filepath);

                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))
                    throw std::runtime_error(
                        &quot;replace-cpp-method: invalid region&quot;);

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s-&gt;payload.begin(),
                             s-&gt;payload.end());
            }

            continue;
        }

        if (s-&gt;command == &quot;replace-py-class&quot; ||
            s-&gt;command == &quot;replace-py-method&quot;)
        {
            PythonSymbolFinder finder(text);

            if (s-&gt;command == &quot;replace-py-class&quot;)
            {
                if (s-&gt;arg1.empty())
                    throw std::runtime_error(
                        &quot;replace-py-class: missing class name for file: &quot; +
                        filepath);

                Region r;
                if (!finder.find_class(s-&gt;arg1, r))
                    throw std::runtime_error(
                        &quot;replace-py-class: class not found: &quot; + s-&gt;arg1 +
                        &quot; in file: &quot; + filepath);

                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))
                    throw std::runtime_error(
                        &quot;replace-py-class: invalid region&quot;);

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s-&gt;payload.begin(),
                             s-&gt;payload.end());
            }
            else // replace-py-method
            {
                std::string cls;
                std::string method;

                if (!s-&gt;arg2.empty())
                {
                    cls = s-&gt;arg1;
                    method = s-&gt;arg2;
                }
                else
                {
                    auto pos = s-&gt;arg1.find('.');
                    if (pos == std::string::npos)
                        throw std::runtime_error(
                            &quot;replace-py-method: expected 'Class.method' or &quot;
                            &quot;'Class method'&quot;);

                    cls = s-&gt;arg1.substr(0, pos);
                    method = s-&gt;arg1.substr(pos + 1);
                }

                if (cls.empty() || method.empty())
                    throw std::runtime_error(
                        &quot;replace-py-method: empty class or method name&quot;);

                Region r;
                if (!finder.find_method(cls, method, r))
                    throw std::runtime_error(
                        &quot;replace-py-method: method not found: &quot; + cls + &quot;.&quot; +
                        method + &quot; in file: &quot; + filepath);

                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))
                    throw std::runtime_error(
                        &quot;replace-py-method: invalid region&quot;);

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s-&gt;payload.begin(),
                             s-&gt;payload.end());
            }

            continue;
        }

        throw std::runtime_error(&quot;apply_symbol_commands: unknown command: &quot; +
                                 s-&gt;command);
    }
}

static Section parse_section(std::istream &amp;in, const std::string &amp;header)
{
    Section s;

    auto pos = header.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error(&quot;bad section header: &quot; + header);

    auto pos2 = header.find(&quot;===&quot;, pos);
    if (pos2 == std::string::npos)
        pos2 = header.size();

    auto raw = header.substr(pos + 1, pos2 - pos - 1);
    s.filepath = trim(raw);
    if (s.filepath.empty())
        throw std::runtime_error(&quot;empty filepath in header: &quot; + header);

    std::string line;
    if (!std::getline(in, line))
        throw std::runtime_error(&quot;unexpected end after header&quot;);

    if (line.rfind(&quot;--- &quot;, 0) != 0)
        throw std::runtime_error(&quot;expected command after header&quot;);

    {
        std::istringstream ss(line.substr(4));
        ss &gt;&gt; s.command;

        // читаем остаток строки как аргументы команды
        std::string rest;
        std::getline(ss, rest);
        if (!rest.empty())
        {
            std::istringstream as(rest);
            as &gt;&gt; s.arg1;
            as &gt;&gt; s.arg2;
        }

        if (is_text_command(s.command))
        {
        }
        else if (s.command == &quot;create-file&quot; || s.command == &quot;delete-file&quot;)
        {
        }
        else if (is_symbol_command(s.command))
        {
        }
        else
        {
            throw std::runtime_error(&quot;index-based commands removed: &quot; +
                                     s.command);
        }
    }

    bool found_end = false;
    while (std::getline(in, line))
    {
        if (line == &quot;=END=&quot;)
        {
            found_end = true;
            break;
        }
        s.payload.push_back(line);
    }

    if (!found_end)
        throw std::runtime_error(&quot;missing =END=&quot;);

    if (is_text_command(s.command))
    {
        // Определяем, в YAML-режиме мы или в старом формате.
        // Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется
        // старая логика.
        bool yaml_mode = false;
        std::size_t first_non_empty = 0;
        while (first_non_empty &lt; s.payload.size() &amp;&amp;
               trim(s.payload[first_non_empty]).empty())
            ++first_non_empty;

        if (first_non_empty &lt; s.payload.size())
        {
            const std::string t = trim(s.payload[first_non_empty]);
            if (t == &quot;BEFORE:&quot; || t == &quot;MARKER:&quot; || t == &quot;AFTER:&quot;)
                yaml_mode = true;
        }

        if (!yaml_mode)
        {
            // Старый режим: всё до '---' — marker, после — payload
            auto it = std::find(
                s.payload.begin(), s.payload.end(), std::string(&quot;---&quot;));
            if (it == s.payload.end())
                throw std::runtime_error(
                    &quot;text command requires '---' separator&quot;);

            s.marker.assign(s.payload.begin(), it);

            std::vector&lt;std::string&gt; tail;
            if (std::next(it) != s.payload.end())
                tail.assign(std::next(it), s.payload.end());

            s.payload.swap(tail);

            if (s.marker.empty())
                throw std::runtime_error(&quot;empty text marker&quot;);
        }
        else
        {
            // YAML-подобный режим:
            // BEFORE:
            //   ...
            // MARKER:
            //   ...
            // AFTER:
            //   ...
            // ---
            // &lt;payload&gt;
            s.before.clear();
            s.marker.clear();
            s.after.clear();

            enum class Block
            {
                NONE,
                BEFORE,
                MARKER,
                AFTER
            };

            Block blk = Block::NONE;
            std::vector&lt;std::string&gt; new_payload;

            bool seen_separator = false;

            for (std::size_t i = first_non_empty; i &lt; s.payload.size(); ++i)
            {
                const std::string &amp;ln = s.payload[i];

                if (!seen_separator &amp;&amp; ln == &quot;---&quot;)
                {
                    seen_separator = true;
                    continue;
                }

                if (!seen_separator)
                {
                    const std::string t = trim(ln);
                    if (t == &quot;BEFORE:&quot;)
                    {
                        blk = Block::BEFORE;
                        continue;
                    }
                    if (t == &quot;MARKER:&quot;)
                    {
                        blk = Block::MARKER;
                        continue;
                    }
                    if (t == &quot;AFTER:&quot;)
                    {
                        blk = Block::AFTER;
                        continue;
                    }

                    switch (blk)
                    {
                    case Block::BEFORE:
                        s.before.push_back(ln);
                        break;
                    case Block::MARKER:
                        s.marker.push_back(ln);
                        break;
                    case Block::AFTER:
                        s.after.push_back(ln);
                        break;
                    case Block::NONE:
                        throw std::runtime_error(
                            &quot;unexpected content before YAML block tag&quot;);
                    }
                }
                else
                {
                    new_payload.push_back(ln);
                }
            }

            s.payload.swap(new_payload);

            if (s.marker.empty())
                throw std::runtime_error(
                    &quot;YAML text command requires MARKER: section&quot;);
        }
    }

    return s;
}

static void apply_for_file(const std::string &amp;filepath,
                           const std::vector&lt;const Section *&gt; &amp;sections)
{
    fs::path p = filepath;
    std::vector&lt;std::string&gt; orig;
    bool existed = true;

    try
    {
        orig = read_file_lines(p);
    }
    catch (...)
    {
        existed = false;
        orig.clear();
    }

    for (const Section *s : sections)
    {
        if (!existed &amp;&amp; s-&gt;command == &quot;delete-file&quot;)
            throw std::runtime_error(&quot;delete-file: file does not exist&quot;);
    }

    for (const Section *s : sections)
    {
        if (s-&gt;command == &quot;create-file&quot;)
        {
            write_file_lines(p, s-&gt;payload);
            return;
        }
        if (s-&gt;command == &quot;delete-file&quot;)
        {
            std::error_code ec;
            fs::remove(p, ec);
            if (ec)
                throw std::runtime_error(&quot;delete-file failed&quot;);
            return;
        }
    }

    std::vector&lt;const Section *&gt; text_sections;
    std::vector&lt;const Section *&gt; symbol_sections;

    for (const Section *s : sections)
    {
        if (is_text_command(s-&gt;command))
            text_sections.push_back(s);
        else if (is_symbol_command(s-&gt;command))
            symbol_sections.push_back(s);
        else
            throw std::runtime_error(&quot;unexpected non-text command: &quot; +
                                     s-&gt;command);
    }

    if (!text_sections.empty())
        apply_text_commands(filepath, orig, text_sections);

    if (!symbol_sections.empty())
        apply_symbol_commands(filepath, orig, symbol_sections);

    if (!text_sections.empty() || !symbol_sections.empty())
        write_file_lines(p, orig);
}

static void apply_all(const std::vector&lt;Section&gt; &amp;sections)
{
    namespace fs = std::filesystem;

    // 1. Собираем список всех файлов, которые будут затронуты
    std::vector&lt;std::string&gt; files;
    files.reserve(sections.size());
    for (auto &amp;s : sections)
        files.push_back(s.filepath);

    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());

    struct Backup
    {
        bool existed = false;
        std::vector&lt;std::string&gt; lines;
    };

    std::map&lt;std::string, Backup&gt; backup;

    // 2. Делаем резервную копию всех файлов
    for (auto &amp;f : files)
    {
        Backup b;
        fs::path p = f;

        std::error_code ec;

        if (fs::exists(p, ec))
        {
            b.existed = true;

            try
            {
                b.lines = read_file_lines(p);
            }
            catch (...)
            {
                throw std::runtime_error(&quot;cannot read original file: &quot; + f);
            }
        }
        else
        {
            b.existed = false;
        }

        backup[f] = std::move(b);
    }

    // 3. Применяем секции с защитой (try/catch)
    try
    {
        for (auto &amp;s : sections)
        {
            std::vector&lt;const Section *&gt; single{&amp;s};
            apply_for_file(s.filepath, single);
        }
    }
    catch (...)
    {
        // 4. Откат (rollback)
        for (auto &amp;[path, b] : backup)
        {
            fs::path p = path;
            std::error_code ec;

            if (b.existed)
            {
                try
                {
                    write_file_lines(p, b.lines);
                }
                catch (...)
                {
                    // если даже откат не удался — сдаёмся
                }
            }
            else
            {
                fs::remove(p, ec);
            }
        }

        throw;
    }
}

int apply_chunk_main(int argc, char **argv)
{
    if (argc &lt; 2)
    {
        std::cerr &lt;&lt; &quot;usage: apply_patch &lt;patchfile&gt;\n&quot;;
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in)
    {
        std::cerr &lt;&lt; &quot;cannot open patch file: &quot; &lt;&lt; argv[1] &lt;&lt; &quot;\n&quot;;
        return 1;
    }

    std::string line;
    std::vector&lt;Section&gt; sections;
    int seq = 0;

    try
    {
        while (std::getline(in, line))
        {
            if (line.rfind(&quot;=== file:&quot;, 0) == 0)
            {
                Section s = parse_section(in, line);
                s.seq = seq++;
                sections.push_back(std::move(s));
            }
        }

        apply_all(sections);
    }
    catch (const std::exception &amp;e)
    {
        std::cerr &lt;&lt; &quot;error while applying patch: &quot; &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;
        return 1;
    }

    return 0;
}
</code></pre>
</body>
</html>

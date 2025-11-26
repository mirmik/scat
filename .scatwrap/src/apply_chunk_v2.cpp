<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/apply_chunk_v2.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;symbols.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;cctype&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;map&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;stdexcept&gt;<br>
#include &lt;string&gt;<br>
#include &lt;string_view&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
struct Section<br>
{<br>
    std::string filepath;<br>
    std::string command;<br>
    int a = -1;<br>
    int b = -1;<br>
    std::vector&lt;std::string&gt; payload;<br>
    std::vector&lt;std::string&gt; marker;<br>
    std::vector&lt;std::string&gt; before; // контекст до маркера (BEFORE:)<br>
    std::vector&lt;std::string&gt; after; // контекст после маркера (AFTER:)<br>
    int seq = 0;<br>
    std::string arg1; // доп. аргументы команды (например, имя класса)<br>
    std::string arg2; // второй аргумент (например, имя метода)<br>
};<br>
<br>
static std::vector&lt;std::string&gt; read_file_lines(const fs::path &amp;p)<br>
{<br>
    std::ifstream in(p);<br>
    if (!in)<br>
        throw std::runtime_error(&quot;cannot open file: &quot; + p.string());<br>
<br>
    std::vector&lt;std::string&gt; out;<br>
    std::string line;<br>
    while (std::getline(in, line))<br>
        out.push_back(line);<br>
<br>
    return out;<br>
}<br>
<br>
static void write_file_lines(const fs::path &amp;p,<br>
                             const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
    std::ofstream out(p, std::ios::trunc);<br>
    if (!out)<br>
        throw std::runtime_error(&quot;cannot write file: &quot; + p.string());<br>
<br>
    for (const auto &amp;s : lines)<br>
        out &lt;&lt; s &lt;&lt; &quot;\n&quot;;<br>
}<br>
<br>
std::string trim(const std::string_view &amp;view)<br>
{<br>
    if (view.size() == 0)<br>
        return &quot;&quot;;<br>
<br>
    const char *left = view.data();<br>
    const char *right = view.data() + view.size() - 1;<br>
    const char *end = view.data() + view.size();<br>
<br>
    while (left != end &amp;&amp;<br>
           (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))<br>
        ++left;<br>
<br>
    if (left == end)<br>
        return &quot;&quot;;<br>
<br>
    while (left != right &amp;&amp; (*right == ' ' || *right == '\n' ||<br>
                             *right == '\r' || *right == '\t'))<br>
        --right;<br>
<br>
    return std::string(left, (right - left) + 1);<br>
}<br>
<br>
static bool is_text_command(const std::string &amp;cmd)<br>
{<br>
    return cmd == &quot;insert-after-text&quot; || cmd == &quot;insert-before-text&quot; ||<br>
           cmd == &quot;replace-text&quot; || cmd == &quot;delete-text&quot;;<br>
}<br>
<br>
static bool is_symbol_command(const std::string &amp;cmd)<br>
{<br>
    return cmd == &quot;replace-cpp-method&quot; || cmd == &quot;replace-cpp-class&quot; ||<br>
           cmd == &quot;replace-py-method&quot; || cmd == &quot;replace-py-class&quot;;<br>
}<br>
<br>
static int find_subsequence(const std::vector&lt;std::string&gt; &amp;haystack,<br>
                            const std::vector&lt;std::string&gt; &amp;needle)<br>
{<br>
    if (needle.empty() || needle.size() &gt; haystack.size())<br>
        return -1;<br>
<br>
    const std::size_t n = haystack.size();<br>
    const std::size_t m = needle.size();<br>
<br>
    for (std::size_t i = 0; i + m &lt;= n; ++i)<br>
    {<br>
        bool ok = true;<br>
        for (std::size_t j = 0; j &lt; m; ++j)<br>
        {<br>
            std::string h = trim(haystack[i + j]);<br>
            std::string nn = trim(needle[j]);<br>
            if (h != nn)<br>
            {<br>
                ok = false;<br>
                break;<br>
            }<br>
        }<br>
        if (ok)<br>
            return static_cast&lt;int&gt;(i);<br>
    }<br>
<br>
    return -1;<br>
}<br>
<br>
// Строгий выбор позиции маркера с учётом BEFORE/AFTER.<br>
// Никакого fuzzy, только точное позиционное совпадение.<br>
static int find_best_marker_match(const std::vector&lt;std::string&gt; &amp;lines,<br>
                                  const Section *s,<br>
                                  const std::vector&lt;int&gt; &amp;candidates)<br>
{<br>
    if (candidates.empty())<br>
        return -1;<br>
<br>
    // Нет дополнительного контекста — ведём себя как раньше.<br>
    if (s-&gt;before.empty() &amp;&amp; s-&gt;after.empty())<br>
        return candidates.front();<br>
<br>
    auto trim_eq = [&amp;](const std::string &amp;a, const std::string &amp;b)<br>
    { return trim(a) == trim(b); };<br>
<br>
    std::vector&lt;int&gt; strict;<br>
<br>
    for (int pos : candidates)<br>
    {<br>
        bool ok = true;<br>
<br>
        // BEFORE: строки сразу над маркером<br>
        if (!s-&gt;before.empty())<br>
        {<br>
            int need = static_cast&lt;int&gt;(s-&gt;before.size());<br>
            if (pos &lt; need)<br>
            {<br>
                ok = false;<br>
            }<br>
            else<br>
            {<br>
                // Последняя строка BEFORE должна быть непосредственно над<br>
                // первой строкой маркера.<br>
                for (int i = 0; i &lt; need; ++i)<br>
                {<br>
                    const std::string &amp;want =<br>
                        s-&gt;before[static_cast&lt;std::size_t&gt;(need - 1 - i)];<br>
                    const std::string &amp;got =<br>
                        lines[static_cast&lt;std::size_t&gt;(pos - 1 - i)];<br>
                    if (!trim_eq(got, want))<br>
                    {<br>
                        ok = false;<br>
                        break;<br>
                    }<br>
                }<br>
            }<br>
        }<br>
<br>
        // AFTER: строки сразу под маркером<br>
        if (ok &amp;&amp; !s-&gt;after.empty())<br>
        {<br>
            int start = pos + static_cast&lt;int&gt;(s-&gt;marker.size());<br>
            int need = static_cast&lt;int&gt;(s-&gt;after.size());<br>
<br>
            if (start &lt; 0 || start + need &gt; static_cast&lt;int&gt;(lines.size()))<br>
            {<br>
                ok = false;<br>
            }<br>
            else<br>
            {<br>
                for (int i = 0; i &lt; need; ++i)<br>
                {<br>
                    const std::string &amp;want =<br>
                        s-&gt;after[static_cast&lt;std::size_t&gt;(i)];<br>
                    const std::string &amp;got =<br>
                        lines[static_cast&lt;std::size_t&gt;(start + i)];<br>
                    if (!trim_eq(got, want))<br>
                    {<br>
                        ok = false;<br>
                        break;<br>
                    }<br>
                }<br>
            }<br>
        }<br>
<br>
        if (ok)<br>
            strict.push_back(pos);<br>
    }<br>
<br>
    if (strict.empty())<br>
        throw std::runtime_error(&quot;strict marker context not found&quot;);<br>
<br>
    if (strict.size() &gt; 1)<br>
        throw std::runtime_error(&quot;strict marker match is ambiguous&quot;);<br>
<br>
    return strict.front();<br>
}<br>
<br>
static void apply_text_commands(const std::string &amp;filepath,<br>
                                std::vector&lt;std::string&gt; &amp;lines,<br>
                                const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
    for (const Section *s : sections)<br>
    {<br>
        if (s-&gt;marker.empty())<br>
            throw std::runtime_error(&quot;empty marker in text command for file: &quot; +<br>
                                     filepath);<br>
<br>
        // Собираем все вхождения маркера<br>
        std::vector&lt;int&gt; candidates;<br>
        {<br>
            int base = 0;<br>
            while (base &lt; static_cast&lt;int&gt;(lines.size()))<br>
            {<br>
                std::vector&lt;std::string&gt; sub(lines.begin() + base, lines.end());<br>
                int idx = find_subsequence(sub, s-&gt;marker);<br>
                if (idx &lt; 0)<br>
                    break;<br>
                candidates.push_back(base + idx);<br>
                base += idx + 1;<br>
            }<br>
        }<br>
<br>
        if (candidates.empty())<br>
            throw std::runtime_error(<br>
                &quot;text marker not found for file: &quot; + filepath +<br>
                &quot;\ncommand: &quot; + s-&gt;command + &quot;\n&quot;);<br>
<br>
        int idx = find_best_marker_match(lines, s, candidates);<br>
        if (idx &lt; 0)<br>
            throw std::runtime_error(&quot;cannot locate marker uniquely&quot;);<br>
<br>
        std::size_t pos = static_cast&lt;std::size_t&gt;(idx);<br>
<br>
        if (s-&gt;command == &quot;insert-after-text&quot;)<br>
        {<br>
            pos += s-&gt;marker.size();<br>
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
                         s-&gt;payload.begin(),<br>
                         s-&gt;payload.end());<br>
        }<br>
        else if (s-&gt;command == &quot;insert-before-text&quot;)<br>
        {<br>
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
                         s-&gt;payload.begin(),<br>
                         s-&gt;payload.end());<br>
        }<br>
        else if (s-&gt;command == &quot;replace-text&quot;)<br>
        {<br>
            auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
            auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
            lines.erase(begin, end);<br>
            lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
                         s-&gt;payload.begin(),<br>
                         s-&gt;payload.end());<br>
        }<br>
        else if (s-&gt;command == &quot;delete-text&quot;)<br>
        {<br>
            auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
            auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
            lines.erase(begin, end);<br>
        }<br>
        else<br>
        {<br>
            throw std::runtime_error(&quot;unknown text command: &quot; + s-&gt;command);<br>
        }<br>
    }<br>
}<br>
<br>
static std::string join_lines(const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
    if (lines.empty())<br>
        return std::string();<br>
<br>
    std::string text;<br>
    std::size_t total = 0;<br>
    for (const auto &amp;s : lines)<br>
        total += s.size() + 1;<br>
    text.reserve(total);<br>
<br>
    for (std::size_t i = 0; i &lt; lines.size(); ++i)<br>
    {<br>
        text += lines[i];<br>
        if (i + 1 &lt; lines.size())<br>
            text += '\n';<br>
    }<br>
<br>
    return text;<br>
}<br>
<br>
static void apply_symbol_commands(const std::string &amp;filepath,<br>
                                  std::vector&lt;std::string&gt; &amp;lines,<br>
                                  const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
    for (const Section *s : sections)<br>
    {<br>
        // Всегда работаем с текущей версией файла<br>
        std::string text = join_lines(lines);<br>
<br>
        if (s-&gt;command == &quot;replace-cpp-class&quot; ||<br>
            s-&gt;command == &quot;replace-cpp-method&quot;)<br>
        {<br>
            CppSymbolFinder finder(text);<br>
<br>
            if (s-&gt;command == &quot;replace-cpp-class&quot;)<br>
            {<br>
                if (s-&gt;arg1.empty())<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-class: missing class name for file: &quot; +<br>
                        filepath);<br>
<br>
                Region r;<br>
                if (!finder.find_class(s-&gt;arg1, r))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-class: class not found: &quot; + s-&gt;arg1 +<br>
                        &quot; in file: &quot; + filepath);<br>
<br>
                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-class: invalid region&quot;);<br>
<br>
                auto begin = lines.begin() + r.start_line;<br>
                auto end = lines.begin() + (r.end_line + 1);<br>
                lines.erase(begin, end);<br>
                lines.insert(lines.begin() + r.start_line,<br>
                             s-&gt;payload.begin(),<br>
                             s-&gt;payload.end());<br>
            }<br>
            else // replace-cpp-method<br>
            {<br>
                std::string cls;<br>
                std::string method;<br>
<br>
                if (!s-&gt;arg2.empty())<br>
                {<br>
                    cls = s-&gt;arg1;<br>
                    method = s-&gt;arg2;<br>
                }<br>
                else<br>
                {<br>
                    auto pos = s-&gt;arg1.find(&quot;::&quot;);<br>
                    if (pos == std::string::npos)<br>
                        throw std::runtime_error(<br>
                            &quot;replace-cpp-method: expected 'Class::method' or &quot;<br>
                            &quot;'Class method'&quot;);<br>
<br>
                    cls = s-&gt;arg1.substr(0, pos);<br>
                    method = s-&gt;arg1.substr(pos + 2);<br>
                }<br>
<br>
                if (cls.empty() || method.empty())<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-method: empty class or method name&quot;);<br>
<br>
                Region r;<br>
                if (!finder.find_method(cls, method, r))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-method: method not found: &quot; + cls +<br>
                        &quot;::&quot; + method + &quot; in file: &quot; + filepath);<br>
<br>
                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-cpp-method: invalid region&quot;);<br>
<br>
                auto begin = lines.begin() + r.start_line;<br>
                auto end = lines.begin() + (r.end_line + 1);<br>
                lines.erase(begin, end);<br>
                lines.insert(lines.begin() + r.start_line,<br>
                             s-&gt;payload.begin(),<br>
                             s-&gt;payload.end());<br>
            }<br>
<br>
            continue;<br>
        }<br>
<br>
        if (s-&gt;command == &quot;replace-py-class&quot; ||<br>
            s-&gt;command == &quot;replace-py-method&quot;)<br>
        {<br>
            PythonSymbolFinder finder(text);<br>
<br>
            if (s-&gt;command == &quot;replace-py-class&quot;)<br>
            {<br>
                if (s-&gt;arg1.empty())<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-class: missing class name for file: &quot; +<br>
                        filepath);<br>
<br>
                Region r;<br>
                if (!finder.find_class(s-&gt;arg1, r))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-class: class not found: &quot; + s-&gt;arg1 +<br>
                        &quot; in file: &quot; + filepath);<br>
<br>
                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-class: invalid region&quot;);<br>
<br>
                auto begin = lines.begin() + r.start_line;<br>
                auto end = lines.begin() + (r.end_line + 1);<br>
                lines.erase(begin, end);<br>
                lines.insert(lines.begin() + r.start_line,<br>
                             s-&gt;payload.begin(),<br>
                             s-&gt;payload.end());<br>
            }<br>
            else // replace-py-method<br>
            {<br>
                std::string cls;<br>
                std::string method;<br>
<br>
                if (!s-&gt;arg2.empty())<br>
                {<br>
                    cls = s-&gt;arg1;<br>
                    method = s-&gt;arg2;<br>
                }<br>
                else<br>
                {<br>
                    auto pos = s-&gt;arg1.find('.');<br>
                    if (pos == std::string::npos)<br>
                        throw std::runtime_error(<br>
                            &quot;replace-py-method: expected 'Class.method' or &quot;<br>
                            &quot;'Class method'&quot;);<br>
<br>
                    cls = s-&gt;arg1.substr(0, pos);<br>
                    method = s-&gt;arg1.substr(pos + 1);<br>
                }<br>
<br>
                if (cls.empty() || method.empty())<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-method: empty class or method name&quot;);<br>
<br>
                Region r;<br>
                if (!finder.find_method(cls, method, r))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-method: method not found: &quot; + cls + &quot;.&quot; +<br>
                        method + &quot; in file: &quot; + filepath);<br>
<br>
                if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
                    r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
                    throw std::runtime_error(<br>
                        &quot;replace-py-method: invalid region&quot;);<br>
<br>
                auto begin = lines.begin() + r.start_line;<br>
                auto end = lines.begin() + (r.end_line + 1);<br>
                lines.erase(begin, end);<br>
                lines.insert(lines.begin() + r.start_line,<br>
                             s-&gt;payload.begin(),<br>
                             s-&gt;payload.end());<br>
            }<br>
<br>
            continue;<br>
        }<br>
<br>
        throw std::runtime_error(&quot;apply_symbol_commands: unknown command: &quot; +<br>
                                 s-&gt;command);<br>
    }<br>
}<br>
<br>
static Section parse_section(std::istream &amp;in, const std::string &amp;header)<br>
{<br>
    Section s;<br>
<br>
    auto pos = header.find(':');<br>
    if (pos == std::string::npos)<br>
        throw std::runtime_error(&quot;bad section header: &quot; + header);<br>
<br>
    auto pos2 = header.find(&quot;===&quot;, pos);<br>
    if (pos2 == std::string::npos)<br>
        pos2 = header.size();<br>
<br>
    auto raw = header.substr(pos + 1, pos2 - pos - 1);<br>
    s.filepath = trim(raw);<br>
    if (s.filepath.empty())<br>
        throw std::runtime_error(&quot;empty filepath in header: &quot; + header);<br>
<br>
    std::string line;<br>
    if (!std::getline(in, line))<br>
        throw std::runtime_error(&quot;unexpected end after header&quot;);<br>
<br>
    if (line.rfind(&quot;--- &quot;, 0) != 0)<br>
        throw std::runtime_error(&quot;expected command after header&quot;);<br>
<br>
    {<br>
        std::istringstream ss(line.substr(4));<br>
        ss &gt;&gt; s.command;<br>
<br>
        // читаем остаток строки как аргументы команды<br>
        std::string rest;<br>
        std::getline(ss, rest);<br>
        if (!rest.empty())<br>
        {<br>
            std::istringstream as(rest);<br>
            as &gt;&gt; s.arg1;<br>
            as &gt;&gt; s.arg2;<br>
        }<br>
<br>
        if (is_text_command(s.command))<br>
        {<br>
        }<br>
        else if (s.command == &quot;create-file&quot; || s.command == &quot;delete-file&quot;)<br>
        {<br>
        }<br>
        else if (is_symbol_command(s.command))<br>
        {<br>
        }<br>
        else<br>
        {<br>
            throw std::runtime_error(&quot;index-based commands removed: &quot; +<br>
                                     s.command);<br>
        }<br>
    }<br>
<br>
    bool found_end = false;<br>
    while (std::getline(in, line))<br>
    {<br>
        if (line == &quot;=END=&quot;)<br>
        {<br>
            found_end = true;<br>
            break;<br>
        }<br>
        s.payload.push_back(line);<br>
    }<br>
<br>
    if (!found_end)<br>
        throw std::runtime_error(&quot;missing =END=&quot;);<br>
<br>
    if (is_text_command(s.command))<br>
    {<br>
        // Определяем, в YAML-режиме мы или в старом формате.<br>
        // Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется<br>
        // старая логика.<br>
        bool yaml_mode = false;<br>
        std::size_t first_non_empty = 0;<br>
        while (first_non_empty &lt; s.payload.size() &amp;&amp;<br>
               trim(s.payload[first_non_empty]).empty())<br>
            ++first_non_empty;<br>
<br>
        if (first_non_empty &lt; s.payload.size())<br>
        {<br>
            const std::string t = trim(s.payload[first_non_empty]);<br>
            if (t == &quot;BEFORE:&quot; || t == &quot;MARKER:&quot; || t == &quot;AFTER:&quot;)<br>
                yaml_mode = true;<br>
        }<br>
<br>
        if (!yaml_mode)<br>
        {<br>
            // Старый режим: всё до '---' — marker, после — payload<br>
            auto it = std::find(<br>
                s.payload.begin(), s.payload.end(), std::string(&quot;---&quot;));<br>
            if (it == s.payload.end())<br>
                throw std::runtime_error(<br>
                    &quot;text command requires '---' separator&quot;);<br>
<br>
            s.marker.assign(s.payload.begin(), it);<br>
<br>
            std::vector&lt;std::string&gt; tail;<br>
            if (std::next(it) != s.payload.end())<br>
                tail.assign(std::next(it), s.payload.end());<br>
<br>
            s.payload.swap(tail);<br>
<br>
            if (s.marker.empty())<br>
                throw std::runtime_error(&quot;empty text marker&quot;);<br>
        }<br>
        else<br>
        {<br>
            // YAML-подобный режим:<br>
            // BEFORE:<br>
            //   ...<br>
            // MARKER:<br>
            //   ...<br>
            // AFTER:<br>
            //   ...<br>
            // ---<br>
            // &lt;payload&gt;<br>
            s.before.clear();<br>
            s.marker.clear();<br>
            s.after.clear();<br>
<br>
            enum class Block<br>
            {<br>
                NONE,<br>
                BEFORE,<br>
                MARKER,<br>
                AFTER<br>
            };<br>
<br>
            Block blk = Block::NONE;<br>
            std::vector&lt;std::string&gt; new_payload;<br>
<br>
            bool seen_separator = false;<br>
<br>
            for (std::size_t i = first_non_empty; i &lt; s.payload.size(); ++i)<br>
            {<br>
                const std::string &amp;ln = s.payload[i];<br>
<br>
                if (!seen_separator &amp;&amp; ln == &quot;---&quot;)<br>
                {<br>
                    seen_separator = true;<br>
                    continue;<br>
                }<br>
<br>
                if (!seen_separator)<br>
                {<br>
                    const std::string t = trim(ln);<br>
                    if (t == &quot;BEFORE:&quot;)<br>
                    {<br>
                        blk = Block::BEFORE;<br>
                        continue;<br>
                    }<br>
                    if (t == &quot;MARKER:&quot;)<br>
                    {<br>
                        blk = Block::MARKER;<br>
                        continue;<br>
                    }<br>
                    if (t == &quot;AFTER:&quot;)<br>
                    {<br>
                        blk = Block::AFTER;<br>
                        continue;<br>
                    }<br>
<br>
                    switch (blk)<br>
                    {<br>
                    case Block::BEFORE:<br>
                        s.before.push_back(ln);<br>
                        break;<br>
                    case Block::MARKER:<br>
                        s.marker.push_back(ln);<br>
                        break;<br>
                    case Block::AFTER:<br>
                        s.after.push_back(ln);<br>
                        break;<br>
                    case Block::NONE:<br>
                        throw std::runtime_error(<br>
                            &quot;unexpected content before YAML block tag&quot;);<br>
                    }<br>
                }<br>
                else<br>
                {<br>
                    new_payload.push_back(ln);<br>
                }<br>
            }<br>
<br>
            s.payload.swap(new_payload);<br>
<br>
            if (s.marker.empty())<br>
                throw std::runtime_error(<br>
                    &quot;YAML text command requires MARKER: section&quot;);<br>
        }<br>
    }<br>
<br>
    return s;<br>
}<br>
<br>
static void apply_for_file(const std::string &amp;filepath,<br>
                           const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
    fs::path p = filepath;<br>
    std::vector&lt;std::string&gt; orig;<br>
    bool existed = true;<br>
<br>
    try<br>
    {<br>
        orig = read_file_lines(p);<br>
    }<br>
    catch (...)<br>
    {<br>
        existed = false;<br>
        orig.clear();<br>
    }<br>
<br>
    for (const Section *s : sections)<br>
    {<br>
        if (!existed &amp;&amp; s-&gt;command == &quot;delete-file&quot;)<br>
            throw std::runtime_error(&quot;delete-file: file does not exist&quot;);<br>
    }<br>
<br>
    for (const Section *s : sections)<br>
    {<br>
        if (s-&gt;command == &quot;create-file&quot;)<br>
        {<br>
            write_file_lines(p, s-&gt;payload);<br>
            return;<br>
        }<br>
        if (s-&gt;command == &quot;delete-file&quot;)<br>
        {<br>
            std::error_code ec;<br>
            fs::remove(p, ec);<br>
            if (ec)<br>
                throw std::runtime_error(&quot;delete-file failed&quot;);<br>
            return;<br>
        }<br>
    }<br>
<br>
    std::vector&lt;const Section *&gt; text_sections;<br>
    std::vector&lt;const Section *&gt; symbol_sections;<br>
<br>
    for (const Section *s : sections)<br>
    {<br>
        if (is_text_command(s-&gt;command))<br>
            text_sections.push_back(s);<br>
        else if (is_symbol_command(s-&gt;command))<br>
            symbol_sections.push_back(s);<br>
        else<br>
            throw std::runtime_error(&quot;unexpected non-text command: &quot; +<br>
                                     s-&gt;command);<br>
    }<br>
<br>
    if (!text_sections.empty())<br>
        apply_text_commands(filepath, orig, text_sections);<br>
<br>
    if (!symbol_sections.empty())<br>
        apply_symbol_commands(filepath, orig, symbol_sections);<br>
<br>
    if (!text_sections.empty() || !symbol_sections.empty())<br>
        write_file_lines(p, orig);<br>
}<br>
<br>
static void apply_all(const std::vector&lt;Section&gt; &amp;sections)<br>
{<br>
    namespace fs = std::filesystem;<br>
<br>
    // 1. Собираем список всех файлов, которые будут затронуты<br>
    std::vector&lt;std::string&gt; files;<br>
    files.reserve(sections.size());<br>
    for (auto &amp;s : sections)<br>
        files.push_back(s.filepath);<br>
<br>
    std::sort(files.begin(), files.end());<br>
    files.erase(std::unique(files.begin(), files.end()), files.end());<br>
<br>
    struct Backup<br>
    {<br>
        bool existed = false;<br>
        std::vector&lt;std::string&gt; lines;<br>
    };<br>
<br>
    std::map&lt;std::string, Backup&gt; backup;<br>
<br>
    // 2. Делаем резервную копию всех файлов<br>
    for (auto &amp;f : files)<br>
    {<br>
        Backup b;<br>
        fs::path p = f;<br>
<br>
        std::error_code ec;<br>
<br>
        if (fs::exists(p, ec))<br>
        {<br>
            b.existed = true;<br>
<br>
            try<br>
            {<br>
                b.lines = read_file_lines(p);<br>
            }<br>
            catch (...)<br>
            {<br>
                throw std::runtime_error(&quot;cannot read original file: &quot; + f);<br>
            }<br>
        }<br>
        else<br>
        {<br>
            b.existed = false;<br>
        }<br>
<br>
        backup[f] = std::move(b);<br>
    }<br>
<br>
    // 3. Применяем секции с защитой (try/catch)<br>
    try<br>
    {<br>
        for (auto &amp;s : sections)<br>
        {<br>
            std::vector&lt;const Section *&gt; single{&amp;s};<br>
            apply_for_file(s.filepath, single);<br>
        }<br>
    }<br>
    catch (...)<br>
    {<br>
        // 4. Откат (rollback)<br>
        for (auto &amp;[path, b] : backup)<br>
        {<br>
            fs::path p = path;<br>
            std::error_code ec;<br>
<br>
            if (b.existed)<br>
            {<br>
                try<br>
                {<br>
                    write_file_lines(p, b.lines);<br>
                }<br>
                catch (...)<br>
                {<br>
                    // если даже откат не удался — сдаёмся<br>
                }<br>
            }<br>
            else<br>
            {<br>
                fs::remove(p, ec);<br>
            }<br>
        }<br>
<br>
        throw;<br>
    }<br>
}<br>
<br>
int apply_chunk_main(int argc, char **argv)<br>
{<br>
    if (argc &lt; 2)<br>
    {<br>
        std::cerr &lt;&lt; &quot;usage: apply_patch &lt;patchfile&gt;\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    std::ifstream in(argv[1]);<br>
    if (!in)<br>
    {<br>
        std::cerr &lt;&lt; &quot;cannot open patch file: &quot; &lt;&lt; argv[1] &lt;&lt; &quot;\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    std::string line;<br>
    std::vector&lt;Section&gt; sections;<br>
    int seq = 0;<br>
<br>
    try<br>
    {<br>
        while (std::getline(in, line))<br>
        {<br>
            if (line.rfind(&quot;=== file:&quot;, 0) == 0)<br>
            {<br>
                Section s = parse_section(in, line);<br>
                s.seq = seq++;<br>
                sections.push_back(std::move(s));<br>
            }<br>
        }<br>
<br>
        apply_all(sections);<br>
    }<br>
    catch (const std::exception &amp;e)<br>
    {<br>
        std::cerr &lt;&lt; &quot;error while applying patch: &quot; &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

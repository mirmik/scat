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
&#9;std::string filepath;<br>
&#9;std::string command;<br>
&#9;int a = -1;<br>
&#9;int b = -1;<br>
&#9;std::vector&lt;std::string&gt; payload;<br>
&#9;std::vector&lt;std::string&gt; marker;<br>
&#9;std::vector&lt;std::string&gt; before; // контекст до маркера (BEFORE:)<br>
&#9;std::vector&lt;std::string&gt; after; // контекст после маркера (AFTER:)<br>
&#9;int seq = 0;<br>
&#9;std::string arg1; // доп. аргументы команды (например, имя класса)<br>
&#9;std::string arg2; // второй аргумент (например, имя метода)<br>
};<br>
<br>
static std::vector&lt;std::string&gt; read_file_lines(const fs::path &amp;p)<br>
{<br>
&#9;std::ifstream in(p);<br>
&#9;if (!in)<br>
&#9;&#9;throw std::runtime_error(&quot;cannot open file: &quot; + p.string());<br>
<br>
&#9;std::vector&lt;std::string&gt; out;<br>
&#9;std::string line;<br>
&#9;while (std::getline(in, line))<br>
&#9;&#9;out.push_back(line);<br>
<br>
&#9;return out;<br>
}<br>
<br>
static void write_file_lines(const fs::path &amp;p,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
&#9;std::ofstream out(p, std::ios::trunc);<br>
&#9;if (!out)<br>
&#9;&#9;throw std::runtime_error(&quot;cannot write file: &quot; + p.string());<br>
<br>
&#9;for (const auto &amp;s : lines)<br>
&#9;&#9;out &lt;&lt; s &lt;&lt; &quot;\n&quot;;<br>
}<br>
<br>
std::string trim(const std::string_view &amp;view)<br>
{<br>
&#9;if (view.size() == 0)<br>
&#9;&#9;return &quot;&quot;;<br>
<br>
&#9;const char *left = view.data();<br>
&#9;const char *right = view.data() + view.size() - 1;<br>
&#9;const char *end = view.data() + view.size();<br>
<br>
&#9;while (left != end &amp;&amp;<br>
&#9;&#9;(*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))<br>
&#9;&#9;++left;<br>
<br>
&#9;if (left == end)<br>
&#9;&#9;return &quot;&quot;;<br>
<br>
&#9;while (left != right &amp;&amp; (*right == ' ' || *right == '\n' ||<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;*right == '\r' || *right == '\t'))<br>
&#9;&#9;--right;<br>
<br>
&#9;return std::string(left, (right - left) + 1);<br>
}<br>
<br>
static bool is_text_command(const std::string &amp;cmd)<br>
{<br>
&#9;return cmd == &quot;insert-after-text&quot; || cmd == &quot;insert-before-text&quot; ||<br>
&#9;&#9;cmd == &quot;replace-text&quot; || cmd == &quot;delete-text&quot;;<br>
}<br>
<br>
static bool is_symbol_command(const std::string &amp;cmd)<br>
{<br>
&#9;return cmd == &quot;replace-cpp-method&quot; || cmd == &quot;replace-cpp-class&quot; ||<br>
&#9;&#9;cmd == &quot;replace-py-method&quot; || cmd == &quot;replace-py-class&quot;;<br>
}<br>
<br>
static int find_subsequence(const std::vector&lt;std::string&gt; &amp;haystack,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;std::string&gt; &amp;needle)<br>
{<br>
&#9;if (needle.empty() || needle.size() &gt; haystack.size())<br>
&#9;&#9;return -1;<br>
<br>
&#9;const std::size_t n = haystack.size();<br>
&#9;const std::size_t m = needle.size();<br>
<br>
&#9;for (std::size_t i = 0; i + m &lt;= n; ++i)<br>
&#9;{<br>
&#9;&#9;bool ok = true;<br>
&#9;&#9;for (std::size_t j = 0; j &lt; m; ++j)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::string h = trim(haystack[i + j]);<br>
&#9;&#9;&#9;std::string nn = trim(needle[j]);<br>
&#9;&#9;&#9;if (h != nn)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;ok = false;<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;if (ok)<br>
&#9;&#9;&#9;return static_cast&lt;int&gt;(i);<br>
&#9;}<br>
<br>
&#9;return -1;<br>
}<br>
<br>
// Строгий выбор позиции маркера с учётом BEFORE/AFTER.<br>
// Никакого fuzzy, только точное позиционное совпадение.<br>
static int find_best_marker_match(const std::vector&lt;std::string&gt; &amp;lines,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const Section *s,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;int&gt; &amp;candidates)<br>
{<br>
&#9;if (candidates.empty())<br>
&#9;&#9;return -1;<br>
<br>
&#9;// Нет дополнительного контекста — ведём себя как раньше.<br>
&#9;if (s-&gt;before.empty() &amp;&amp; s-&gt;after.empty())<br>
&#9;&#9;return candidates.front();<br>
<br>
&#9;auto trim_eq = [&amp;](const std::string &amp;a, const std::string &amp;b)<br>
&#9;{ return trim(a) == trim(b); };<br>
<br>
&#9;std::vector&lt;int&gt; strict;<br>
<br>
&#9;for (int pos : candidates)<br>
&#9;{<br>
&#9;&#9;bool ok = true;<br>
<br>
&#9;&#9;// BEFORE: строки сразу над маркером<br>
&#9;&#9;if (!s-&gt;before.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;int need = static_cast&lt;int&gt;(s-&gt;before.size());<br>
&#9;&#9;&#9;if (pos &lt; need)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;ok = false;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;// Последняя строка BEFORE должна быть непосредственно над<br>
&#9;&#9;&#9;&#9;// первой строкой маркера.<br>
&#9;&#9;&#9;&#9;for (int i = 0; i &lt; need; ++i)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;want =<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;before[static_cast&lt;std::size_t&gt;(need - 1 - i)];<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;got =<br>
&#9;&#9;&#9;&#9;&#9;&#9;lines[static_cast&lt;std::size_t&gt;(pos - 1 - i)];<br>
&#9;&#9;&#9;&#9;&#9;if (!trim_eq(got, want))<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;ok = false;<br>
&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// AFTER: строки сразу под маркером<br>
&#9;&#9;if (ok &amp;&amp; !s-&gt;after.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;int start = pos + static_cast&lt;int&gt;(s-&gt;marker.size());<br>
&#9;&#9;&#9;int need = static_cast&lt;int&gt;(s-&gt;after.size());<br>
<br>
&#9;&#9;&#9;if (start &lt; 0 || start + need &gt; static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;ok = false;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;for (int i = 0; i &lt; need; ++i)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;want =<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;after[static_cast&lt;std::size_t&gt;(i)];<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;got =<br>
&#9;&#9;&#9;&#9;&#9;&#9;lines[static_cast&lt;std::size_t&gt;(start + i)];<br>
&#9;&#9;&#9;&#9;&#9;if (!trim_eq(got, want))<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;ok = false;<br>
&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (ok)<br>
&#9;&#9;&#9;strict.push_back(pos);<br>
&#9;}<br>
<br>
&#9;if (strict.empty())<br>
&#9;&#9;throw std::runtime_error(&quot;strict marker context not found&quot;);<br>
<br>
&#9;if (strict.size() &gt; 1)<br>
&#9;&#9;throw std::runtime_error(&quot;strict marker match is ambiguous&quot;);<br>
<br>
&#9;return strict.front();<br>
}<br>
<br>
static void apply_text_commands(const std::string &amp;filepath,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;std::vector&lt;std::string&gt; &amp;lines,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&#9;for (const Section *s : sections)<br>
&#9;{<br>
&#9;&#9;if (s-&gt;marker.empty())<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;empty marker in text command for file: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;filepath);<br>
<br>
&#9;&#9;// Собираем все вхождения маркера<br>
&#9;&#9;std::vector&lt;int&gt; candidates;<br>
&#9;&#9;{<br>
&#9;&#9;&#9;int base = 0;<br>
&#9;&#9;&#9;while (base &lt; static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::vector&lt;std::string&gt; sub(lines.begin() + base, lines.end());<br>
&#9;&#9;&#9;&#9;int idx = find_subsequence(sub, s-&gt;marker);<br>
&#9;&#9;&#9;&#9;if (idx &lt; 0)<br>
&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;candidates.push_back(base + idx);<br>
&#9;&#9;&#9;&#9;base += idx + 1;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (candidates.empty())<br>
&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&quot;text marker not found for file: &quot; + filepath +<br>
&#9;&#9;&#9;&#9;&quot;\ncommand: &quot; + s-&gt;command + &quot;\n&quot;);<br>
<br>
&#9;&#9;int idx = find_best_marker_match(lines, s, candidates);<br>
&#9;&#9;if (idx &lt; 0)<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;cannot locate marker uniquely&quot;);<br>
<br>
&#9;&#9;std::size_t pos = static_cast&lt;std::size_t&gt;(idx);<br>
<br>
&#9;&#9;if (s-&gt;command == &quot;insert-after-text&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;pos += s-&gt;marker.size();<br>
&#9;&#9;&#9;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;}<br>
&#9;&#9;else if (s-&gt;command == &quot;insert-before-text&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;}<br>
&#9;&#9;else if (s-&gt;command == &quot;replace-text&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
&#9;&#9;&#9;auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;&#9;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;}<br>
&#9;&#9;else if (s-&gt;command == &quot;delete-text&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
&#9;&#9;&#9;auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;unknown text command: &quot; + s-&gt;command);<br>
&#9;&#9;}<br>
&#9;}<br>
}<br>
<br>
static std::string join_lines(const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
&#9;if (lines.empty())<br>
&#9;&#9;return std::string();<br>
<br>
&#9;std::string text;<br>
&#9;std::size_t total = 0;<br>
&#9;for (const auto &amp;s : lines)<br>
&#9;&#9;total += s.size() + 1;<br>
&#9;text.reserve(total);<br>
<br>
&#9;for (std::size_t i = 0; i &lt; lines.size(); ++i)<br>
&#9;{<br>
&#9;&#9;text += lines[i];<br>
&#9;&#9;if (i + 1 &lt; lines.size())<br>
&#9;&#9;&#9;text += '\n';<br>
&#9;}<br>
<br>
&#9;return text;<br>
}<br>
<br>
static void apply_symbol_commands(const std::string &amp;filepath,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;std::vector&lt;std::string&gt; &amp;lines,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&#9;for (const Section *s : sections)<br>
&#9;{<br>
&#9;&#9;// Всегда работаем с текущей версией файла<br>
&#9;&#9;std::string text = join_lines(lines);<br>
<br>
&#9;&#9;if (s-&gt;command == &quot;replace-cpp-class&quot; ||<br>
&#9;&#9;&#9;s-&gt;command == &quot;replace-cpp-method&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;CppSymbolFinder finder(text);<br>
<br>
&#9;&#9;&#9;if (s-&gt;command == &quot;replace-cpp-class&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;if (s-&gt;arg1.empty())<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-class: missing class name for file: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;filepath);<br>
<br>
&#9;&#9;&#9;&#9;Region r;<br>
&#9;&#9;&#9;&#9;if (!finder.find_class(s-&gt;arg1, r))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-class: class not found: &quot; + s-&gt;arg1 +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot; in file: &quot; + filepath);<br>
<br>
&#9;&#9;&#9;&#9;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&#9;&#9;&#9;&#9;&#9;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-class: invalid region&quot;);<br>
<br>
&#9;&#9;&#9;&#9;auto begin = lines.begin() + r.start_line;<br>
&#9;&#9;&#9;&#9;auto end = lines.begin() + (r.end_line + 1);<br>
&#9;&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;&#9;&#9;lines.insert(lines.begin() + r.start_line,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else // replace-cpp-method<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::string cls;<br>
&#9;&#9;&#9;&#9;std::string method;<br>
<br>
&#9;&#9;&#9;&#9;if (!s-&gt;arg2.empty())<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;cls = s-&gt;arg1;<br>
&#9;&#9;&#9;&#9;&#9;method = s-&gt;arg2;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;auto pos = s-&gt;arg1.find(&quot;::&quot;);<br>
&#9;&#9;&#9;&#9;&#9;if (pos == std::string::npos)<br>
&#9;&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-method: expected 'Class::method' or &quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;'Class method'&quot;);<br>
<br>
&#9;&#9;&#9;&#9;&#9;cls = s-&gt;arg1.substr(0, pos);<br>
&#9;&#9;&#9;&#9;&#9;method = s-&gt;arg1.substr(pos + 2);<br>
&#9;&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;&#9;if (cls.empty() || method.empty())<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-method: empty class or method name&quot;);<br>
<br>
&#9;&#9;&#9;&#9;Region r;<br>
&#9;&#9;&#9;&#9;if (!finder.find_method(cls, method, r))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-method: method not found: &quot; + cls +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;::&quot; + method + &quot; in file: &quot; + filepath);<br>
<br>
&#9;&#9;&#9;&#9;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&#9;&#9;&#9;&#9;&#9;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-cpp-method: invalid region&quot;);<br>
<br>
&#9;&#9;&#9;&#9;auto begin = lines.begin() + r.start_line;<br>
&#9;&#9;&#9;&#9;auto end = lines.begin() + (r.end_line + 1);<br>
&#9;&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;&#9;&#9;lines.insert(lines.begin() + r.start_line,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (s-&gt;command == &quot;replace-py-class&quot; ||<br>
&#9;&#9;&#9;s-&gt;command == &quot;replace-py-method&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;PythonSymbolFinder finder(text);<br>
<br>
&#9;&#9;&#9;if (s-&gt;command == &quot;replace-py-class&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;if (s-&gt;arg1.empty())<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-class: missing class name for file: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;filepath);<br>
<br>
&#9;&#9;&#9;&#9;Region r;<br>
&#9;&#9;&#9;&#9;if (!finder.find_class(s-&gt;arg1, r))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-class: class not found: &quot; + s-&gt;arg1 +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot; in file: &quot; + filepath);<br>
<br>
&#9;&#9;&#9;&#9;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&#9;&#9;&#9;&#9;&#9;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-class: invalid region&quot;);<br>
<br>
&#9;&#9;&#9;&#9;auto begin = lines.begin() + r.start_line;<br>
&#9;&#9;&#9;&#9;auto end = lines.begin() + (r.end_line + 1);<br>
&#9;&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;&#9;&#9;lines.insert(lines.begin() + r.start_line,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else // replace-py-method<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::string cls;<br>
&#9;&#9;&#9;&#9;std::string method;<br>
<br>
&#9;&#9;&#9;&#9;if (!s-&gt;arg2.empty())<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;cls = s-&gt;arg1;<br>
&#9;&#9;&#9;&#9;&#9;method = s-&gt;arg2;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;auto pos = s-&gt;arg1.find('.');<br>
&#9;&#9;&#9;&#9;&#9;if (pos == std::string::npos)<br>
&#9;&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-method: expected 'Class.method' or &quot;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;'Class method'&quot;);<br>
<br>
&#9;&#9;&#9;&#9;&#9;cls = s-&gt;arg1.substr(0, pos);<br>
&#9;&#9;&#9;&#9;&#9;method = s-&gt;arg1.substr(pos + 1);<br>
&#9;&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;&#9;if (cls.empty() || method.empty())<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-method: empty class or method name&quot;);<br>
<br>
&#9;&#9;&#9;&#9;Region r;<br>
&#9;&#9;&#9;&#9;if (!finder.find_method(cls, method, r))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-method: method not found: &quot; + cls + &quot;.&quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;method + &quot; in file: &quot; + filepath);<br>
<br>
&#9;&#9;&#9;&#9;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&#9;&#9;&#9;&#9;&#9;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;replace-py-method: invalid region&quot;);<br>
<br>
&#9;&#9;&#9;&#9;auto begin = lines.begin() + r.start_line;<br>
&#9;&#9;&#9;&#9;auto end = lines.begin() + (r.end_line + 1);<br>
&#9;&#9;&#9;&#9;lines.erase(begin, end);<br>
&#9;&#9;&#9;&#9;lines.insert(lines.begin() + r.start_line,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;payload.end());<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;throw std::runtime_error(&quot;apply_symbol_commands: unknown command: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;command);<br>
&#9;}<br>
}<br>
<br>
static Section parse_section(std::istream &amp;in, const std::string &amp;header)<br>
{<br>
&#9;Section s;<br>
<br>
&#9;auto pos = header.find(':');<br>
&#9;if (pos == std::string::npos)<br>
&#9;&#9;throw std::runtime_error(&quot;bad section header: &quot; + header);<br>
<br>
&#9;auto pos2 = header.find(&quot;===&quot;, pos);<br>
&#9;if (pos2 == std::string::npos)<br>
&#9;&#9;pos2 = header.size();<br>
<br>
&#9;auto raw = header.substr(pos + 1, pos2 - pos - 1);<br>
&#9;s.filepath = trim(raw);<br>
&#9;if (s.filepath.empty())<br>
&#9;&#9;throw std::runtime_error(&quot;empty filepath in header: &quot; + header);<br>
<br>
&#9;std::string line;<br>
&#9;if (!std::getline(in, line))<br>
&#9;&#9;throw std::runtime_error(&quot;unexpected end after header&quot;);<br>
<br>
&#9;if (line.rfind(&quot;--- &quot;, 0) != 0)<br>
&#9;&#9;throw std::runtime_error(&quot;expected command after header&quot;);<br>
<br>
&#9;{<br>
&#9;&#9;std::istringstream ss(line.substr(4));<br>
&#9;&#9;ss &gt;&gt; s.command;<br>
<br>
&#9;&#9;// читаем остаток строки как аргументы команды<br>
&#9;&#9;std::string rest;<br>
&#9;&#9;std::getline(ss, rest);<br>
&#9;&#9;if (!rest.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::istringstream as(rest);<br>
&#9;&#9;&#9;as &gt;&gt; s.arg1;<br>
&#9;&#9;&#9;as &gt;&gt; s.arg2;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (is_text_command(s.command))<br>
&#9;&#9;{<br>
&#9;&#9;}<br>
&#9;&#9;else if (s.command == &quot;create-file&quot; || s.command == &quot;delete-file&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;}<br>
&#9;&#9;else if (is_symbol_command(s.command))<br>
&#9;&#9;{<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;index-based commands removed: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;s.command);<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;bool found_end = false;<br>
&#9;while (std::getline(in, line))<br>
&#9;{<br>
&#9;&#9;if (line == &quot;=END=&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;found_end = true;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;}<br>
&#9;&#9;s.payload.push_back(line);<br>
&#9;}<br>
<br>
&#9;if (!found_end)<br>
&#9;&#9;throw std::runtime_error(&quot;missing =END=&quot;);<br>
<br>
&#9;if (is_text_command(s.command))<br>
&#9;{<br>
&#9;&#9;// Определяем, в YAML-режиме мы или в старом формате.<br>
&#9;&#9;// Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется<br>
&#9;&#9;// старая логика.<br>
&#9;&#9;bool yaml_mode = false;<br>
&#9;&#9;std::size_t first_non_empty = 0;<br>
&#9;&#9;while (first_non_empty &lt; s.payload.size() &amp;&amp;<br>
&#9;&#9;&#9;trim(s.payload[first_non_empty]).empty())<br>
&#9;&#9;&#9;++first_non_empty;<br>
<br>
&#9;&#9;if (first_non_empty &lt; s.payload.size())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string t = trim(s.payload[first_non_empty]);<br>
&#9;&#9;&#9;if (t == &quot;BEFORE:&quot; || t == &quot;MARKER:&quot; || t == &quot;AFTER:&quot;)<br>
&#9;&#9;&#9;&#9;yaml_mode = true;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (!yaml_mode)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// Старый режим: всё до '---' — marker, после — payload<br>
&#9;&#9;&#9;auto it = std::find(<br>
&#9;&#9;&#9;&#9;s.payload.begin(), s.payload.end(), std::string(&quot;---&quot;));<br>
&#9;&#9;&#9;if (it == s.payload.end())<br>
&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&quot;text command requires '---' separator&quot;);<br>
<br>
&#9;&#9;&#9;s.marker.assign(s.payload.begin(), it);<br>
<br>
&#9;&#9;&#9;std::vector&lt;std::string&gt; tail;<br>
&#9;&#9;&#9;if (std::next(it) != s.payload.end())<br>
&#9;&#9;&#9;&#9;tail.assign(std::next(it), s.payload.end());<br>
<br>
&#9;&#9;&#9;s.payload.swap(tail);<br>
<br>
&#9;&#9;&#9;if (s.marker.empty())<br>
&#9;&#9;&#9;&#9;throw std::runtime_error(&quot;empty text marker&quot;);<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// YAML-подобный режим:<br>
&#9;&#9;&#9;// BEFORE:<br>
&#9;&#9;&#9;//   ...<br>
&#9;&#9;&#9;// MARKER:<br>
&#9;&#9;&#9;//   ...<br>
&#9;&#9;&#9;// AFTER:<br>
&#9;&#9;&#9;//   ...<br>
&#9;&#9;&#9;// ---<br>
&#9;&#9;&#9;// &lt;payload&gt;<br>
&#9;&#9;&#9;s.before.clear();<br>
&#9;&#9;&#9;s.marker.clear();<br>
&#9;&#9;&#9;s.after.clear();<br>
<br>
&#9;&#9;&#9;enum class Block<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;NONE,<br>
&#9;&#9;&#9;&#9;BEFORE,<br>
&#9;&#9;&#9;&#9;MARKER,<br>
&#9;&#9;&#9;&#9;AFTER<br>
&#9;&#9;&#9;};<br>
<br>
&#9;&#9;&#9;Block blk = Block::NONE;<br>
&#9;&#9;&#9;std::vector&lt;std::string&gt; new_payload;<br>
<br>
&#9;&#9;&#9;bool seen_separator = false;<br>
<br>
&#9;&#9;&#9;for (std::size_t i = first_non_empty; i &lt; s.payload.size(); ++i)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;const std::string &amp;ln = s.payload[i];<br>
<br>
&#9;&#9;&#9;&#9;if (!seen_separator &amp;&amp; ln == &quot;---&quot;)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;seen_separator = true;<br>
&#9;&#9;&#9;&#9;&#9;continue;<br>
&#9;&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;&#9;if (!seen_separator)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;const std::string t = trim(ln);<br>
&#9;&#9;&#9;&#9;&#9;if (t == &quot;BEFORE:&quot;)<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;blk = Block::BEFORE;<br>
&#9;&#9;&#9;&#9;&#9;&#9;continue;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;&#9;if (t == &quot;MARKER:&quot;)<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;blk = Block::MARKER;<br>
&#9;&#9;&#9;&#9;&#9;&#9;continue;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;&#9;if (t == &quot;AFTER:&quot;)<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;blk = Block::AFTER;<br>
&#9;&#9;&#9;&#9;&#9;&#9;continue;<br>
&#9;&#9;&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;&#9;&#9;switch (blk)<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;case Block::BEFORE:<br>
&#9;&#9;&#9;&#9;&#9;&#9;s.before.push_back(ln);<br>
&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;case Block::MARKER:<br>
&#9;&#9;&#9;&#9;&#9;&#9;s.marker.push_back(ln);<br>
&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;case Block::AFTER:<br>
&#9;&#9;&#9;&#9;&#9;&#9;s.after.push_back(ln);<br>
&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;case Block::NONE:<br>
&#9;&#9;&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;unexpected content before YAML block tag&quot;);<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;new_payload.push_back(ln);<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;s.payload.swap(new_payload);<br>
<br>
&#9;&#9;&#9;if (s.marker.empty())<br>
&#9;&#9;&#9;&#9;throw std::runtime_error(<br>
&#9;&#9;&#9;&#9;&#9;&quot;YAML text command requires MARKER: section&quot;);<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;return s;<br>
}<br>
<br>
static void apply_for_file(const std::string &amp;filepath,<br>
&#9;&#9;&#9;&#9;&#9;&#9;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&#9;fs::path p = filepath;<br>
&#9;std::vector&lt;std::string&gt; orig;<br>
&#9;bool existed = true;<br>
<br>
&#9;try<br>
&#9;{<br>
&#9;&#9;orig = read_file_lines(p);<br>
&#9;}<br>
&#9;catch (...)<br>
&#9;{<br>
&#9;&#9;existed = false;<br>
&#9;&#9;orig.clear();<br>
&#9;}<br>
<br>
&#9;for (const Section *s : sections)<br>
&#9;{<br>
&#9;&#9;if (!existed &amp;&amp; s-&gt;command == &quot;delete-file&quot;)<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;delete-file: file does not exist&quot;);<br>
&#9;}<br>
<br>
&#9;for (const Section *s : sections)<br>
&#9;{<br>
&#9;&#9;if (s-&gt;command == &quot;create-file&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;write_file_lines(p, s-&gt;payload);<br>
&#9;&#9;&#9;return;<br>
&#9;&#9;}<br>
&#9;&#9;if (s-&gt;command == &quot;delete-file&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::error_code ec;<br>
&#9;&#9;&#9;fs::remove(p, ec);<br>
&#9;&#9;&#9;if (ec)<br>
&#9;&#9;&#9;&#9;throw std::runtime_error(&quot;delete-file failed&quot;);<br>
&#9;&#9;&#9;return;<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;std::vector&lt;const Section *&gt; text_sections;<br>
&#9;std::vector&lt;const Section *&gt; symbol_sections;<br>
<br>
&#9;for (const Section *s : sections)<br>
&#9;{<br>
&#9;&#9;if (is_text_command(s-&gt;command))<br>
&#9;&#9;&#9;text_sections.push_back(s);<br>
&#9;&#9;else if (is_symbol_command(s-&gt;command))<br>
&#9;&#9;&#9;symbol_sections.push_back(s);<br>
&#9;&#9;else<br>
&#9;&#9;&#9;throw std::runtime_error(&quot;unexpected non-text command: &quot; +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;s-&gt;command);<br>
&#9;}<br>
<br>
&#9;if (!text_sections.empty())<br>
&#9;&#9;apply_text_commands(filepath, orig, text_sections);<br>
<br>
&#9;if (!symbol_sections.empty())<br>
&#9;&#9;apply_symbol_commands(filepath, orig, symbol_sections);<br>
<br>
&#9;if (!text_sections.empty() || !symbol_sections.empty())<br>
&#9;&#9;write_file_lines(p, orig);<br>
}<br>
<br>
static void apply_all(const std::vector&lt;Section&gt; &amp;sections)<br>
{<br>
&#9;namespace fs = std::filesystem;<br>
<br>
&#9;// 1. Собираем список всех файлов, которые будут затронуты<br>
&#9;std::vector&lt;std::string&gt; files;<br>
&#9;files.reserve(sections.size());<br>
&#9;for (auto &amp;s : sections)<br>
&#9;&#9;files.push_back(s.filepath);<br>
<br>
&#9;std::sort(files.begin(), files.end());<br>
&#9;files.erase(std::unique(files.begin(), files.end()), files.end());<br>
<br>
&#9;struct Backup<br>
&#9;{<br>
&#9;&#9;bool existed = false;<br>
&#9;&#9;std::vector&lt;std::string&gt; lines;<br>
&#9;};<br>
<br>
&#9;std::map&lt;std::string, Backup&gt; backup;<br>
<br>
&#9;// 2. Делаем резервную копию всех файлов<br>
&#9;for (auto &amp;f : files)<br>
&#9;{<br>
&#9;&#9;Backup b;<br>
&#9;&#9;fs::path p = f;<br>
<br>
&#9;&#9;std::error_code ec;<br>
<br>
&#9;&#9;if (fs::exists(p, ec))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;b.existed = true;<br>
<br>
&#9;&#9;&#9;try<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;b.lines = read_file_lines(p);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;catch (...)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;throw std::runtime_error(&quot;cannot read original file: &quot; + f);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;b.existed = false;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;backup[f] = std::move(b);<br>
&#9;}<br>
<br>
&#9;// 3. Применяем секции с защитой (try/catch)<br>
&#9;try<br>
&#9;{<br>
&#9;&#9;for (auto &amp;s : sections)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::vector&lt;const Section *&gt; single{&amp;s};<br>
&#9;&#9;&#9;apply_for_file(s.filepath, single);<br>
&#9;&#9;}<br>
&#9;}<br>
&#9;catch (...)<br>
&#9;{<br>
&#9;&#9;// 4. Откат (rollback)<br>
&#9;&#9;for (auto &amp;[path, b] : backup)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;fs::path p = path;<br>
&#9;&#9;&#9;std::error_code ec;<br>
<br>
&#9;&#9;&#9;if (b.existed)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;try<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;write_file_lines(p, b.lines);<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;catch (...)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;// если даже откат не удался — сдаёмся<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;fs::remove(p, ec);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;throw;<br>
&#9;}<br>
}<br>
<br>
int apply_chunk_main(int argc, char **argv)<br>
{<br>
&#9;if (argc &lt; 2)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;usage: apply_patch &lt;patchfile&gt;\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;std::ifstream in(argv[1]);<br>
&#9;if (!in)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;cannot open patch file: &quot; &lt;&lt; argv[1] &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;std::string line;<br>
&#9;std::vector&lt;Section&gt; sections;<br>
&#9;int seq = 0;<br>
<br>
&#9;try<br>
&#9;{<br>
&#9;&#9;while (std::getline(in, line))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (line.rfind(&quot;=== file:&quot;, 0) == 0)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;Section s = parse_section(in, line);<br>
&#9;&#9;&#9;&#9;s.seq = seq++;<br>
&#9;&#9;&#9;&#9;sections.push_back(std::move(s));<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;apply_all(sections);<br>
&#9;}<br>
&#9;catch (const std::exception &amp;e)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;error while applying patch: &quot; &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

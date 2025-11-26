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
&emsp;std::string filepath;<br>
&emsp;std::string command;<br>
&emsp;int a = -1;<br>
&emsp;int b = -1;<br>
&emsp;std::vector&lt;std::string&gt; payload;<br>
&emsp;std::vector&lt;std::string&gt; marker;<br>
&emsp;std::vector&lt;std::string&gt; before; // контекст до маркера (BEFORE:)<br>
&emsp;std::vector&lt;std::string&gt; after; // контекст после маркера (AFTER:)<br>
&emsp;int seq = 0;<br>
&emsp;std::string arg1; // доп. аргументы команды (например, имя класса)<br>
&emsp;std::string arg2; // второй аргумент (например, имя метода)<br>
};<br>
<br>
static std::vector&lt;std::string&gt; read_file_lines(const fs::path &amp;p)<br>
{<br>
&emsp;std::ifstream in(p);<br>
&emsp;if (!in)<br>
&emsp;&emsp;throw std::runtime_error(&quot;cannot open file: &quot; + p.string());<br>
<br>
&emsp;std::vector&lt;std::string&gt; out;<br>
&emsp;std::string line;<br>
&emsp;while (std::getline(in, line))<br>
&emsp;&emsp;out.push_back(line);<br>
<br>
&emsp;return out;<br>
}<br>
<br>
static void write_file_lines(const fs::path &amp;p,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
&emsp;std::ofstream out(p, std::ios::trunc);<br>
&emsp;if (!out)<br>
&emsp;&emsp;throw std::runtime_error(&quot;cannot write file: &quot; + p.string());<br>
<br>
&emsp;for (const auto &amp;s : lines)<br>
&emsp;&emsp;out &lt;&lt; s &lt;&lt; &quot;\n&quot;;<br>
}<br>
<br>
std::string trim(const std::string_view &amp;view)<br>
{<br>
&emsp;if (view.size() == 0)<br>
&emsp;&emsp;return &quot;&quot;;<br>
<br>
&emsp;const char *left = view.data();<br>
&emsp;const char *right = view.data() + view.size() - 1;<br>
&emsp;const char *end = view.data() + view.size();<br>
<br>
&emsp;while (left != end &amp;&amp;<br>
&emsp;&emsp;(*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))<br>
&emsp;&emsp;++left;<br>
<br>
&emsp;if (left == end)<br>
&emsp;&emsp;return &quot;&quot;;<br>
<br>
&emsp;while (left != right &amp;&amp; (*right == ' ' || *right == '\n' ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;*right == '\r' || *right == '\t'))<br>
&emsp;&emsp;--right;<br>
<br>
&emsp;return std::string(left, (right - left) + 1);<br>
}<br>
<br>
static bool is_text_command(const std::string &amp;cmd)<br>
{<br>
&emsp;return cmd == &quot;insert-after-text&quot; || cmd == &quot;insert-before-text&quot; ||<br>
&emsp;&emsp;cmd == &quot;replace-text&quot; || cmd == &quot;delete-text&quot;;<br>
}<br>
<br>
static bool is_symbol_command(const std::string &amp;cmd)<br>
{<br>
&emsp;return cmd == &quot;replace-cpp-method&quot; || cmd == &quot;replace-cpp-class&quot; ||<br>
&emsp;&emsp;cmd == &quot;replace-py-method&quot; || cmd == &quot;replace-py-class&quot;;<br>
}<br>
<br>
static int find_subsequence(const std::vector&lt;std::string&gt; &amp;haystack,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;std::string&gt; &amp;needle)<br>
{<br>
&emsp;if (needle.empty() || needle.size() &gt; haystack.size())<br>
&emsp;&emsp;return -1;<br>
<br>
&emsp;const std::size_t n = haystack.size();<br>
&emsp;const std::size_t m = needle.size();<br>
<br>
&emsp;for (std::size_t i = 0; i + m &lt;= n; ++i)<br>
&emsp;{<br>
&emsp;&emsp;bool ok = true;<br>
&emsp;&emsp;for (std::size_t j = 0; j &lt; m; ++j)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::string h = trim(haystack[i + j]);<br>
&emsp;&emsp;&emsp;std::string nn = trim(needle[j]);<br>
&emsp;&emsp;&emsp;if (h != nn)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;ok = false;<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;if (ok)<br>
&emsp;&emsp;&emsp;return static_cast&lt;int&gt;(i);<br>
&emsp;}<br>
<br>
&emsp;return -1;<br>
}<br>
<br>
// Строгий выбор позиции маркера с учётом BEFORE/AFTER.<br>
// Никакого fuzzy, только точное позиционное совпадение.<br>
static int find_best_marker_match(const std::vector&lt;std::string&gt; &amp;lines,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const Section *s,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;int&gt; &amp;candidates)<br>
{<br>
&emsp;if (candidates.empty())<br>
&emsp;&emsp;return -1;<br>
<br>
&emsp;// Нет дополнительного контекста — ведём себя как раньше.<br>
&emsp;if (s-&gt;before.empty() &amp;&amp; s-&gt;after.empty())<br>
&emsp;&emsp;return candidates.front();<br>
<br>
&emsp;auto trim_eq = [&amp;](const std::string &amp;a, const std::string &amp;b)<br>
&emsp;{ return trim(a) == trim(b); };<br>
<br>
&emsp;std::vector&lt;int&gt; strict;<br>
<br>
&emsp;for (int pos : candidates)<br>
&emsp;{<br>
&emsp;&emsp;bool ok = true;<br>
<br>
&emsp;&emsp;// BEFORE: строки сразу над маркером<br>
&emsp;&emsp;if (!s-&gt;before.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;int need = static_cast&lt;int&gt;(s-&gt;before.size());<br>
&emsp;&emsp;&emsp;if (pos &lt; need)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;ok = false;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;// Последняя строка BEFORE должна быть непосредственно над<br>
&emsp;&emsp;&emsp;&emsp;// первой строкой маркера.<br>
&emsp;&emsp;&emsp;&emsp;for (int i = 0; i &lt; need; ++i)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;want =<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;before[static_cast&lt;std::size_t&gt;(need - 1 - i)];<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;got =<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;lines[static_cast&lt;std::size_t&gt;(pos - 1 - i)];<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (!trim_eq(got, want))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;ok = false;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// AFTER: строки сразу под маркером<br>
&emsp;&emsp;if (ok &amp;&amp; !s-&gt;after.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;int start = pos + static_cast&lt;int&gt;(s-&gt;marker.size());<br>
&emsp;&emsp;&emsp;int need = static_cast&lt;int&gt;(s-&gt;after.size());<br>
<br>
&emsp;&emsp;&emsp;if (start &lt; 0 || start + need &gt; static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;ok = false;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;for (int i = 0; i &lt; need; ++i)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;want =<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;after[static_cast&lt;std::size_t&gt;(i)];<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;got =<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;lines[static_cast&lt;std::size_t&gt;(start + i)];<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (!trim_eq(got, want))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;ok = false;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (ok)<br>
&emsp;&emsp;&emsp;strict.push_back(pos);<br>
&emsp;}<br>
<br>
&emsp;if (strict.empty())<br>
&emsp;&emsp;throw std::runtime_error(&quot;strict marker context not found&quot;);<br>
<br>
&emsp;if (strict.size() &gt; 1)<br>
&emsp;&emsp;throw std::runtime_error(&quot;strict marker match is ambiguous&quot;);<br>
<br>
&emsp;return strict.front();<br>
}<br>
<br>
static void apply_text_commands(const std::string &amp;filepath,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::vector&lt;std::string&gt; &amp;lines,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&emsp;for (const Section *s : sections)<br>
&emsp;{<br>
&emsp;&emsp;if (s-&gt;marker.empty())<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;empty marker in text command for file: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;filepath);<br>
<br>
&emsp;&emsp;// Собираем все вхождения маркера<br>
&emsp;&emsp;std::vector&lt;int&gt; candidates;<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;int base = 0;<br>
&emsp;&emsp;&emsp;while (base &lt; static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::vector&lt;std::string&gt; sub(lines.begin() + base, lines.end());<br>
&emsp;&emsp;&emsp;&emsp;int idx = find_subsequence(sub, s-&gt;marker);<br>
&emsp;&emsp;&emsp;&emsp;if (idx &lt; 0)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;candidates.push_back(base + idx);<br>
&emsp;&emsp;&emsp;&emsp;base += idx + 1;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (candidates.empty())<br>
&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&quot;text marker not found for file: &quot; + filepath +<br>
&emsp;&emsp;&emsp;&emsp;&quot;\ncommand: &quot; + s-&gt;command + &quot;\n&quot;);<br>
<br>
&emsp;&emsp;int idx = find_best_marker_match(lines, s, candidates);<br>
&emsp;&emsp;if (idx &lt; 0)<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;cannot locate marker uniquely&quot;);<br>
<br>
&emsp;&emsp;std::size_t pos = static_cast&lt;std::size_t&gt;(idx);<br>
<br>
&emsp;&emsp;if (s-&gt;command == &quot;insert-after-text&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;pos += s-&gt;marker.size();<br>
&emsp;&emsp;&emsp;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (s-&gt;command == &quot;insert-before-text&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (s-&gt;command == &quot;replace-text&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
&emsp;&emsp;&emsp;auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;&emsp;lines.insert(lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (s-&gt;command == &quot;delete-text&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;auto begin = lines.begin() + static_cast&lt;std::ptrdiff_t&gt;(pos);<br>
&emsp;&emsp;&emsp;auto end = begin + static_cast&lt;std::ptrdiff_t&gt;(s-&gt;marker.size());<br>
&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;unknown text command: &quot; + s-&gt;command);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
}<br>
<br>
static std::string join_lines(const std::vector&lt;std::string&gt; &amp;lines)<br>
{<br>
&emsp;if (lines.empty())<br>
&emsp;&emsp;return std::string();<br>
<br>
&emsp;std::string text;<br>
&emsp;std::size_t total = 0;<br>
&emsp;for (const auto &amp;s : lines)<br>
&emsp;&emsp;total += s.size() + 1;<br>
&emsp;text.reserve(total);<br>
<br>
&emsp;for (std::size_t i = 0; i &lt; lines.size(); ++i)<br>
&emsp;{<br>
&emsp;&emsp;text += lines[i];<br>
&emsp;&emsp;if (i + 1 &lt; lines.size())<br>
&emsp;&emsp;&emsp;text += '\n';<br>
&emsp;}<br>
<br>
&emsp;return text;<br>
}<br>
<br>
static void apply_symbol_commands(const std::string &amp;filepath,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::vector&lt;std::string&gt; &amp;lines,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&emsp;for (const Section *s : sections)<br>
&emsp;{<br>
&emsp;&emsp;// Всегда работаем с текущей версией файла<br>
&emsp;&emsp;std::string text = join_lines(lines);<br>
<br>
&emsp;&emsp;if (s-&gt;command == &quot;replace-cpp-class&quot; ||<br>
&emsp;&emsp;&emsp;s-&gt;command == &quot;replace-cpp-method&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;CppSymbolFinder finder(text);<br>
<br>
&emsp;&emsp;&emsp;if (s-&gt;command == &quot;replace-cpp-class&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;if (s-&gt;arg1.empty())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-class: missing class name for file: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;Region r;<br>
&emsp;&emsp;&emsp;&emsp;if (!finder.find_class(s-&gt;arg1, r))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-class: class not found: &quot; + s-&gt;arg1 +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot; in file: &quot; + filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-class: invalid region&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;auto begin = lines.begin() + r.start_line;<br>
&emsp;&emsp;&emsp;&emsp;auto end = lines.begin() + (r.end_line + 1);<br>
&emsp;&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;&emsp;&emsp;lines.insert(lines.begin() + r.start_line,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else // replace-cpp-method<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::string cls;<br>
&emsp;&emsp;&emsp;&emsp;std::string method;<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (!s-&gt;arg2.empty())<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;cls = s-&gt;arg1;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;method = s-&gt;arg2;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;auto pos = s-&gt;arg1.find(&quot;::&quot;);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (pos == std::string::npos)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-method: expected 'Class::method' or &quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;'Class method'&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;&emsp;cls = s-&gt;arg1.substr(0, pos);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;method = s-&gt;arg1.substr(pos + 2);<br>
&emsp;&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (cls.empty() || method.empty())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-method: empty class or method name&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;Region r;<br>
&emsp;&emsp;&emsp;&emsp;if (!finder.find_method(cls, method, r))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-method: method not found: &quot; + cls +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;::&quot; + method + &quot; in file: &quot; + filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-cpp-method: invalid region&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;auto begin = lines.begin() + r.start_line;<br>
&emsp;&emsp;&emsp;&emsp;auto end = lines.begin() + (r.end_line + 1);<br>
&emsp;&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;&emsp;&emsp;lines.insert(lines.begin() + r.start_line,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (s-&gt;command == &quot;replace-py-class&quot; ||<br>
&emsp;&emsp;&emsp;s-&gt;command == &quot;replace-py-method&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;PythonSymbolFinder finder(text);<br>
<br>
&emsp;&emsp;&emsp;if (s-&gt;command == &quot;replace-py-class&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;if (s-&gt;arg1.empty())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-class: missing class name for file: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;Region r;<br>
&emsp;&emsp;&emsp;&emsp;if (!finder.find_class(s-&gt;arg1, r))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-class: class not found: &quot; + s-&gt;arg1 +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot; in file: &quot; + filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-class: invalid region&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;auto begin = lines.begin() + r.start_line;<br>
&emsp;&emsp;&emsp;&emsp;auto end = lines.begin() + (r.end_line + 1);<br>
&emsp;&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;&emsp;&emsp;lines.insert(lines.begin() + r.start_line,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else // replace-py-method<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::string cls;<br>
&emsp;&emsp;&emsp;&emsp;std::string method;<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (!s-&gt;arg2.empty())<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;cls = s-&gt;arg1;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;method = s-&gt;arg2;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;auto pos = s-&gt;arg1.find('.');<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (pos == std::string::npos)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-method: expected 'Class.method' or &quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;'Class method'&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;&emsp;cls = s-&gt;arg1.substr(0, pos);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;method = s-&gt;arg1.substr(pos + 1);<br>
&emsp;&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (cls.empty() || method.empty())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-method: empty class or method name&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;Region r;<br>
&emsp;&emsp;&emsp;&emsp;if (!finder.find_method(cls, method, r))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-method: method not found: &quot; + cls + &quot;.&quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;method + &quot; in file: &quot; + filepath);<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (r.start_line &lt; 0 || r.end_line &lt; r.start_line ||<br>
&emsp;&emsp;&emsp;&emsp;&emsp;r.end_line &gt;= static_cast&lt;int&gt;(lines.size()))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;replace-py-method: invalid region&quot;);<br>
<br>
&emsp;&emsp;&emsp;&emsp;auto begin = lines.begin() + r.start_line;<br>
&emsp;&emsp;&emsp;&emsp;auto end = lines.begin() + (r.end_line + 1);<br>
&emsp;&emsp;&emsp;&emsp;lines.erase(begin, end);<br>
&emsp;&emsp;&emsp;&emsp;lines.insert(lines.begin() + r.start_line,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;payload.end());<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;throw std::runtime_error(&quot;apply_symbol_commands: unknown command: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;command);<br>
&emsp;}<br>
}<br>
<br>
static Section parse_section(std::istream &amp;in, const std::string &amp;header)<br>
{<br>
&emsp;Section s;<br>
<br>
&emsp;auto pos = header.find(':');<br>
&emsp;if (pos == std::string::npos)<br>
&emsp;&emsp;throw std::runtime_error(&quot;bad section header: &quot; + header);<br>
<br>
&emsp;auto pos2 = header.find(&quot;===&quot;, pos);<br>
&emsp;if (pos2 == std::string::npos)<br>
&emsp;&emsp;pos2 = header.size();<br>
<br>
&emsp;auto raw = header.substr(pos + 1, pos2 - pos - 1);<br>
&emsp;s.filepath = trim(raw);<br>
&emsp;if (s.filepath.empty())<br>
&emsp;&emsp;throw std::runtime_error(&quot;empty filepath in header: &quot; + header);<br>
<br>
&emsp;std::string line;<br>
&emsp;if (!std::getline(in, line))<br>
&emsp;&emsp;throw std::runtime_error(&quot;unexpected end after header&quot;);<br>
<br>
&emsp;if (line.rfind(&quot;--- &quot;, 0) != 0)<br>
&emsp;&emsp;throw std::runtime_error(&quot;expected command after header&quot;);<br>
<br>
&emsp;{<br>
&emsp;&emsp;std::istringstream ss(line.substr(4));<br>
&emsp;&emsp;ss &gt;&gt; s.command;<br>
<br>
&emsp;&emsp;// читаем остаток строки как аргументы команды<br>
&emsp;&emsp;std::string rest;<br>
&emsp;&emsp;std::getline(ss, rest);<br>
&emsp;&emsp;if (!rest.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::istringstream as(rest);<br>
&emsp;&emsp;&emsp;as &gt;&gt; s.arg1;<br>
&emsp;&emsp;&emsp;as &gt;&gt; s.arg2;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (is_text_command(s.command))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (s.command == &quot;create-file&quot; || s.command == &quot;delete-file&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (is_symbol_command(s.command))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;index-based commands removed: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s.command);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;bool found_end = false;<br>
&emsp;while (std::getline(in, line))<br>
&emsp;{<br>
&emsp;&emsp;if (line == &quot;=END=&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;found_end = true;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;s.payload.push_back(line);<br>
&emsp;}<br>
<br>
&emsp;if (!found_end)<br>
&emsp;&emsp;throw std::runtime_error(&quot;missing =END=&quot;);<br>
<br>
&emsp;if (is_text_command(s.command))<br>
&emsp;{<br>
&emsp;&emsp;// Определяем, в YAML-режиме мы или в старом формате.<br>
&emsp;&emsp;// Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется<br>
&emsp;&emsp;// старая логика.<br>
&emsp;&emsp;bool yaml_mode = false;<br>
&emsp;&emsp;std::size_t first_non_empty = 0;<br>
&emsp;&emsp;while (first_non_empty &lt; s.payload.size() &amp;&amp;<br>
&emsp;&emsp;&emsp;trim(s.payload[first_non_empty]).empty())<br>
&emsp;&emsp;&emsp;++first_non_empty;<br>
<br>
&emsp;&emsp;if (first_non_empty &lt; s.payload.size())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string t = trim(s.payload[first_non_empty]);<br>
&emsp;&emsp;&emsp;if (t == &quot;BEFORE:&quot; || t == &quot;MARKER:&quot; || t == &quot;AFTER:&quot;)<br>
&emsp;&emsp;&emsp;&emsp;yaml_mode = true;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (!yaml_mode)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// Старый режим: всё до '---' — marker, после — payload<br>
&emsp;&emsp;&emsp;auto it = std::find(<br>
&emsp;&emsp;&emsp;&emsp;s.payload.begin(), s.payload.end(), std::string(&quot;---&quot;));<br>
&emsp;&emsp;&emsp;if (it == s.payload.end())<br>
&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;text command requires '---' separator&quot;);<br>
<br>
&emsp;&emsp;&emsp;s.marker.assign(s.payload.begin(), it);<br>
<br>
&emsp;&emsp;&emsp;std::vector&lt;std::string&gt; tail;<br>
&emsp;&emsp;&emsp;if (std::next(it) != s.payload.end())<br>
&emsp;&emsp;&emsp;&emsp;tail.assign(std::next(it), s.payload.end());<br>
<br>
&emsp;&emsp;&emsp;s.payload.swap(tail);<br>
<br>
&emsp;&emsp;&emsp;if (s.marker.empty())<br>
&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(&quot;empty text marker&quot;);<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// YAML-подобный режим:<br>
&emsp;&emsp;&emsp;// BEFORE:<br>
&emsp;&emsp;&emsp;//   ...<br>
&emsp;&emsp;&emsp;// MARKER:<br>
&emsp;&emsp;&emsp;//   ...<br>
&emsp;&emsp;&emsp;// AFTER:<br>
&emsp;&emsp;&emsp;//   ...<br>
&emsp;&emsp;&emsp;// ---<br>
&emsp;&emsp;&emsp;// &lt;payload&gt;<br>
&emsp;&emsp;&emsp;s.before.clear();<br>
&emsp;&emsp;&emsp;s.marker.clear();<br>
&emsp;&emsp;&emsp;s.after.clear();<br>
<br>
&emsp;&emsp;&emsp;enum class Block<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;NONE,<br>
&emsp;&emsp;&emsp;&emsp;BEFORE,<br>
&emsp;&emsp;&emsp;&emsp;MARKER,<br>
&emsp;&emsp;&emsp;&emsp;AFTER<br>
&emsp;&emsp;&emsp;};<br>
<br>
&emsp;&emsp;&emsp;Block blk = Block::NONE;<br>
&emsp;&emsp;&emsp;std::vector&lt;std::string&gt; new_payload;<br>
<br>
&emsp;&emsp;&emsp;bool seen_separator = false;<br>
<br>
&emsp;&emsp;&emsp;for (std::size_t i = first_non_empty; i &lt; s.payload.size(); ++i)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;const std::string &amp;ln = s.payload[i];<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (!seen_separator &amp;&amp; ln == &quot;---&quot;)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;seen_separator = true;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;&emsp;if (!seen_separator)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string t = trim(ln);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (t == &quot;BEFORE:&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;blk = Block::BEFORE;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (t == &quot;MARKER:&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;blk = Block::MARKER;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (t == &quot;AFTER:&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;blk = Block::AFTER;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;&emsp;&emsp;switch (blk)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;case Block::BEFORE:<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s.before.push_back(ln);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;case Block::MARKER:<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s.marker.push_back(ln);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;case Block::AFTER:<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s.after.push_back(ln);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;case Block::NONE:<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;unexpected content before YAML block tag&quot;);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;new_payload.push_back(ln);<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;s.payload.swap(new_payload);<br>
<br>
&emsp;&emsp;&emsp;if (s.marker.empty())<br>
&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;YAML text command requires MARKER: section&quot;);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;return s;<br>
}<br>
<br>
static void apply_for_file(const std::string &amp;filepath,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::vector&lt;const Section *&gt; &amp;sections)<br>
{<br>
&emsp;fs::path p = filepath;<br>
&emsp;std::vector&lt;std::string&gt; orig;<br>
&emsp;bool existed = true;<br>
<br>
&emsp;try<br>
&emsp;{<br>
&emsp;&emsp;orig = read_file_lines(p);<br>
&emsp;}<br>
&emsp;catch (...)<br>
&emsp;{<br>
&emsp;&emsp;existed = false;<br>
&emsp;&emsp;orig.clear();<br>
&emsp;}<br>
<br>
&emsp;for (const Section *s : sections)<br>
&emsp;{<br>
&emsp;&emsp;if (!existed &amp;&amp; s-&gt;command == &quot;delete-file&quot;)<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;delete-file: file does not exist&quot;);<br>
&emsp;}<br>
<br>
&emsp;for (const Section *s : sections)<br>
&emsp;{<br>
&emsp;&emsp;if (s-&gt;command == &quot;create-file&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;write_file_lines(p, s-&gt;payload);<br>
&emsp;&emsp;&emsp;return;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;if (s-&gt;command == &quot;delete-file&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::error_code ec;<br>
&emsp;&emsp;&emsp;fs::remove(p, ec);<br>
&emsp;&emsp;&emsp;if (ec)<br>
&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(&quot;delete-file failed&quot;);<br>
&emsp;&emsp;&emsp;return;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;std::vector&lt;const Section *&gt; text_sections;<br>
&emsp;std::vector&lt;const Section *&gt; symbol_sections;<br>
<br>
&emsp;for (const Section *s : sections)<br>
&emsp;{<br>
&emsp;&emsp;if (is_text_command(s-&gt;command))<br>
&emsp;&emsp;&emsp;text_sections.push_back(s);<br>
&emsp;&emsp;else if (is_symbol_command(s-&gt;command))<br>
&emsp;&emsp;&emsp;symbol_sections.push_back(s);<br>
&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;throw std::runtime_error(&quot;unexpected non-text command: &quot; +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;s-&gt;command);<br>
&emsp;}<br>
<br>
&emsp;if (!text_sections.empty())<br>
&emsp;&emsp;apply_text_commands(filepath, orig, text_sections);<br>
<br>
&emsp;if (!symbol_sections.empty())<br>
&emsp;&emsp;apply_symbol_commands(filepath, orig, symbol_sections);<br>
<br>
&emsp;if (!text_sections.empty() || !symbol_sections.empty())<br>
&emsp;&emsp;write_file_lines(p, orig);<br>
}<br>
<br>
static void apply_all(const std::vector&lt;Section&gt; &amp;sections)<br>
{<br>
&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;// 1. Собираем список всех файлов, которые будут затронуты<br>
&emsp;std::vector&lt;std::string&gt; files;<br>
&emsp;files.reserve(sections.size());<br>
&emsp;for (auto &amp;s : sections)<br>
&emsp;&emsp;files.push_back(s.filepath);<br>
<br>
&emsp;std::sort(files.begin(), files.end());<br>
&emsp;files.erase(std::unique(files.begin(), files.end()), files.end());<br>
<br>
&emsp;struct Backup<br>
&emsp;{<br>
&emsp;&emsp;bool existed = false;<br>
&emsp;&emsp;std::vector&lt;std::string&gt; lines;<br>
&emsp;};<br>
<br>
&emsp;std::map&lt;std::string, Backup&gt; backup;<br>
<br>
&emsp;// 2. Делаем резервную копию всех файлов<br>
&emsp;for (auto &amp;f : files)<br>
&emsp;{<br>
&emsp;&emsp;Backup b;<br>
&emsp;&emsp;fs::path p = f;<br>
<br>
&emsp;&emsp;std::error_code ec;<br>
<br>
&emsp;&emsp;if (fs::exists(p, ec))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;b.existed = true;<br>
<br>
&emsp;&emsp;&emsp;try<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;b.lines = read_file_lines(p);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;catch (...)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;throw std::runtime_error(&quot;cannot read original file: &quot; + f);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;b.existed = false;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;backup[f] = std::move(b);<br>
&emsp;}<br>
<br>
&emsp;// 3. Применяем секции с защитой (try/catch)<br>
&emsp;try<br>
&emsp;{<br>
&emsp;&emsp;for (auto &amp;s : sections)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::vector&lt;const Section *&gt; single{&amp;s};<br>
&emsp;&emsp;&emsp;apply_for_file(s.filepath, single);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
&emsp;catch (...)<br>
&emsp;{<br>
&emsp;&emsp;// 4. Откат (rollback)<br>
&emsp;&emsp;for (auto &amp;[path, b] : backup)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;fs::path p = path;<br>
&emsp;&emsp;&emsp;std::error_code ec;<br>
<br>
&emsp;&emsp;&emsp;if (b.existed)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;try<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;write_file_lines(p, b.lines);<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;catch (...)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;// если даже откат не удался — сдаёмся<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;fs::remove(p, ec);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;throw;<br>
&emsp;}<br>
}<br>
<br>
int apply_chunk_main(int argc, char **argv)<br>
{<br>
&emsp;if (argc &lt; 2)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;usage: apply_patch &lt;patchfile&gt;\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;std::ifstream in(argv[1]);<br>
&emsp;if (!in)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;cannot open patch file: &quot; &lt;&lt; argv[1] &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;std::string line;<br>
&emsp;std::vector&lt;Section&gt; sections;<br>
&emsp;int seq = 0;<br>
<br>
&emsp;try<br>
&emsp;{<br>
&emsp;&emsp;while (std::getline(in, line))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (line.rfind(&quot;=== file:&quot;, 0) == 0)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;Section s = parse_section(in, line);<br>
&emsp;&emsp;&emsp;&emsp;s.seq = seq++;<br>
&emsp;&emsp;&emsp;&emsp;sections.push_back(std::move(s));<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;apply_all(sections);<br>
&emsp;}<br>
&emsp;catch (const std::exception &amp;e)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;error while applying patch: &quot; &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

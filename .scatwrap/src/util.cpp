<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;util.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt; // у тебя уже есть для std::cerr<br>
#include &lt;sstream&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
extern bool g_use_absolute_paths;<br>
<br>
bool starts_with(const std::string &amp;s, const std::string &amp;prefix)<br>
{<br>
&#9;return s.size() &gt;= prefix.size() &amp;&amp;<br>
&#9;&#9;s.compare(0, prefix.size(), prefix) == 0;<br>
}<br>
<br>
bool ends_with(const std::string &amp;s, const std::string &amp;suffix)<br>
{<br>
&#9;return s.size() &gt;= suffix.size() &amp;&amp;<br>
&#9;&#9;s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;<br>
}<br>
<br>
std::string make_display_path(const fs::path &amp;p)<br>
{<br>
&#9;if (g_use_absolute_paths)<br>
&#9;&#9;return fs::absolute(p).string();<br>
<br>
&#9;std::error_code ec;<br>
&#9;fs::path rel = fs::relative(p, fs::current_path(), ec);<br>
&#9;if (!ec)<br>
&#9;&#9;return rel.string();<br>
<br>
&#9;return p.string();<br>
}<br>
<br>
void dump_file(const fs::path &amp;p, bool first, const Options &amp;opt)<br>
{<br>
&#9;std::ifstream in(p, std::ios::binary);<br>
&#9;if (!in)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;Cannot open: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;return;<br>
&#9;}<br>
<br>
&#9;if (!first)<br>
&#9;&#9;std::cout &lt;&lt; &quot;\n&quot;;<br>
<br>
&#9;std::cout &lt;&lt; &quot;===== &quot; &lt;&lt; make_display_path(p) &lt;&lt; &quot; =====\n&quot;;<br>
&#9;// std::cout &lt;&lt; in.rdbuf() &lt;&lt; &quot;=EOF=\n&quot;;<br>
<br>
&#9;std::string line;<br>
&#9;size_t line_no = 0;<br>
&#9;while (std::getline(in, line))<br>
&#9;{<br>
&#9;&#9;if (opt.line_numbers)<br>
&#9;&#9;&#9;std::cout &lt;&lt; line_no &lt;&lt; &quot;: &quot; &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;else<br>
&#9;&#9;&#9;std::cout &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;++line_no;<br>
&#9;}<br>
}<br>
std::uintmax_t get_file_size(const fs::path &amp;p)<br>
{<br>
&#9;std::error_code ec;<br>
&#9;auto sz = fs::file_size(p, ec);<br>
&#9;if (ec)<br>
&#9;&#9;return 0;<br>
&#9;return sz;<br>
}<br>
<br>
bool match_simple(const fs::path &amp;p, const std::string &amp;pat)<br>
{<br>
&#9;std::string name = p.filename().string();<br>
<br>
&#9;if (pat == &quot;*&quot;)<br>
&#9;&#9;return true;<br>
<br>
&#9;auto pos = pat.find('*');<br>
&#9;if (pos == std::string::npos)<br>
&#9;&#9;return name == pat;<br>
<br>
&#9;std::string a = pat.substr(0, pos);<br>
&#9;std::string b = pat.substr(pos + 1);<br>
<br>
&#9;return starts_with(name, a) &amp;&amp; ends_with(name, b);<br>
}<br>
<br>
std::string html_escape(std::string_view src)<br>
{<br>
&#9;std::string out;<br>
&#9;out.reserve(src.size() * 11 / 10 + 16); // чуть с запасом<br>
<br>
&#9;for (char ch : src)<br>
&#9;{<br>
&#9;&#9;switch (ch)<br>
&#9;&#9;{<br>
&#9;&#9;case '&amp;':<br>
&#9;&#9;&#9;out += &quot;&amp;amp;&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '&lt;':<br>
&#9;&#9;&#9;out += &quot;&amp;lt;&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '&gt;':<br>
&#9;&#9;&#9;out += &quot;&amp;gt;&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '&quot;':<br>
&#9;&#9;&#9;out += &quot;&amp;quot;&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;default:<br>
&#9;&#9;&#9;out.push_back(ch);<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;return out;<br>
}<br>
<br>
std::string wrap_cpp_as_html(std::string_view cpp_code,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;std::string_view title)<br>
{<br>
&#9;std::string html;<br>
&#9;html.reserve(cpp_code.size() * 11 / 10 + 256);<br>
<br>
&#9;html += &quot;&lt;!DOCTYPE html&gt;\n&quot;;<br>
&#9;html += &quot;&lt;html lang=\&quot;en\&quot;&gt;\n&quot;;<br>
&#9;html += &quot;&lt;head&gt;\n&quot;;<br>
&#9;html += &quot;  &lt;meta charset=\&quot;UTF-8\&quot;&gt;\n&quot;;<br>
&#9;html += &quot;  &lt;title&gt;&quot;;<br>
&#9;html += html_escape(title);      // title экранируем, пробелы оставляем обычными<br>
&#9;html += &quot;&lt;/title&gt;\n&quot;;<br>
&#9;html += &quot;&lt;/head&gt;\n&quot;;<br>
&#9;html += &quot;&lt;body&gt;\n&quot;;<br>
<br>
&#9;html += &quot;&lt;!-- BEGIN SCAT CODE --&gt;\n&quot;;<br>
<br>
&#9;std::size_t pos = 0;<br>
&#9;const std::size_t n = cpp_code.size();<br>
<br>
&#9;while (pos &lt; n)<br>
&#9;{<br>
&#9;&#9;std::size_t nl = cpp_code.find('\n', pos);<br>
&#9;&#9;if (nl == std::string_view::npos)<br>
&#9;&#9;&#9;nl = n;<br>
<br>
&#9;&#9;std::string_view line = cpp_code.substr(pos, nl - pos);<br>
<br>
&#9;&#9;// 1) экранируем спецсимволы (&amp;, &lt;, &gt;, &quot;)<br>
&#9;&#9;std::string escaped = html_escape(line);<br>
<br>
&#9;&#9;// 2) превращаем каждый пробел в &amp;nbsp;, чтобы отступы не схлопывались<br>
&#9;&#9;for (char ch : escaped)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (ch == ' ')<br>
&#9;&#9;&#9;&#9;html += &quot;&amp;nbsp;&quot;;<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;html.push_back(ch);<br>
&#9;&#9;}<br>
<br>
&#9;&#9;html += &quot;&lt;br&gt;\n&quot;;<br>
<br>
&#9;&#9;pos = nl + 1;<br>
&#9;}<br>
<br>
&#9;html += &quot;&lt;!-- END SCAT CODE --&gt;\n&quot;;<br>
<br>
&#9;html += &quot;&lt;/body&gt;\n&quot;;<br>
&#9;html += &quot;&lt;/html&gt;\n&quot;;<br>
<br>
&#9;return html;<br>
}<br>
<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>

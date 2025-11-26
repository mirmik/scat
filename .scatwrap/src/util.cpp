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
&emsp;return s.size() &gt;= prefix.size() &amp;&amp;<br>
&emsp;&emsp;s.compare(0, prefix.size(), prefix) == 0;<br>
}<br>
<br>
bool ends_with(const std::string &amp;s, const std::string &amp;suffix)<br>
{<br>
&emsp;return s.size() &gt;= suffix.size() &amp;&amp;<br>
&emsp;&emsp;s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;<br>
}<br>
<br>
std::string make_display_path(const fs::path &amp;p)<br>
{<br>
&emsp;if (g_use_absolute_paths)<br>
&emsp;&emsp;return fs::absolute(p).string();<br>
<br>
&emsp;std::error_code ec;<br>
&emsp;fs::path rel = fs::relative(p, fs::current_path(), ec);<br>
&emsp;if (!ec)<br>
&emsp;&emsp;return rel.string();<br>
<br>
&emsp;return p.string();<br>
}<br>
<br>
void dump_file(const fs::path &amp;p, bool first, const Options &amp;opt)<br>
{<br>
&emsp;std::ifstream in(p, std::ios::binary);<br>
&emsp;if (!in)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;Cannot open: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
<br>
&emsp;if (!first)<br>
&emsp;&emsp;std::cout &lt;&lt; &quot;\n&quot;;<br>
<br>
&emsp;std::cout &lt;&lt; &quot;===== &quot; &lt;&lt; make_display_path(p) &lt;&lt; &quot; =====\n&quot;;<br>
&emsp;// std::cout &lt;&lt; in.rdbuf() &lt;&lt; &quot;=EOF=\n&quot;;<br>
<br>
&emsp;std::string line;<br>
&emsp;size_t line_no = 0;<br>
&emsp;while (std::getline(in, line))<br>
&emsp;{<br>
&emsp;&emsp;if (opt.line_numbers)<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; line_no &lt;&lt; &quot;: &quot; &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;++line_no;<br>
&emsp;}<br>
}<br>
std::uintmax_t get_file_size(const fs::path &amp;p)<br>
{<br>
&emsp;std::error_code ec;<br>
&emsp;auto sz = fs::file_size(p, ec);<br>
&emsp;if (ec)<br>
&emsp;&emsp;return 0;<br>
&emsp;return sz;<br>
}<br>
<br>
bool match_simple(const fs::path &amp;p, const std::string &amp;pat)<br>
{<br>
&emsp;std::string name = p.filename().string();<br>
<br>
&emsp;if (pat == &quot;*&quot;)<br>
&emsp;&emsp;return true;<br>
<br>
&emsp;auto pos = pat.find('*');<br>
&emsp;if (pos == std::string::npos)<br>
&emsp;&emsp;return name == pat;<br>
<br>
&emsp;std::string a = pat.substr(0, pos);<br>
&emsp;std::string b = pat.substr(pos + 1);<br>
<br>
&emsp;return starts_with(name, a) &amp;&amp; ends_with(name, b);<br>
}<br>
<br>
std::string html_escape(std::string_view src)<br>
{<br>
&emsp;std::string out;<br>
&emsp;out.reserve(src.size() * 11 / 10 + 16); // чуть с запасом<br>
<br>
&emsp;for (char ch : src)<br>
&emsp;{<br>
&emsp;&emsp;switch (ch)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;case '&amp;':<br>
&emsp;&emsp;&emsp;out += &quot;&amp;amp;&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '&lt;':<br>
&emsp;&emsp;&emsp;out += &quot;&amp;lt;&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '&gt;':<br>
&emsp;&emsp;&emsp;out += &quot;&amp;gt;&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '&quot;':<br>
&emsp;&emsp;&emsp;out += &quot;&amp;quot;&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;default:<br>
&emsp;&emsp;&emsp;out.push_back(ch);<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;return out;<br>
}<br>
<br>
std::string wrap_cpp_as_html(std::string_view cpp_code,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::string_view title)<br>
{<br>
&emsp;std::string html;<br>
&emsp;html.reserve(cpp_code.size() * 11 / 10 + 256);<br>
<br>
&emsp;html += &quot;&lt;!DOCTYPE html&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;html lang=\&quot;en\&quot;&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;head&gt;\n&quot;;<br>
&emsp;html += &quot;  &lt;meta charset=\&quot;UTF-8\&quot;&gt;\n&quot;;<br>
&emsp;html += &quot;  &lt;title&gt;&quot;;<br>
&emsp;html += html_escape(title);<br>
&emsp;html += &quot;&lt;/title&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;/head&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;body&gt;\n&quot;;<br>
<br>
&emsp;html += &quot;&lt;!-- BEGIN SCAT CODE --&gt;\n&quot;;<br>
<br>
&emsp;std::size_t pos = 0;<br>
&emsp;const std::size_t n = cpp_code.size();<br>
<br>
&emsp;while (pos &lt; n)<br>
&emsp;{<br>
&emsp;&emsp;std::size_t nl = cpp_code.find('\n', pos);<br>
&emsp;&emsp;if (nl == std::string_view::npos)<br>
&emsp;&emsp;&emsp;nl = n;<br>
<br>
&emsp;&emsp;std::string_view line = cpp_code.substr(pos, nl - pos);<br>
<br>
&emsp;&emsp;// считаем ведущие пробелы<br>
&emsp;&emsp;std::size_t lead_spaces = 0;<br>
&emsp;&emsp;while (lead_spaces &lt; line.size() &amp;&amp; line[lead_spaces] == ' ')<br>
&emsp;&emsp;&emsp;++lead_spaces;<br>
<br>
&emsp;&emsp;// каждые 4 пробела -&gt; одна &quot;ступень&quot; отступа<br>
&emsp;&emsp;std::size_t em_count = lead_spaces / 4;<br>
<br>
&emsp;&emsp;for (std::size_t i = 0; i &lt; em_count; ++i)<br>
&emsp;&emsp;&emsp;html += &quot;&amp;emsp;&quot;; // em space как HTML-сущность<br>
<br>
&emsp;&emsp;// остаток (lead_spaces % 4) выбрасываем<br>
&emsp;&emsp;std::string_view rest = line.substr(lead_spaces);<br>
<br>
&emsp;&emsp;// экранируем спецсимволы только в полезной части строки<br>
&emsp;&emsp;html += html_escape(rest);<br>
<br>
&emsp;&emsp;html += &quot;&lt;br&gt;\n&quot;;<br>
<br>
&emsp;&emsp;pos = nl + 1;<br>
&emsp;}<br>
<br>
&emsp;html += &quot;&lt;!-- END SCAT CODE --&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;/body&gt;\n&quot;;<br>
&emsp;html += &quot;&lt;/html&gt;\n&quot;;<br>
<br>
&emsp;return html;<br>
}<br>
<br>
<br>
<br>
<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>

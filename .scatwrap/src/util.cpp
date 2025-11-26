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
    return s.size() &gt;= prefix.size() &amp;&amp;<br>
           s.compare(0, prefix.size(), prefix) == 0;<br>
}<br>
<br>
bool ends_with(const std::string &amp;s, const std::string &amp;suffix)<br>
{<br>
    return s.size() &gt;= suffix.size() &amp;&amp;<br>
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;<br>
}<br>
<br>
std::string make_display_path(const fs::path &amp;p)<br>
{<br>
    if (g_use_absolute_paths)<br>
        return fs::absolute(p).string();<br>
<br>
    std::error_code ec;<br>
    fs::path rel = fs::relative(p, fs::current_path(), ec);<br>
    if (!ec)<br>
        return rel.string();<br>
<br>
    return p.string();<br>
}<br>
<br>
void dump_file(const fs::path &amp;p, bool first, const Options &amp;opt)<br>
{<br>
    std::ifstream in(p, std::ios::binary);<br>
    if (!in)<br>
    {<br>
        std::cerr &lt;&lt; &quot;Cannot open: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
        return;<br>
    }<br>
<br>
    if (!first)<br>
        std::cout &lt;&lt; &quot;\n&quot;;<br>
<br>
    std::cout &lt;&lt; &quot;===== &quot; &lt;&lt; make_display_path(p) &lt;&lt; &quot; =====\n&quot;;<br>
    // std::cout &lt;&lt; in.rdbuf() &lt;&lt; &quot;=EOF=\n&quot;;<br>
<br>
    std::string line;<br>
    size_t line_no = 0;<br>
    while (std::getline(in, line))<br>
    {<br>
        if (opt.line_numbers)<br>
            std::cout &lt;&lt; line_no &lt;&lt; &quot;: &quot; &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
        else<br>
            std::cout &lt;&lt; line &lt;&lt; &quot;\n&quot;;<br>
        ++line_no;<br>
    }<br>
}<br>
std::uintmax_t get_file_size(const fs::path &amp;p)<br>
{<br>
    std::error_code ec;<br>
    auto sz = fs::file_size(p, ec);<br>
    if (ec)<br>
        return 0;<br>
    return sz;<br>
}<br>
<br>
bool match_simple(const fs::path &amp;p, const std::string &amp;pat)<br>
{<br>
    std::string name = p.filename().string();<br>
<br>
    if (pat == &quot;*&quot;)<br>
        return true;<br>
<br>
    auto pos = pat.find('*');<br>
    if (pos == std::string::npos)<br>
        return name == pat;<br>
<br>
    std::string a = pat.substr(0, pos);<br>
    std::string b = pat.substr(pos + 1);<br>
<br>
    return starts_with(name, a) &amp;&amp; ends_with(name, b);<br>
}<br>
<br>
std::string html_escape(std::string_view src)<br>
{<br>
    std::string out;<br>
    out.reserve(src.size() * 11 / 10 + 16); // чуть с запасом<br>
<br>
    for (char ch : src)<br>
    {<br>
        switch (ch)<br>
        {<br>
        case '&amp;':<br>
            out += &quot;&amp;amp;&quot;;<br>
            break;<br>
        case '&lt;':<br>
            out += &quot;&amp;lt;&quot;;<br>
            break;<br>
        case '&gt;':<br>
            out += &quot;&amp;gt;&quot;;<br>
            break;<br>
        case '&quot;':<br>
            out += &quot;&amp;quot;&quot;;<br>
            break;<br>
        default:<br>
            out.push_back(ch);<br>
            break;<br>
        }<br>
    }<br>
<br>
    return out;<br>
}<br>
<br>
std::string wrap_cpp_as_html(std::string_view cpp_code,<br>
                             std::string_view title)<br>
{<br>
    std::string html;<br>
    html.reserve(cpp_code.size() * 11 / 10 + 256);<br>
<br>
    html += &quot;&lt;!DOCTYPE html&gt;\n&quot;;<br>
    html += &quot;&lt;html lang=\&quot;en\&quot;&gt;\n&quot;;<br>
    html += &quot;&lt;head&gt;\n&quot;;<br>
    html += &quot;  &lt;meta charset=\&quot;UTF-8\&quot;&gt;\n&quot;;<br>
    html += &quot;  &lt;title&gt;&quot;;<br>
    html += html_escape(title);      // на всякий случай экранируем title<br>
    html += &quot;&lt;/title&gt;\n&quot;;<br>
    html += &quot;&lt;/head&gt;\n&quot;;<br>
    html += &quot;&lt;body&gt;\n&quot;;<br>
<br>
    html += &quot;&lt;!-- BEGIN SCAT CODE --&gt;\n&quot;;<br>
<br>
    // Разбиваем по строкам и каждая строка = escaped + &lt;br&gt;<br>
    std::size_t pos = 0;<br>
    const std::size_t n = cpp_code.size();<br>
<br>
    while (pos &lt; n)<br>
    {<br>
        std::size_t nl = cpp_code.find('\n', pos);<br>
        if (nl == std::string_view::npos)<br>
            nl = n;<br>
<br>
        std::string_view line = cpp_code.substr(pos, nl - pos);<br>
        html += html_escape(line);<br>
        html += &quot;&lt;br&gt;\n&quot;;<br>
<br>
        pos = nl + 1; // если nl == n, pos станет &gt; n и цикл закончится<br>
    }<br>
<br>
    html += &quot;&lt;!-- END SCAT CODE --&gt;\n&quot;;<br>
<br>
    html += &quot;&lt;/body&gt;\n&quot;;<br>
    html += &quot;&lt;/html&gt;\n&quot;;<br>
<br>
    return html;<br>
}<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>

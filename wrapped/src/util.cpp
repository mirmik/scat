<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.cpp</title>
</head>
<body>
<pre><code>
#include &quot;util.h&quot;
#include &lt;fstream&gt;
#include &lt;sstream&gt;
#include &lt;iostream&gt;      // у тебя уже есть для std::cerr
#include &lt;filesystem&gt;


namespace fs = std::filesystem;

extern bool g_use_absolute_paths;

bool starts_with(const std::string&amp; s, const std::string&amp; prefix)
{
    return s.size() &gt;= prefix.size() &amp;&amp; s.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string&amp; s, const std::string&amp; suffix)
{
    return s.size() &gt;= suffix.size() &amp;&amp; s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string make_display_path(const fs::path&amp; p)
{
    if (g_use_absolute_paths)
        return fs::absolute(p).string();

    std::error_code ec;
    fs::path rel = fs::relative(p, fs::current_path(), ec);
    if (!ec)
        return rel.string();

    return p.string();
}

void dump_file(const fs::path&amp; p, bool first, const Options&amp; opt)
{
    std::ifstream in(p, std::ios::binary);
    if (!in)
    {
        std::cerr &lt;&lt; &quot;Cannot open: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;
        return;
    }

    if (!first)
        std::cout &lt;&lt; &quot;\n&quot;;

    std::cout &lt;&lt; &quot;===== &quot; &lt;&lt; make_display_path(p) &lt;&lt; &quot; =====\n&quot;;
    //std::cout &lt;&lt; in.rdbuf() &lt;&lt; &quot;=EOF=\n&quot;;

    std::string line;
    size_t line_no = 0;
    while (std::getline(in, line))
    {
        if (opt.line_numbers)
            std::cout &lt;&lt; line_no &lt;&lt; &quot;: &quot; &lt;&lt; line &lt;&lt; &quot;\n&quot;;
        else
            std::cout &lt;&lt; line &lt;&lt; &quot;\n&quot;;
        ++line_no;
    }
}
std::uintmax_t get_file_size(const fs::path&amp; p)
{
    std::error_code ec;
    auto sz = fs::file_size(p, ec);
    if (ec)
        return 0;
    return sz;
}

bool match_simple(const fs::path&amp; p, const std::string&amp; pat)
{
    std::string name = p.filename().string();

    if (pat == &quot;*&quot;)
        return true;

    auto pos = pat.find('*');
    if (pos == std::string::npos)
        return name == pat;

    std::string a = pat.substr(0, pos);
    std::string b = pat.substr(pos + 1);

    return starts_with(name, a) &amp;&amp; ends_with(name, b);
}

std::string html_escape(std::string_view src) {
    std::string out;
    out.reserve(src.size() * 11 / 10 + 16); // чуть с запасом

    for (char ch : src) {
        switch (ch) {
        case '&amp;':
            out += &quot;&amp;amp;&quot;;
            break;
        case '&lt;':
            out += &quot;&amp;lt;&quot;;
            break;
        case '&gt;':
            out += &quot;&amp;gt;&quot;;
            break;
        case '&quot;':
            out += &quot;&amp;quot;&quot;;
            break;
        default:
            out.push_back(ch);
            break;
        }
    }

    return out;
}



std::string wrap_cpp_as_html(std::string_view cpp_code,
                             std::string_view title) {
    std::string escaped = html_escape(cpp_code);

    std::string html;
    html.reserve(escaped.size() + 256);

    html += &quot;&lt;!DOCTYPE html&gt;\n&quot;;
    html += &quot;&lt;html lang=\&quot;en\&quot;&gt;\n&quot;;
    html += &quot;&lt;head&gt;\n&quot;;
    html += &quot;  &lt;meta charset=\&quot;UTF-8\&quot;&gt;\n&quot;;
    html += &quot;  &lt;title&gt;&quot;;
    html += std::string(title);
    html += &quot;&lt;/title&gt;\n&quot;;
    html += &quot;&lt;/head&gt;\n&quot;;
    html += &quot;&lt;body&gt;\n&quot;;
    html += &quot;&lt;pre&gt;&lt;code&gt;\n&quot;;
    html += escaped;
    html += &quot;\n&lt;/code&gt;&lt;/pre&gt;\n&quot;;
    html += &quot;&lt;/body&gt;\n&quot;;
    html += &quot;&lt;/html&gt;\n&quot;;

    return html;
}

</code></pre>
</body>
</html>

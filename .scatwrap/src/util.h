<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &quot;options.h&quot;
#include &lt;cstdint&gt;
#include &lt;filesystem&gt;
#include &lt;string&gt;
#include &lt;string_view&gt;
#include &lt;vector&gt;

std::string make_display_path(const std::filesystem::path &amp;p);
void dump_file(const std::filesystem::path &amp;p, bool first, const Options &amp;opt);

// Безопасное получение размера файла; при ошибке возвращает 0.
std::uintmax_t get_file_size(const std::filesystem::path &amp;p);

bool match_simple(const std::filesystem::path &amp;p, const std::string &amp;mask);

// HTML helpers
std::string html_escape(std::string_view src);
std::string wrap_cpp_as_html(std::string_view cpp_code,
                             std::string_view title = &quot;C++ code&quot;);
</code></pre>
</body>
</html>

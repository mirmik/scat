<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &quot;options.h&quot;<br>
#include &lt;cstdint&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;string&gt;<br>
#include &lt;string_view&gt;<br>
#include &lt;vector&gt;<br>
<br>
std::string make_display_path(const std::filesystem::path &amp;p);<br>
void dump_file(const std::filesystem::path &amp;p, bool first, const Options &amp;opt);<br>
<br>
// Безопасное получение размера файла; при ошибке возвращает 0.<br>
std::uintmax_t get_file_size(const std::filesystem::path &amp;p);<br>
<br>
bool match_simple(const std::filesystem::path &amp;p, const std::string &amp;mask);<br>
<br>
// HTML helpers<br>
std::string html_escape(std::string_view src);<br>
std::string wrap_cpp_as_html(std::string_view cpp_code,<br>
                             std::string_view title = &quot;C++ code&quot;);<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&quot;options.h&quot;<br>
#include&#32;&lt;cstdint&gt;<br>
#include&#32;&lt;filesystem&gt;<br>
#include&#32;&lt;string&gt;<br>
#include&#32;&lt;string_view&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
std::string&#32;make_display_path(const&#32;std::filesystem::path&#32;&amp;p);<br>
void&#32;dump_file(const&#32;std::filesystem::path&#32;&amp;p,&#32;bool&#32;first,&#32;const&#32;Options&#32;&amp;opt);<br>
<br>
//&#32;Безопасное&#32;получение&#32;размера&#32;файла;&#32;при&#32;ошибке&#32;возвращает&#32;0.<br>
std::uintmax_t&#32;get_file_size(const&#32;std::filesystem::path&#32;&amp;p);<br>
<br>
bool&#32;match_simple(const&#32;std::filesystem::path&#32;&amp;p,&#32;const&#32;std::string&#32;&amp;mask);<br>
<br>
//&#32;HTML&#32;helpers<br>
std::string&#32;html_escape(std::string_view&#32;src);<br>
std::string&#32;wrap_cpp_as_html(std::string_view&#32;cpp_code,<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;std::string_view&#32;title&#32;=&#32;&quot;C++&#32;code&quot;);<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&quot;options.h&quot;<br>
#include&nbsp;&lt;cstdint&gt;<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;string_view&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
std::string&nbsp;make_display_path(const&nbsp;std::filesystem::path&nbsp;&amp;p);<br>
void&nbsp;dump_file(const&nbsp;std::filesystem::path&nbsp;&amp;p,&nbsp;bool&nbsp;first,&nbsp;const&nbsp;Options&nbsp;&amp;opt);<br>
<br>
//&nbsp;Безопасное&nbsp;получение&nbsp;размера&nbsp;файла;&nbsp;при&nbsp;ошибке&nbsp;возвращает&nbsp;0.<br>
std::uintmax_t&nbsp;get_file_size(const&nbsp;std::filesystem::path&nbsp;&amp;p);<br>
<br>
bool&nbsp;match_simple(const&nbsp;std::filesystem::path&nbsp;&amp;p,&nbsp;const&nbsp;std::string&nbsp;&amp;mask);<br>
<br>
//&nbsp;HTML&nbsp;helpers<br>
std::string&nbsp;html_escape(std::string_view&nbsp;src);<br>
std::string&nbsp;wrap_cpp_as_html(std::string_view&nbsp;cpp_code,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string_view&nbsp;title&nbsp;=&nbsp;&quot;C++&nbsp;code&quot;);<br>
<!-- END SCAT CODE -->
</body>
</html>

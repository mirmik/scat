<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/util.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&quot;options.h&quot;<br>
#include &#20;&lt;cstdint&gt;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;string_view&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
std::string &#20;make_display_path(const &#20;std::filesystem::path &#20;&amp;p);<br>
void &#20;dump_file(const &#20;std::filesystem::path &#20;&amp;p, &#20;bool &#20;first, &#20;const &#20;Options &#20;&amp;opt);<br>
<br>
// &#20;Безопасное &#20;получение &#20;размера &#20;файла; &#20;при &#20;ошибке &#20;возвращает &#20;0.<br>
std::uintmax_t &#20;get_file_size(const &#20;std::filesystem::path &#20;&amp;p);<br>
<br>
bool &#20;match_simple(const &#20;std::filesystem::path &#20;&amp;p, &#20;const &#20;std::string &#20;&amp;mask);<br>
<br>
// &#20;HTML &#20;helpers<br>
std::string &#20;html_escape(std::string_view &#20;src);<br>
std::string &#20;wrap_cpp_as_html(std::string_view &#20;cpp_code,<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;std::string_view &#20;title &#20;= &#20;&quot;C++ &#20;code&quot;);<br>
<!-- END SCAT CODE -->
</body>
</html>

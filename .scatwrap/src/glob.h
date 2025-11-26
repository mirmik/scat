<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
//&nbsp;expand_glob:<br>
//&nbsp;&nbsp;&nbsp;Поддерживает&nbsp;паттерны:<br>
//&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;&quot;*&quot;&nbsp;внутри&nbsp;уровня<br>
//&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;&quot;**&quot;&nbsp;—&nbsp;рекурсивный&nbsp;обход<br>
//&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;сложные&nbsp;пути&nbsp;вида&nbsp;&quot;foo/*/bar/**/*.txt&quot;<br>
//<br>
//&nbsp;Возвращает&nbsp;список&nbsp;regular&nbsp;files.<br>
std::vector&lt;std::filesystem::path&gt;&nbsp;expand_glob(const&nbsp;std::string&nbsp;&amp;pattern);<br>
<!-- END SCAT CODE -->
</body>
</html>

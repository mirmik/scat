<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&lt;filesystem&gt;<br>
#include&#32;&lt;string&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
//&#32;expand_glob:<br>
//&#32;&#32;&#32;Поддерживает&#32;паттерны:<br>
//&#32;&#32;&#32;&#32;&#32;*&#32;&quot;*&quot;&#32;внутри&#32;уровня<br>
//&#32;&#32;&#32;&#32;&#32;*&#32;&quot;**&quot;&#32;—&#32;рекурсивный&#32;обход<br>
//&#32;&#32;&#32;&#32;&#32;*&#32;сложные&#32;пути&#32;вида&#32;&quot;foo/*/bar/**/*.txt&quot;<br>
//<br>
//&#32;Возвращает&#32;список&#32;regular&#32;files.<br>
std::vector&lt;std::filesystem::path&gt;&#32;expand_glob(const&#32;std::string&#32;&amp;pattern);<br>
<!-- END SCAT CODE -->
</body>
</html>

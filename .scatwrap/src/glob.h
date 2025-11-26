<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
// &#20;expand_glob:<br>
// &#20; &#20; &#20;Поддерживает &#20;паттерны:<br>
// &#20; &#20; &#20; &#20; &#20;* &#20;&quot;*&quot; &#20;внутри &#20;уровня<br>
// &#20; &#20; &#20; &#20; &#20;* &#20;&quot;**&quot; &#20;— &#20;рекурсивный &#20;обход<br>
// &#20; &#20; &#20; &#20; &#20;* &#20;сложные &#20;пути &#20;вида &#20;&quot;foo/*/bar/**/*.txt&quot;<br>
//<br>
// &#20;Возвращает &#20;список &#20;regular &#20;files.<br>
std::vector&lt;std::filesystem::path&gt; &#20;expand_glob(const &#20;std::string &#20;&amp;pattern);<br>
<!-- END SCAT CODE -->
</body>
</html>

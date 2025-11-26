<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &lt;filesystem&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
// expand_glob:<br>
//   Поддерживает паттерны:<br>
//     * &quot;*&quot; внутри уровня<br>
//     * &quot;**&quot; — рекурсивный обход<br>
//     * сложные пути вида &quot;foo/*/bar/**/*.txt&quot;<br>
//<br>
// Возвращает список regular files.<br>
std::vector&lt;std::filesystem::path&gt; expand_glob(const std::string &amp;pattern);<br>
<!-- END SCAT CODE -->
</body>
</html>

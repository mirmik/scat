<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &lt;filesystem&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

// expand_glob:
//   Поддерживает паттерны:
//     * &quot;*&quot; внутри уровня
//     * &quot;**&quot; — рекурсивный обход
//     * сложные пути вида &quot;foo/*/bar/**/*.txt&quot;
//
// Возвращает список regular files.
std::vector&lt;std::filesystem::path&gt;
expand_glob(const std::string&amp; pattern);

</code></pre>
</body>
</html>

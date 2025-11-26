<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &lt;vector&gt;
#include &lt;filesystem&gt;
#include &lt;string&gt;
#include &quot;rules.h&quot;

struct Config {
    std::vector&lt;Rule&gt; text_rules;
    std::vector&lt;Rule&gt; tree_rules;
    std::string       map_format;   // новый блок [MAPFORMAT], может быть пустым
};

Config parse_config(const std::filesystem::path&amp; path);

</code></pre>
</body>
</html>

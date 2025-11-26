<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &quot;rules.h&quot;
#include &lt;filesystem&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

struct Config
{
    std::vector&lt;Rule&gt; text_rules;
    std::vector&lt;Rule&gt; tree_rules;
    std::string map_format; // новый блок [MAPFORMAT], может быть пустым
};

Config parse_config(const std::filesystem::path &amp;path);

</code></pre>
</body>
</html>

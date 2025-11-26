<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &quot;rules.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
struct Config<br>
{<br>
&#9;std::vector&lt;Rule&gt; text_rules;<br>
&#9;std::vector&lt;Rule&gt; tree_rules;<br>
&#9;std::string map_format; // новый блок [MAPFORMAT], может быть пустым<br>
};<br>
<br>
Config parse_config(const std::filesystem::path &amp;path);<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&quot;rules.h&quot;<br>
#include&#32;&lt;filesystem&gt;<br>
#include&#32;&lt;string&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
struct&#32;Config<br>
{<br>
&#32;&#32;&#32;&#32;std::vector&lt;Rule&gt;&#32;text_rules;<br>
&#32;&#32;&#32;&#32;std::vector&lt;Rule&gt;&#32;tree_rules;<br>
&#32;&#32;&#32;&#32;std::string&#32;map_format;&#32;//&#32;новый&#32;блок&#32;[MAPFORMAT],&#32;может&#32;быть&#32;пустым<br>
};<br>
<br>
Config&#32;parse_config(const&#32;std::filesystem::path&#32;&amp;path);<br>
<!-- END SCAT CODE -->
</body>
</html>

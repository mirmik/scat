<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&quot;rules.h&quot;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
struct &#20;Config<br>
{<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;text_rules;<br>
 &#20; &#20; &#20; &#20;std::vector&lt;Rule&gt; &#20;tree_rules;<br>
 &#20; &#20; &#20; &#20;std::string &#20;map_format; &#20;// &#20;новый &#20;блок &#20;[MAPFORMAT], &#20;может &#20;быть &#20;пустым<br>
};<br>
<br>
Config &#20;parse_config(const &#20;std::filesystem::path &#20;&amp;path);<br>
<!-- END SCAT CODE -->
</body>
</html>

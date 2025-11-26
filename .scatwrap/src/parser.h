<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&quot;rules.h&quot;<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;Config<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;Rule&gt;&nbsp;text_rules;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;Rule&gt;&nbsp;tree_rules;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;map_format;&nbsp;//&nbsp;новый&nbsp;блок&nbsp;[MAPFORMAT],&nbsp;может&nbsp;быть&nbsp;пустым<br>
};<br>
<br>
Config&nbsp;parse_config(const&nbsp;std::filesystem::path&nbsp;&amp;path);<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&quot;options.h&quot;<br>
#include&#32;&quot;rules.h&quot;<br>
#include&#32;&lt;filesystem&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_rules(const&#32;std::vector&lt;Rule&gt;&#32;&amp;rules,&#32;const&#32;Options&#32;&amp;opt);<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_paths(const&#32;std::vector&lt;std::string&gt;&#32;&amp;paths,&#32;const&#32;Options&#32;&amp;opt);<br>
<!-- END SCAT CODE -->
</body>
</html>

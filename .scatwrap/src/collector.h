<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&quot;options.h&quot;<br>
#include&nbsp;&quot;rules.h&quot;<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_rules(const&nbsp;std::vector&lt;Rule&gt;&nbsp;&amp;rules,&nbsp;const&nbsp;Options&nbsp;&amp;opt);<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_paths(const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;paths,&nbsp;const&nbsp;Options&nbsp;&amp;opt);<br>
<!-- END SCAT CODE -->
</body>
</html>

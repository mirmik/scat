<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&quot;options.h&quot;<br>
#include &#20;&quot;rules.h&quot;<br>
#include &#20;&lt;filesystem&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_rules(const &#20;std::vector&lt;Rule&gt; &#20;&amp;rules, &#20;const &#20;Options &#20;&amp;opt);<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_paths(const &#20;std::vector&lt;std::string&gt; &#20;&amp;paths, &#20;const &#20;Options &#20;&amp;opt);<br>
<!-- END SCAT CODE -->
</body>
</html>

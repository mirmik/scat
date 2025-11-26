<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &quot;options.h&quot;<br>
#include &quot;rules.h&quot;<br>
#include &lt;filesystem&gt;<br>
#include &lt;vector&gt;<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules, const Options &amp;opt);<br>
<br>
std::vector&lt;std::filesystem::path&gt;<br>
collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths, const Options &amp;opt);<br>
<!-- END SCAT CODE -->
</body>
</html>

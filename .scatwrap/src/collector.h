<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &quot;options.h&quot;
#include &quot;rules.h&quot;
#include &lt;filesystem&gt;
#include &lt;vector&gt;

std::vector&lt;std::filesystem::path&gt;
collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules, const Options &amp;opt);

std::vector&lt;std::filesystem::path&gt;
collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths, const Options &amp;opt);

</code></pre>
</body>
</html>

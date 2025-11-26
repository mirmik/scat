<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/scat.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &lt;vector&gt;
#include &lt;filesystem&gt;
#include &quot;chunk_help.h&quot;

int scat_main(int argc, char** argv);
void print_tree(const std::vector&lt;std::filesystem::path&gt;&amp; files);

</code></pre>
</body>
</html>

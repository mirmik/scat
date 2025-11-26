<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/rules.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &lt;string&gt;

struct Rule
{
    std::string pattern;
    bool exclude = false;

    static Rule from_string(const std::string &amp;line)
    {
        Rule r;
        if (!line.empty() &amp;&amp; line[0] == '!')
        {
            r.exclude = true;
            r.pattern = line.substr(1);
        }
        else
        {
            r.exclude = false;
            r.pattern = line;
        }
        return r;
    }
};
</code></pre>
</body>
</html>

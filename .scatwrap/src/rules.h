<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/rules.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &lt;string&gt;<br>
<br>
struct Rule<br>
{<br>
    std::string pattern;<br>
    bool exclude = false;<br>
<br>
    static Rule from_string(const std::string &amp;line)<br>
    {<br>
        Rule r;<br>
        if (!line.empty() &amp;&amp; line[0] == '!')<br>
        {<br>
            r.exclude = true;<br>
            r.pattern = line.substr(1);<br>
        }<br>
        else<br>
        {<br>
            r.exclude = false;<br>
            r.pattern = line;<br>
        }<br>
        return r;<br>
    }<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

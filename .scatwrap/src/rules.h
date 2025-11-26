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
&#9;std::string pattern;<br>
&#9;bool exclude = false;<br>
<br>
&#9;static Rule from_string(const std::string &amp;line)<br>
&#9;{<br>
&#9;&#9;Rule r;<br>
&#9;&#9;if (!line.empty() &amp;&amp; line[0] == '!')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;r.exclude = true;<br>
&#9;&#9;&#9;r.pattern = line.substr(1);<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;r.exclude = false;<br>
&#9;&#9;&#9;r.pattern = line;<br>
&#9;&#9;}<br>
&#9;&#9;return r;<br>
&#9;}<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

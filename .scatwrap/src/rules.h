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
&emsp;std::string pattern;<br>
&emsp;bool exclude = false;<br>
<br>
&emsp;static Rule from_string(const std::string &amp;line)<br>
&emsp;{<br>
&emsp;&emsp;Rule r;<br>
&emsp;&emsp;if (!line.empty() &amp;&amp; line[0] == '!')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;r.exclude = true;<br>
&emsp;&emsp;&emsp;r.pattern = line.substr(1);<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;r.exclude = false;<br>
&emsp;&emsp;&emsp;r.pattern = line;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;return r;<br>
&emsp;}<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

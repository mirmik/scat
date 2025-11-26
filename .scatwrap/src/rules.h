<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/rules.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&lt;string&gt;<br>
<br>
struct&nbsp;Rule<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;pattern;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;exclude&nbsp;=&nbsp;false;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;Rule&nbsp;from_string(const&nbsp;std::string&nbsp;&amp;line)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Rule&nbsp;r;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(!line.empty()&nbsp;&amp;&amp;&nbsp;line[0]&nbsp;==&nbsp;'!')<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;r.exclude&nbsp;=&nbsp;true;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;r.pattern&nbsp;=&nbsp;line.substr(1);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;else<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;r.exclude&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;r.pattern&nbsp;=&nbsp;line;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;r;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &lt;iostream&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;string&gt;<br>
<br>
void copy_to_clipboard(const std::string &amp;text);<br>
<br>
class CopyGuard<br>
{<br>
public:<br>
&emsp;explicit CopyGuard(bool enabled);<br>
&emsp;~CopyGuard();<br>
<br>
private:<br>
&emsp;bool enabled_;<br>
&emsp;std::ostringstream buffer_;<br>
&emsp;std::streambuf *old_buf_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

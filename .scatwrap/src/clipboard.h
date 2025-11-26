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
&#9;explicit CopyGuard(bool enabled);<br>
&#9;~CopyGuard();<br>
<br>
private:<br>
&#9;bool enabled_;<br>
&#9;std::ostringstream buffer_;<br>
&#9;std::streambuf *old_buf_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

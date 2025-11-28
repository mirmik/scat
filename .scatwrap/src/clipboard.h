<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&lt;iostream&gt;<br>
#include&nbsp;&lt;sstream&gt;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
void&nbsp;copy_to_clipboard(const&nbsp;std::string&nbsp;&amp;text,&nbsp;bool&nbsp;verbose&nbsp;=&nbsp;false);<br>
<br>
class&nbsp;CopyGuard<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;explicit&nbsp;CopyGuard(bool&nbsp;enabled,&nbsp;bool&nbsp;verbose&nbsp;=&nbsp;false);<br>
&nbsp;&nbsp;&nbsp;&nbsp;~CopyGuard();<br>
<br>
private:<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;enabled_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;verbose_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::ostringstream&nbsp;buffer_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::streambuf&nbsp;*old_buf_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

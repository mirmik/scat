<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&lt;iostream&gt;<br>
#include&#32;&lt;sstream&gt;<br>
#include&#32;&lt;string&gt;<br>
<br>
void&#32;copy_to_clipboard(const&#32;std::string&#32;&amp;text);<br>
<br>
class&#32;CopyGuard<br>
{<br>
public:<br>
&#32;&#32;&#32;&#32;explicit&#32;CopyGuard(bool&#32;enabled);<br>
&#32;&#32;&#32;&#32;~CopyGuard();<br>
<br>
private:<br>
&#32;&#32;&#32;&#32;bool&#32;enabled_;<br>
&#32;&#32;&#32;&#32;std::ostringstream&#32;buffer_;<br>
&#32;&#32;&#32;&#32;std::streambuf&#32;*old_buf_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

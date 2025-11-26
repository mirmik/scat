<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&lt;iostream&gt;<br>
#include &#20;&lt;sstream&gt;<br>
#include &#20;&lt;string&gt;<br>
<br>
void &#20;copy_to_clipboard(const &#20;std::string &#20;&amp;text);<br>
<br>
class &#20;CopyGuard<br>
{<br>
public:<br>
 &#20; &#20; &#20; &#20;explicit &#20;CopyGuard(bool &#20;enabled);<br>
 &#20; &#20; &#20; &#20;~CopyGuard();<br>
<br>
private:<br>
 &#20; &#20; &#20; &#20;bool &#20;enabled_;<br>
 &#20; &#20; &#20; &#20;std::ostringstream &#20;buffer_;<br>
 &#20; &#20; &#20; &#20;std::streambuf &#20;*old_buf_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>

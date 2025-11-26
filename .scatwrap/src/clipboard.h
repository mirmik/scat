<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<pre><code>
#pragma once
#include &lt;iostream&gt;
#include &lt;sstream&gt;
#include &lt;string&gt;

void copy_to_clipboard(const std::string &amp;text);

class CopyGuard
{
public:
    explicit CopyGuard(bool enabled);
    ~CopyGuard();

private:
    bool enabled_;
    std::ostringstream buffer_;
    std::streambuf *old_buf_;
};

</code></pre>
</body>
</html>

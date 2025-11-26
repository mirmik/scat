<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;clipboard.h&quot;<br>
#include &lt;cstdio&gt;<br>
#include &lt;iostream&gt;<br>
<br>
// Копирование текста в системный буфер обмена.<br>
// Платформы:<br>
//   Linux/Unix: wl-copy, xclip, xsel (что найдётся и успешно отработает)<br>
//   macOS: pbcopy<br>
//   Windows: clip<br>
void copy_to_clipboard(const std::string &amp;text)<br>
{<br>
&#9;if (text.empty())<br>
&#9;&#9;return;<br>
<br>
#if defined(_WIN32)<br>
&#9;FILE *pipe = _popen(&quot;clip&quot;, &quot;w&quot;);<br>
&#9;if (!pipe)<br>
&#9;&#9;return;<br>
&#9;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&#9;_pclose(pipe);<br>
#elif defined(__APPLE__)<br>
&#9;FILE *pipe = popen(&quot;pbcopy&quot;, &quot;w&quot;);<br>
&#9;if (!pipe)<br>
&#9;&#9;return;<br>
&#9;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&#9;pclose(pipe);<br>
#else<br>
&#9;// POSIX: пытаемся по очереди несколько утилит.<br>
&#9;// stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.<br>
&#9;const char *commands[] = {<br>
&#9;&#9;&quot;wl-copy 2&gt;/dev/null&quot;,<br>
&#9;&#9;&quot;xclip -selection clipboard 2&gt;/dev/null&quot;,<br>
&#9;&#9;&quot;xsel --clipboard --input 2&gt;/dev/null&quot;,<br>
&#9;};<br>
<br>
&#9;for (const char *cmd : commands)<br>
&#9;{<br>
&#9;&#9;FILE *pipe = popen(cmd, &quot;w&quot;);<br>
&#9;&#9;if (!pipe)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&#9;&#9;int rc = pclose(pipe);<br>
&#9;&#9;if (rc == 0)<br>
&#9;&#9;&#9;break; // какая-то из утилит успешно отработала<br>
&#9;}<br>
#endif<br>
}<br>
<br>
CopyGuard::CopyGuard(bool enabled) : enabled_(enabled), old_buf_(nullptr)<br>
{<br>
&#9;if (enabled_)<br>
&#9;{<br>
&#9;&#9;old_buf_ = std::cout.rdbuf(buffer_.rdbuf());<br>
&#9;}<br>
}<br>
<br>
CopyGuard::~CopyGuard()<br>
{<br>
&#9;if (!enabled_)<br>
&#9;&#9;return;<br>
<br>
&#9;// вернуть настоящий буфер std::cout<br>
&#9;std::cout.rdbuf(old_buf_);<br>
<br>
&#9;const std::string out = buffer_.str();<br>
&#9;if (!out.empty())<br>
&#9;{<br>
&#9;&#9;// НИЧЕГО не печатаем в консоль!<br>
&#9;&#9;// Просто отправляем весь текст в буфер обмена.<br>
&#9;&#9;copy_to_clipboard(out);<br>
&#9;}<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

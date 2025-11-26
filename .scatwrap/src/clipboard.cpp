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
&emsp;if (text.empty())<br>
&emsp;&emsp;return;<br>
<br>
#if defined(_WIN32)<br>
&emsp;FILE *pipe = _popen(&quot;clip&quot;, &quot;w&quot;);<br>
&emsp;if (!pipe)<br>
&emsp;&emsp;return;<br>
&emsp;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&emsp;_pclose(pipe);<br>
#elif defined(__APPLE__)<br>
&emsp;FILE *pipe = popen(&quot;pbcopy&quot;, &quot;w&quot;);<br>
&emsp;if (!pipe)<br>
&emsp;&emsp;return;<br>
&emsp;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&emsp;pclose(pipe);<br>
#else<br>
&emsp;// POSIX: пытаемся по очереди несколько утилит.<br>
&emsp;// stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.<br>
&emsp;const char *commands[] = {<br>
&emsp;&emsp;&quot;wl-copy 2&gt;/dev/null&quot;,<br>
&emsp;&emsp;&quot;xclip -selection clipboard 2&gt;/dev/null&quot;,<br>
&emsp;&emsp;&quot;xsel --clipboard --input 2&gt;/dev/null&quot;,<br>
&emsp;};<br>
<br>
&emsp;for (const char *cmd : commands)<br>
&emsp;{<br>
&emsp;&emsp;FILE *pipe = popen(cmd, &quot;w&quot;);<br>
&emsp;&emsp;if (!pipe)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;std::fwrite(text.data(), 1, text.size(), pipe);<br>
&emsp;&emsp;int rc = pclose(pipe);<br>
&emsp;&emsp;if (rc == 0)<br>
&emsp;&emsp;&emsp;break; // какая-то из утилит успешно отработала<br>
&emsp;}<br>
#endif<br>
}<br>
<br>
CopyGuard::CopyGuard(bool enabled) : enabled_(enabled), old_buf_(nullptr)<br>
{<br>
&emsp;if (enabled_)<br>
&emsp;{<br>
&emsp;&emsp;old_buf_ = std::cout.rdbuf(buffer_.rdbuf());<br>
&emsp;}<br>
}<br>
<br>
CopyGuard::~CopyGuard()<br>
{<br>
&emsp;if (!enabled_)<br>
&emsp;&emsp;return;<br>
<br>
&emsp;// вернуть настоящий буфер std::cout<br>
&emsp;std::cout.rdbuf(old_buf_);<br>
<br>
&emsp;const std::string out = buffer_.str();<br>
&emsp;if (!out.empty())<br>
&emsp;{<br>
&emsp;&emsp;// НИЧЕГО не печатаем в консоль!<br>
&emsp;&emsp;// Просто отправляем весь текст в буфер обмена.<br>
&emsp;&emsp;copy_to_clipboard(out);<br>
&emsp;}<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

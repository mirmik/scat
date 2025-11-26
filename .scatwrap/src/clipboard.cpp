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
    if (text.empty())<br>
        return;<br>
<br>
#if defined(_WIN32)<br>
    FILE *pipe = _popen(&quot;clip&quot;, &quot;w&quot;);<br>
    if (!pipe)<br>
        return;<br>
    std::fwrite(text.data(), 1, text.size(), pipe);<br>
    _pclose(pipe);<br>
#elif defined(__APPLE__)<br>
    FILE *pipe = popen(&quot;pbcopy&quot;, &quot;w&quot;);<br>
    if (!pipe)<br>
        return;<br>
    std::fwrite(text.data(), 1, text.size(), pipe);<br>
    pclose(pipe);<br>
#else<br>
    // POSIX: пытаемся по очереди несколько утилит.<br>
    // stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.<br>
    const char *commands[] = {<br>
        &quot;wl-copy 2&gt;/dev/null&quot;,<br>
        &quot;xclip -selection clipboard 2&gt;/dev/null&quot;,<br>
        &quot;xsel --clipboard --input 2&gt;/dev/null&quot;,<br>
    };<br>
<br>
    for (const char *cmd : commands)<br>
    {<br>
        FILE *pipe = popen(cmd, &quot;w&quot;);<br>
        if (!pipe)<br>
            continue;<br>
<br>
        std::fwrite(text.data(), 1, text.size(), pipe);<br>
        int rc = pclose(pipe);<br>
        if (rc == 0)<br>
            break; // какая-то из утилит успешно отработала<br>
    }<br>
#endif<br>
}<br>
<br>
CopyGuard::CopyGuard(bool enabled) : enabled_(enabled), old_buf_(nullptr)<br>
{<br>
    if (enabled_)<br>
    {<br>
        old_buf_ = std::cout.rdbuf(buffer_.rdbuf());<br>
    }<br>
}<br>
<br>
CopyGuard::~CopyGuard()<br>
{<br>
    if (!enabled_)<br>
        return;<br>
<br>
    // вернуть настоящий буфер std::cout<br>
    std::cout.rdbuf(old_buf_);<br>
<br>
    const std::string out = buffer_.str();<br>
    if (!out.empty())<br>
    {<br>
        // НИЧЕГО не печатаем в консоль!<br>
        // Просто отправляем весь текст в буфер обмена.<br>
        copy_to_clipboard(out);<br>
    }<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

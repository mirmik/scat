<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.cpp</title>
</head>
<body>
<pre><code>
#include &quot;clipboard.h&quot;
#include &lt;cstdio&gt;
#include &lt;iostream&gt;

// Копирование текста в системный буфер обмена.
// Платформы:
//   Linux/Unix: wl-copy, xclip, xsel (что найдётся и успешно отработает)
//   macOS: pbcopy
//   Windows: clip
void copy_to_clipboard(const std::string&amp; text)
{
    if (text.empty())
        return;

#if defined(_WIN32)
    FILE* pipe = _popen(&quot;clip&quot;, &quot;w&quot;);
    if (!pipe)
        return;
    std::fwrite(text.data(), 1, text.size(), pipe);
    _pclose(pipe);
#elif defined(__APPLE__)
    FILE* pipe = popen(&quot;pbcopy&quot;, &quot;w&quot;);
    if (!pipe)
        return;
    std::fwrite(text.data(), 1, text.size(), pipe);
    pclose(pipe);
#else
    // POSIX: пытаемся по очереди несколько утилит.
    // stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.
    const char* commands[] = {
        &quot;wl-copy 2&gt;/dev/null&quot;,
        &quot;xclip -selection clipboard 2&gt;/dev/null&quot;,
        &quot;xsel --clipboard --input 2&gt;/dev/null&quot;,
    };

    for (const char* cmd : commands)
    {
        FILE* pipe = popen(cmd, &quot;w&quot;);
        if (!pipe)
            continue;

        std::fwrite(text.data(), 1, text.size(), pipe);
        int rc = pclose(pipe);
        if (rc == 0)
            break; // какая-то из утилит успешно отработала
    }
#endif
}


     CopyGuard::CopyGuard(bool enabled)
        : enabled_(enabled), old_buf_(nullptr)
    {
        if (enabled_) {
            old_buf_ = std::cout.rdbuf(buffer_.rdbuf());
        }
    }

    CopyGuard::~CopyGuard()
    {
        if (!enabled_)
            return;

        // вернуть настоящий буфер std::cout
        std::cout.rdbuf(old_buf_);

        const std::string out = buffer_.str();
        if (!out.empty()) {
            // НИЧЕГО не печатаем в консоль!
            // Просто отправляем весь текст в буфер обмена.
            copy_to_clipboard(out);
        }
    }

</code></pre>
</body>
</html>

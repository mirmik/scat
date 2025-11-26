#include "clipboard.h"
#include <cstdio>
#include <iostream>

// Копирование текста в системный буфер обмена.
// Платформы:
//   Linux/Unix: wl-copy, xclip, xsel (что найдётся и успешно отработает)
//   macOS: pbcopy
//   Windows: clip
void copy_to_clipboard(const std::string& text)
{
    if (text.empty())
        return;

#if defined(_WIN32)
    FILE* pipe = _popen("clip", "w");
    if (!pipe)
        return;
    std::fwrite(text.data(), 1, text.size(), pipe);
    _pclose(pipe);
#elif defined(__APPLE__)
    FILE* pipe = popen("pbcopy", "w");
    if (!pipe)
        return;
    std::fwrite(text.data(), 1, text.size(), pipe);
    pclose(pipe);
#else
    // POSIX: пытаемся по очереди несколько утилит.
    // stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.
    const char* commands[] = {
        "wl-copy 2>/dev/null",
        "xclip -selection clipboard 2>/dev/null",
        "xsel --clipboard --input 2>/dev/null",
    };

    for (const char* cmd : commands)
    {
        FILE* pipe = popen(cmd, "w");
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

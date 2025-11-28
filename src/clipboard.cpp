#include "clipboard.h"
#include <cstdio>
#include <iostream>

// Копирование текста в системный буфер обмена.
// Платформы:
//   Linux/Unix: wl-copy, xclip, xsel (что найдётся и успешно отработает)
//   macOS: pbcopy
//   Windows: clip
// Копирование текста в системный буфер обмена.
// Платформы:
//   Linux/Unix: wl-copy, xclip, xsel (что найдётся и успешно отработает)
//   macOS: pbcopy
//   Windows: clip
void copy_to_clipboard(const std::string &text, bool verbose)
{
    if (text.empty())
    {
        if (verbose)
            std::cerr << "scat: copy: nothing to copy (0 bytes), skipping\n";
        return;
    }

#if defined(_WIN32)
    if (verbose)
        std::cerr << "scat: copy: using 'clip' (" << text.size()
                  << " bytes)\n";

    FILE *pipe = _popen("clip", "w");
    if (!pipe)
    {
        if (verbose)
            std::cerr << "scat: copy: failed to start 'clip'\n";
        return;
    }

    std::size_t written = std::fwrite(text.data(), 1, text.size(), pipe);
    if (verbose && written != text.size())
    {
        std::cerr << "scat: copy: wrote " << written << " bytes out of "
                  << text.size() << "\n";
    }

    int rc = _pclose(pipe);
    if (verbose)
    {
        if (rc == 0)
            std::cerr << "scat: copy: 'clip' exited with code 0\n";
        else
            std::cerr << "scat: copy: 'clip' exited with code " << rc
                      << "\n";
    }
#elif defined(__APPLE__)
    if (verbose)
        std::cerr << "scat: copy: using 'pbcopy' (" << text.size()
                  << " bytes)\n";

    FILE *pipe = popen("pbcopy", "w");
    if (!pipe)
    {
        if (verbose)
            std::cerr << "scat: copy: failed to start 'pbcopy'\n";
        return;
    }

    std::size_t written = std::fwrite(text.data(), 1, text.size(), pipe);
    if (verbose && written != text.size())
    {
        std::cerr << "scat: copy: wrote " << written << " bytes out of "
                  << text.size() << "\n";
    }

    int rc = pclose(pipe);
    if (verbose)
    {
        if (rc == 0)
            std::cerr << "scat: copy: 'pbcopy' exited with code 0\n";
        else
            std::cerr << "scat: copy: 'pbcopy' exited with code " << rc
                      << "\n";
    }
#else
    // POSIX: пытаемся по очереди несколько утилит.
    // stderr каждой уводим в /dev/null, чтобы они не засоряли терминал.
    const char *commands[] = {
        "wl-copy 2>/dev/null",
        "xclip -selection clipboard 2>/dev/null",
        "xsel --clipboard --input 2>/dev/null",
    };
    const char *names[] = {"wl-copy", "xclip", "xsel"};

    bool success = false;
    const std::size_t total = text.size();

    for (std::size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i)
    {
        const char *cmd = commands[i];
        const char *name = names[i];

        if (verbose)
            std::cerr << "scat: copy: trying '" << name << "' (" << total
                      << " bytes)\n";

        FILE *pipe = popen(cmd, "w");
        if (!pipe)
        {
            if (verbose)
                std::cerr << "scat: copy: failed to start '" << name
                          << "'\n";
            continue;
        }

        std::size_t written = std::fwrite(text.data(), 1, total, pipe);
        if (verbose && written != total)
        {
            std::cerr << "scat: copy: '" << name << "' wrote " << written
                      << " bytes out of " << total << "\n";
        }

        int rc = pclose(pipe);

        if (verbose)
        {
            if (rc == 0)
                std::cerr << "scat: copy: '" << name
                          << "' exited with code 0\n";
            else
                std::cerr << "scat: copy: '" << name
                          << "' exited with code " << rc << "\n";
        }

        if (rc == 0)
        {
            success = true;
            break; // какая-то из утилит успешно отработала
        }
    }

    if (verbose && !success)
    {
        std::cerr << "scat: copy: all clipboard commands failed; clipboard "
                     "not updated\n";
    }
#endif
}

CopyGuard::CopyGuard(bool enabled, bool verbose)
    : enabled_(enabled)
    , verbose_(verbose)
    , old_buf_(nullptr)
{
    if (enabled_)
    {
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
    if (!out.empty())
    {
        // НИЧЕГО не печатаем в консоль!
        // Просто отправляем весь текст в буфер обмена.
        copy_to_clipboard(out, verbose_);
    }
    else if (verbose_)
    {
        std::cerr << "scat: copy: buffer is empty, nothing to copy\n";
    }
}

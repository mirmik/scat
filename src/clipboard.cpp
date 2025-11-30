      #include "clipboard.h"
      #include <cstdio>
      #include "clipboard.h"
      #include <cstdio>
      #include <cstdlib>
      #include <iostream>
      #include <string>
      #ifndef _WIN32
      #    include <signal.h>
      #endif

      // Копирование текста в системный буфер обмена.
      
      // Вся логика запуска утилит — один проход по таблице kClipboardTools.
      struct ClipboardTool
      {
          const char *name;          // Человеческое имя утилиты (для логов)
          const char *command;       // Команда, которую отдаём popen/_popen
          bool (*is_available)();    // Опциональная проверка среды, nullptr если не нужна
      };

      // Платформы и порядок утилит:
      #if defined(_WIN32)

      static const ClipboardTool kClipboardTools[] = {
          {"clip", "clip", nullptr},
      };

      #elif defined(__APPLE__)

      static const ClipboardTool kClipboardTools[] = {
          {"pbcopy", "pbcopy", nullptr},
      };

      #else

      // На POSIX-платформах сразу зашиваем подавление stderr в команду.
      static bool has_wayland_display()
      {
          const char *env = std::getenv("WAYLAND_DISPLAY");
          return env && *env;
      }

      static const ClipboardTool kClipboardTools[] = {
          {"termux-clipboard-set", "termux-clipboard-set 2>/dev/null", nullptr},
          {"wl-copy", "wl-copy 2>/dev/null", &has_wayland_display},
          {"xclip",   "xclip -selection clipboard 2>/dev/null", nullptr},
          {"xsel",    "xsel --clipboard --input 2>/dev/null", nullptr},
      };

      #endif

      // Запускает одну утилиту из списка, пишет в её stdin и ждёт код возврата.
      // Возвращает true, если команда успешно завершилась (код 0).
      static bool run_clipboard_tool(const ClipboardTool &tool,
                                     const std::string &text,
                                     bool verbose,
                                     const char *verb)
      {
          if (tool.is_available && !tool.is_available())
          {
              if (verbose)
              {
                  std::cerr << "scat: copy: skipping '" << tool.name
                            << "' (not available in current environment)\n";
              }
              return false;
          }

          if (verbose)
          {
              std::cerr << "scat: copy: " << verb << " '" << tool.name << "' ("
                        << text.size() << " bytes)\n";
          }

      #if defined(_WIN32)
          FILE *pipe = _popen(tool.command, "w");
      #else
          FILE *pipe = popen(tool.command, "w");
      #endif

          if (!pipe)
          {
              if (verbose)
              {
                  std::cerr << "scat: copy: failed to start '" << tool.name
                            << "'\n";
              }
              return false;
          }

      #ifndef _WIN32
          struct sigaction sa_new{};
          struct sigaction sa_old{};
          sa_new.sa_handler = SIG_IGN;
          sigemptyset(&sa_new.sa_mask);
          sa_new.sa_flags = 0;
          bool sigpipe_guard_installed =
              (sigaction(SIGPIPE, &sa_new, &sa_old) == 0);
      #endif

          const std::size_t total = text.size();
          std::size_t written =
              std::fwrite(text.data(), 1, total, pipe);

          if (verbose && written != total)
          {
              std::cerr << "scat: copy: '" << tool.name << "' wrote "
                        << written << " bytes out of " << total << "\n";
          }

      #ifndef _WIN32
          if (sigpipe_guard_installed)
              sigaction(SIGPIPE, &sa_old, nullptr);
      #endif

      #if defined(_WIN32)
          int rc = _pclose(pipe);
      #else
          int rc = pclose(pipe);
      #endif

          if (verbose)
          {
              if (rc == 0)
              {
                  std::cerr << "scat: copy: '" << tool.name
                            << "' exited with code 0\n";
              }
              else
              {
                  std::cerr << "scat: copy: '" << tool.name
                            << "' exited with code " << rc << "\n";
              }
          }

          return rc == 0;
      }

      void copy_to_clipboard(const std::string &text, bool verbose)
      {
          if (text.empty())
          {
              if (verbose)
                  std::cerr << "scat: copy: nothing to copy (0 bytes), skipping\n";
              return;
          }

          const std::size_t tool_count =
              sizeof(kClipboardTools) / sizeof(kClipboardTools[0]);

          if (tool_count == 0)
          {
              if (verbose)
              {
                  std::cerr << "scat: copy: no clipboard tools configured for "
                               "this platform\n";
              }
              return;
          }

          const char *single_verb = (tool_count == 1) ? "using" : "trying";

          bool success = false;
          for (std::size_t i = 0; i < tool_count; ++i)
          {
              const ClipboardTool &tool = kClipboardTools[i];

              if (run_clipboard_tool(tool, text, verbose, single_verb))
              {
                  success = true;
                  break; // одна из утилит успешно отработала
              }
          }

          if (verbose && !success)
          {
              std::cerr << "scat: copy: all clipboard commands failed; clipboard "
                           "not updated\n";
          }
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

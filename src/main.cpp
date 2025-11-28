#include "scat.h"
#ifdef _WIN32
#    include <windows.h>
#endif

int main(int argc, char **argv)
{
#ifdef _WIN32
    // Переключаем кодовые страницы консоли на UTF-8, чтобы UTF-8 текст
    // (включая кириллицу) нормально печатался в cmd/PowerShell.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    return scat_main(argc, argv);
}

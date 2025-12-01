#include "guard/guard.h"
#include "options.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

// объявляем внешнюю функцию из scat.cpp
int wrap_files_to_html(const std::vector<fs::path> &files, const Options &opt);

static void write_file(const fs::path &p, const std::string &text)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p);
    out << text;
}

TEST_CASE("wrap: removes stale files in wrap_root")
{
    // отдельный tmp-каталог под тест
    fs::path tmp = fs::temp_directory_path() / "wrap_cleanup_test_1";
    fs::remove_all(tmp);

    fs::path srcdir = tmp / "src";
    fs::create_directories(srcdir);

    // живой исходный файл, который будем врапить
    fs::path alive1 = srcdir / "alive1.cpp";
    write_file(alive1, "int A = 1;");

    // директория для врапов
    fs::path wrap_root = tmp / "wrap";
    fs::create_directories(wrap_root);

    // "мёртвый" врап, который должен быть удалён
    fs::path dead_wrap = wrap_root / "dead_file.cpp";
    write_file(dead_wrap, "<html>DEAD</html>");

    Options opt;
    opt.wrap_root = wrap_root.string();

    std::vector<fs::path> files = {alive1};

    // вызываем реальный wrap-процесс
    int rc = wrap_files_to_html(files, opt);
    CHECK(rc == 0);

    // проверяем, что для живого файла действительно появился врап
    {
        std::error_code ec;
        fs::path rel = fs::relative(alive1, fs::current_path(), ec);
        if (ec)
        {
            // тот же fallback, что и в wrap_files_to_html
            rel = alive1.filename();
        }
        fs::path expected_wrap = wrap_root / rel;
        CHECK(fs::exists(expected_wrap));
    }

    // и что "мёртвый" врап исчез
    CHECK(!fs::exists(dead_wrap));
}

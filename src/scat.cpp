#include "scat.h"
#include "clipboard.h"
#include "collector.h"
#include "git_info.h"
#include "options.h"
#include "parser.h"
#include "rules.h"
#include "util.h"
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif


namespace fs = std::filesystem;

bool g_use_absolute_paths = false;

void print_tree(const std::vector<std::filesystem::path> &files)
{
    // Собираем относительные пути
    std::vector<std::string> rels;
    rels.reserve(files.size());
    for (auto &p : files)
        rels.push_back(make_display_path(p));

    std::sort(rels.begin(), rels.end());

    struct Node
    {
        std::map<std::string, Node *> children;
        bool is_file = false;
    };

    Node root;

    // ----------------------------
    //  Построение дерева
    // ----------------------------
    for (auto &r : rels)
    {
        fs::path p = r;
        Node *cur = &root;

        // вытаскиваем компоненты p в список
        std::vector<std::string> parts;
        for (auto &part : p)
            parts.push_back(part.string());

        int total = (int)parts.size();

        for (int i = 0; i < total; ++i)
        {
            const std::string &name = parts[i];
            bool last = (i == total - 1);

            if (!cur->children.count(name))
                cur->children[name] = new Node();

            cur = cur->children[name];
            if (last)
                cur->is_file = true;
        }
    }

    // ----------------------------
    //  Рекурсивная печать
    // ----------------------------
    std::function<void(Node *, const std::string &, bool, const std::string &)>
        go;

    go = [&](Node *node,
             const std::string &name,
             bool last,
             const std::string &prefix)
    {
        if (!name.empty())
        {
            std::cout << prefix << (last ? "└── " : "├── ") << name;

            if (!node->is_file)
                std::cout << "/";

            std::cout << "\n";
        }

        // сортируем детей по имени
        std::vector<std::string> keys;
        keys.reserve(node->children.size());
        for (auto &[k, _] : node->children)
            keys.push_back(k);
        std::sort(keys.begin(), keys.end());

        for (size_t i = 0; i < keys.size(); ++i)
        {
            bool is_last = (i + 1 == keys.size());
            Node *child = node->children[keys[i]];

            go(child,
               keys[i],
               is_last,
               prefix + (name.empty() ? "" : (last ? "    " : "│   ")));
        }
    };

    std::cout << "===== PROJECT TREE =====\n";
    go(&root, "", true, "");
    std::cout << "========================\n\n";
}

static void
print_list_with_sizes_to(const std::vector<std::filesystem::path> &files,
                         const Options &opt,
                         std::ostream &os)
{
    namespace fs = std::filesystem;

    struct Item
    {
        fs::path path;
        std::string display;
        std::uintmax_t size;
    };

    std::vector<Item> items;
    items.reserve(files.size());
    std::uintmax_t total = 0;
    std::size_t max_len = 0;

    for (auto &f : files)
    {
        auto disp = make_display_path(f);
        auto sz = get_file_size(f);

        total += sz;

        std::size_t shown_len = opt.path_prefix.size() + disp.size();
        if (shown_len > max_len)
            max_len = shown_len;

        items.push_back(Item{f, std::move(disp), sz});
    }

    if (opt.sorted)
    {
        std::sort(items.begin(),
                  items.end(),
                  [](const Item &a, const Item &b)
                  {
                      if (a.size != b.size)
                          return a.size > b.size; // по убыванию
                      return a.display < b.display;
                  });
    }

    const bool show_size = opt.show_size;
    for (const auto &it : items)
    {
        std::string shown = opt.path_prefix + it.display;
        os << shown;

        if (show_size)
        {
            if (max_len > shown.size())
            {
                std::size_t pad = max_len - shown.size();
                for (std::size_t i = 0; i < pad; ++i)
                    os << ' ';
            }
            os << " (" << it.size << " bytes)";
        }

        os << "\n";
    }

    if (show_size)
    {
        os << "Total size: " << total << " bytes\n";
    }
}

// старая функция теперь просто обёртка вокруг новой
static void
print_list_with_sizes(const std::vector<std::filesystem::path> &files,
                      const Options &opt)
{
    print_list_with_sizes_to(files, opt, std::cout);
}

// Вывод всех файлов и подсчёт суммарного размера
static std::uintmax_t
dump_files_and_total(const std::vector<std::filesystem::path> &files,
                     const Options &opt)
{
    std::uintmax_t total = 0;
    bool first = true;

    for (auto &f : files)
    {
        dump_file(f, first, opt);
        first = false;
        total += get_file_size(f);
    }

    return total;
}

int run_server(const Options &opt);

// общий каркас обработки списка файлов
static int process_files(const std::vector<std::filesystem::path> &files,
                         const Options &opt,
                         const std::function<void(std::uintmax_t)> &after_dump)
{
    if (files.empty())
    {
        std::cerr << "No files collected.\n";
        return 0;
    }

    if (!opt.wrap_root.empty())
        return wrap_files_to_html(files, opt);

    if (opt.list_only)
    {
        print_list_with_sizes(files, opt);
        return 0;
    }

    std::uintmax_t total = dump_files_and_total(files, opt);

    if (after_dump)
        after_dump(total);

    return 0;
}

// =========================
// CONFIG MODE
// =========================

static int run_config_mode(const Options &opt)
{
    Config cfg;
    try
    {
        cfg = parse_config(opt.config_file);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    auto text_files = collect_from_rules(cfg.text_rules, opt);

    auto after_dump = [&](std::uintmax_t total)
    {
        // TREE rules (если есть)
        if (!cfg.tree_rules.empty())
        {
            auto tree_files = collect_from_rules(cfg.tree_rules, opt);
            if (!tree_files.empty())
            {
                std::cout << "\n";
                print_tree(tree_files);
            }
        }
        std::cout << "\nTotal size: " << total << " bytes\n";
    };

    return process_files(text_files, opt, after_dump);
}

// =========================
// NORMAL MODE
// =========================

static int run_normal_mode(const Options &opt)
{
    std::vector<std::filesystem::path> files =
        collect_from_paths(opt.paths, opt);

    auto after_dump = [&](std::uintmax_t total)
    {
        // В обычном режиме дерево не печатаем
        std::cout << "\nTotal size: " << total << " bytes\n";
    };

    return process_files(files, opt, after_dump);
}

static std::string substitute_rawmap(const std::string &tmpl,
                                     const std::string &rawmap)
{
    const std::string token = "{RAWMAP}";
    if (tmpl.empty())
        return rawmap;

    std::string out;
    out.reserve(tmpl.size() + rawmap.size() + 16);

    std::size_t pos = 0;
    while (true)
    {
        std::size_t p = tmpl.find(token, pos);
        if (p == std::string::npos)
        {
            out.append(tmpl, pos, std::string::npos);
            break;
        }
        out.append(tmpl, pos, p - pos);
        out.append(rawmap);
        pos = p + token.size();
    }
    return out;
}

static int install_pre_commit_hook()
{
    // Узнаём .git-директорию через git
    std::string git_dir = detect_git_dir();
    if (git_dir.empty())
    {
        std::cerr
            << "--hook-install: not a git repository or git not available\n";
        return 1;
    }

    fs::path git_path = fs::path(git_dir);
    fs::path hooks_dir = git_path / "hooks";
    std::error_code ec;
    fs::create_directories(hooks_dir, ec);

    fs::path hook_path = hooks_dir / "pre-commit";

    std::cout << "===== .git/hooks/pre-commit =====\n";

    std::string existing;
    if (fs::exists(hook_path))
    {
        std::ifstream in(hook_path);
        if (in)
        {
            std::ostringstream ss;
            ss << in.rdbuf();
            existing = ss.str();
        }
    }

    const std::string marker = "# pre-commit hook for scat wrapping";

    // Если наш хук уже есть — ничего не делаем
    if (!existing.empty() && existing.find(marker) != std::string::npos)
    {
        std::cout << "scat: pre-commit hook already contains scat wrapper\n";
        return 0;
    }

    if (existing.empty())
    {
        // Создаём новый pre-commit с твоим скриптом
        std::ofstream out(hook_path, std::ios::trunc);
        if (!out)
        {
            std::cerr << "scat: cannot write hook file: " << hook_path << "\n";
            return 1;
        }

        out << "#!/bin/sh\n"
               "# pre-commit hook for scat wrapping\n"
               "\n"
               "set -e  # если любая команда упадёт — прерываем коммит\n"
               "\n"
               "# Опционально: не мешаемся, если scat не установлен\n"
               "if ! command -v scat >/dev/null 2>&1; then\n"
               "    echo \"pre-commit: 'scat' not found in PATH, skipping "
               "wrap\"\n"
               "    exit 0\n"
               "fi\n"
               "\n"
               "echo \"pre-commit: running 'scat --wrap wrapped'...\"\n"
               "\n"
               "# Рабочая директория хуков по умолчанию — корень репозитория,\n"
               "# так что можно просто дернуть scat.\n"
               "scat --wrap .scatwrap\n"
               "\n"
               "# Добавляем всё из wrapped в индекс (новые, изменённые, "
               "удалённые)\n"
               "git add -A .scatwrap\n"
               "\n"
               "echo \"pre-commit: wrapped/ updated and added to commit\"\n"
               "\n"
               "exit 0\n";
    }
    else
    {
        // Уже есть какой-то хук — аккуратно добавляем наш блок в конец
        std::ofstream out(hook_path, std::ios::app);
        if (!out)
        {
            std::cerr << "scat: cannot append to hook file: " << hook_path
                      << "\n";
            return 1;
        }

        if (!existing.empty() && existing.back() != '\n')
            out << "\n";

        out << "\n# ----- added by scat --hook-install -----\n"
               "if command -v scat >/dev/null 2>&1; then\n"
               "    echo \"pre-commit: running 'scat --wrap wrapped'...\"\n"
               "    scat --wrap .scatwrap\n"
               "    git add -A .scatwrap\n"
               "    echo \"pre-commit: wrapped/ updated and added to commit\"\n"
               "fi\n"
               "# ----- end scat hook -----\n";
    }

#ifndef _WIN32
    // chmod +x на Unix-подобных
    std::string hp = hook_path.string();
    struct stat st;
    if (::stat(hp.c_str(), &st) == 0)
    {
        mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;
        ::chmod(hp.c_str(), mode);
    }
#endif

    return 0;
}

int scat_main(int argc, char **argv)
{
    Options opt = parse_options(argc, argv);
    g_use_absolute_paths = opt.abs_paths;
    CopyGuard copy_guard(opt.copy_out);
    if (opt.show_version)
    {
        std::cout << "scat 0.1.0\n";
        return 0;
    }

    // Установка git pre-commit hook'а и выход
    if (opt.hook_install)
    {
        return install_pre_commit_hook();
    }

    // Git info mode: print repository metadata and exit
    if (opt.git_info)
    {
        GitInfo info = detect_git_info();

        if (!info.has_commit && !info.has_remote)
        {
            std::cout << "Git: not a repository or git is not available\n";
        }
        else
        {
            if (info.has_commit)
                std::cout << "Git commit: " << info.commit << "\n";
            if (info.has_remote)
                std::cout << "Git remote: " << info.remote << "\n";
        }

        return 0;
    }

    // GH map mode: построить prefix = raw.githubusercontent/... и дальше
    // работать как -l --prefix
    if (opt.gh_map)
    {
        GitHubInfo gh = detect_github_info();
        if (!gh.ok)
        {
            std::cerr << "--ghmap: unable to detect GitHub remote/commit\n";
            return 1;
        }

        std::string prefix = "https://raw.githubusercontent.com/" + gh.user +
                             "/" + gh.repo + "/" + gh.commit + "/.scatwrap/";

            if (!opt.config_file.empty())
    {
        Config cfg;
        try {
            cfg = parse_config(opt.config_file);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }

        auto text_files = collect_from_rules(cfg.text_rules, opt);
        if (text_files.empty())
        {
            std::cerr << "No files collected.\n";
            return 0;
        }

        // Собираем "сырой" список ссылок в строку — это и есть {RAWMAP}
        Options list_opt = opt;
        list_opt.path_prefix = prefix;

        std::ostringstream oss;
        print_list_with_sizes_to(text_files, list_opt, oss);
        std::string rawmap = oss.str();

        std::string output;
        if (!cfg.map_format.empty())
            output = substitute_rawmap(cfg.map_format, rawmap);
        else
            output = rawmap;

        std::cout << output;
        return 0;
    }
        else
        {
            opt.list_only = true;
            opt.wrap_root.clear();
            opt.path_prefix = prefix;
        }
    }

    // HTTP server mode
    if (opt.server_port != 0)
        return run_server(opt);

    // ------------------------------------------------------------
    // CONFIG MODE — uses scat.txt or --config F
    // ------------------------------------------------------------
    if (!opt.config_file.empty())
        return run_config_mode(opt);

    // ------------------------------------------------------------
    // NORMAL MODE — user provided paths
    // ------------------------------------------------------------
    return run_normal_mode(opt);
}

int wrap_files_to_html(const std::vector<std::filesystem::path> &files,
                       const Options &opt)
{
    namespace fs = std::filesystem;

    if (opt.wrap_root.empty())
    {
        return 0;
    }

    fs::path root = opt.wrap_root;

    std::error_code ec;
    fs::create_directories(root, ec);

    // запоминаем все "живые" относительные пути, для которых мы только что
    // перегенерируем врапы
    std::set<std::string> alive_rel_paths;

    for (const auto &f : files)
    {
        // считаем относительный путь относительно текущего каталога
        std::error_code rec;
        fs::path rel = fs::relative(f, fs::current_path(), rec);
        if (rec)
        {
            // если не получилось — хотя бы имя файла
            rel = f.filename();
        }

        // запоминаем, что врап для этого относительного пути должен существовать
        alive_rel_paths.insert(rel.generic_string());

        fs::path dst = root / rel;
        fs::create_directories(dst.parent_path(), ec);
        std::ifstream in(f, std::ios::binary);
        if (!in)
        {
            std::cerr << "Cannot open for wrap: " << f << "\n";
            continue;
        }

        std::ostringstream ss;
        ss << in.rdbuf();

        std::string title = rel.generic_string();
        std::string html = wrap_cpp_as_html(ss.str(), title);
        std::ofstream out(dst, std::ios::binary);
        if (!out)
        {
            std::cerr << "Cannot write wrapped file: " << dst << "\n";
            continue;
        }

        out << html;
    }

    // ------------------------------------------------------------
    // ЧИСТКА СТАРЫХ ВРАПОВ
    //
    // Проходим по wrap_root рекурсивно и удаляем все обычные файлы,
    // для которых нет соответствующего "живого" относительного пути.
    // При этом НЕ трогаем конфиг-файл scat.txt (если он там есть).
    // ------------------------------------------------------------
    std::error_code rec_ec;
    for (fs::recursive_directory_iterator it(root, rec_ec), end;
         !rec_ec && it != end;
         it.increment(rec_ec))
    {
        if (!it->is_regular_file())
            continue;

        fs::path p = it->path();

        // относительный путь внутри wrap_root
        fs::path rel = fs::relative(p, root, rec_ec);
        if (rec_ec)
        {
            // что-то пошло не так — не рискуем, просто пропускаем
            rec_ec.clear();
            continue;
        }

        std::string key = rel.generic_string();

        if (!alive_rel_paths.count(key))
        {
            std::error_code rm_ec;
            fs::remove(p, rm_ec);
            // ошибок удаления игнорируем, максимум можно залогировать
        }
    }

    return 0;
}


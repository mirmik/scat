<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/scat.cpp</title>
</head>
<body>
<pre><code>
#include &quot;scat.h&quot;
#include &quot;clipboard.h&quot;
#include &quot;collector.h&quot;
#include &quot;git_info.h&quot;
#include &quot;options.h&quot;
#include &quot;parser.h&quot;
#include &quot;rules.h&quot;
#include &quot;util.h&quot;
#include &lt;cstdio&gt;
#include &lt;exception&gt;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;functional&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;set&gt;
#include &lt;sstream&gt;

#ifndef _WIN32
#include &lt;sys/stat.h&gt;
#include &lt;sys/types.h&gt;
#endif

int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                       const Options &amp;opt);

namespace fs = std::filesystem;

bool g_use_absolute_paths = false;

int apply_chunk_main(int argc, char **argv);

void print_tree(const std::vector&lt;std::filesystem::path&gt; &amp;files)
{
    // Собираем относительные пути
    std::vector&lt;std::string&gt; rels;
    rels.reserve(files.size());
    for (auto &amp;p : files)
        rels.push_back(make_display_path(p));

    std::sort(rels.begin(), rels.end());

    struct Node
    {
        std::map&lt;std::string, Node *&gt; children;
        bool is_file = false;
    };

    Node root;

    // ----------------------------
    //  Построение дерева
    // ----------------------------
    for (auto &amp;r : rels)
    {
        fs::path p = r;
        Node *cur = &amp;root;

        // вытаскиваем компоненты p в список
        std::vector&lt;std::string&gt; parts;
        for (auto &amp;part : p)
            parts.push_back(part.string());

        int total = (int)parts.size();

        for (int i = 0; i &lt; total; ++i)
        {
            const std::string &amp;name = parts[i];
            bool last = (i == total - 1);

            if (!cur-&gt;children.count(name))
                cur-&gt;children[name] = new Node();

            cur = cur-&gt;children[name];
            if (last)
                cur-&gt;is_file = true;
        }
    }

    // ----------------------------
    //  Рекурсивная печать
    // ----------------------------
    std::function&lt;void(Node *, const std::string &amp;, bool, const std::string &amp;)&gt;
        go;

    go = [&amp;](Node *node,
             const std::string &amp;name,
             bool last,
             const std::string &amp;prefix)
    {
        if (!name.empty())
        {
            std::cout &lt;&lt; prefix &lt;&lt; (last ? &quot;└── &quot; : &quot;├── &quot;) &lt;&lt; name;

            if (!node-&gt;is_file)
                std::cout &lt;&lt; &quot;/&quot;;

            std::cout &lt;&lt; &quot;\n&quot;;
        }

        // сортируем детей по имени
        std::vector&lt;std::string&gt; keys;
        keys.reserve(node-&gt;children.size());
        for (auto &amp;[k, _] : node-&gt;children)
            keys.push_back(k);
        std::sort(keys.begin(), keys.end());

        for (size_t i = 0; i &lt; keys.size(); ++i)
        {
            bool is_last = (i + 1 == keys.size());
            Node *child = node-&gt;children[keys[i]];

            go(child,
               keys[i],
               is_last,
               prefix + (name.empty() ? &quot;&quot; : (last ? &quot;    &quot; : &quot;│   &quot;)));
        }
    };

    std::cout &lt;&lt; &quot;===== PROJECT TREE =====\n&quot;;
    go(&amp;root, &quot;&quot;, true, &quot;&quot;);
    std::cout &lt;&lt; &quot;========================\n\n&quot;;
}

static void
print_list_with_sizes_to(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                         const Options &amp;opt,
                         std::ostream &amp;os)
{
    namespace fs = std::filesystem;

    struct Item
    {
        fs::path path;
        std::string display;
        std::uintmax_t size;
    };

    std::vector&lt;Item&gt; items;
    items.reserve(files.size());
    std::uintmax_t total = 0;
    std::size_t max_len = 0;

    for (auto &amp;f : files)
    {
        auto disp = make_display_path(f);
        auto sz = get_file_size(f);

        total += sz;

        std::size_t shown_len = opt.path_prefix.size() + disp.size();
        if (shown_len &gt; max_len)
            max_len = shown_len;

        items.push_back(Item{f, std::move(disp), sz});
    }

    if (opt.sorted)
    {
        std::sort(items.begin(),
                  items.end(),
                  [](const Item &amp;a, const Item &amp;b)
                  {
                      if (a.size != b.size)
                          return a.size &gt; b.size; // по убыванию
                      return a.display &lt; b.display;
                  });
    }

    const bool show_size = opt.show_size;
    for (const auto &amp;it : items)
    {
        std::string shown = opt.path_prefix + it.display;
        os &lt;&lt; shown;

        if (show_size)
        {
            if (max_len &gt; shown.size())
            {
                std::size_t pad = max_len - shown.size();
                for (std::size_t i = 0; i &lt; pad; ++i)
                    os &lt;&lt; ' ';
            }
            os &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)&quot;;
        }

        os &lt;&lt; &quot;\n&quot;;
    }

    if (show_size)
    {
        os &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;
    }
}

// старая функция теперь просто обёртка вокруг новой
static void
print_list_with_sizes(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                      const Options &amp;opt)
{
    print_list_with_sizes_to(files, opt, std::cout);
}

// Вывод всех файлов и подсчёт суммарного размера
static std::uintmax_t
dump_files_and_total(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                     const Options &amp;opt)
{
    std::uintmax_t total = 0;
    bool first = true;

    for (auto &amp;f : files)
    {
        dump_file(f, first, opt);
        first = false;
        total += get_file_size(f);
    }

    return total;
}

int run_server(const Options &amp;opt);

// общий каркас обработки списка файлов
static int process_files(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                         const Options &amp;opt,
                         const std::function&lt;void(std::uintmax_t)&gt; &amp;after_dump)
{
    if (files.empty())
    {
        std::cerr &lt;&lt; &quot;No files collected.\n&quot;;
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

static int run_config_mode(const Options &amp;opt)
{
    Config cfg;
    try
    {
        cfg = parse_config(opt.config_file);
    }
    catch (const std::exception &amp;e)
    {
        std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;
        return 1;
    }

    auto text_files = collect_from_rules(cfg.text_rules, opt);

    auto after_dump = [&amp;](std::uintmax_t total)
    {
        // TREE rules (если есть)
        if (!cfg.tree_rules.empty())
        {
            auto tree_files = collect_from_rules(cfg.tree_rules, opt);
            if (!tree_files.empty())
            {
                std::cout &lt;&lt; &quot;\n&quot;;
                print_tree(tree_files);
            }
        }

        if (opt.chunk_trailer)
        {
            std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;
            print_chunk_help();
        }

        std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;
    };

    return process_files(text_files, opt, after_dump);
}

// =========================
// NORMAL MODE
// =========================

static int run_normal_mode(const Options &amp;opt)
{
    std::vector&lt;std::filesystem::path&gt; files =
        collect_from_paths(opt.paths, opt);

    auto after_dump = [&amp;](std::uintmax_t total)
    {
        // В обычном режиме дерево не печатаем
        if (opt.chunk_trailer)
        {
            std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;
            print_chunk_help();
        }

        std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;
    };

    return process_files(files, opt, after_dump);
}

static std::string substitute_rawmap(const std::string &amp;tmpl,
                                     const std::string &amp;rawmap)
{
    const std::string token = &quot;{RAWMAP}&quot;;
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
            &lt;&lt; &quot;--hook-install: not a git repository or git not available\n&quot;;
        return 1;
    }

    fs::path git_path = fs::path(git_dir);
    fs::path hooks_dir = git_path / &quot;hooks&quot;;
    std::error_code ec;
    fs::create_directories(hooks_dir, ec);

    fs::path hook_path = hooks_dir / &quot;pre-commit&quot;;

    std::cout &lt;&lt; &quot;===== .git/hooks/pre-commit =====\n&quot;;

    std::string existing;
    if (fs::exists(hook_path))
    {
        std::ifstream in(hook_path);
        if (in)
        {
            std::ostringstream ss;
            ss &lt;&lt; in.rdbuf();
            existing = ss.str();
        }
    }

    const std::string marker = &quot;# pre-commit hook for scat wrapping&quot;;

    // Если наш хук уже есть — ничего не делаем
    if (!existing.empty() &amp;&amp; existing.find(marker) != std::string::npos)
    {
        std::cout &lt;&lt; &quot;scat: pre-commit hook already contains scat wrapper\n&quot;;
        return 0;
    }

    if (existing.empty())
    {
        // Создаём новый pre-commit с твоим скриптом
        std::ofstream out(hook_path, std::ios::trunc);
        if (!out)
        {
            std::cerr &lt;&lt; &quot;scat: cannot write hook file: &quot; &lt;&lt; hook_path &lt;&lt; &quot;\n&quot;;
            return 1;
        }

        out &lt;&lt; &quot;#!/bin/sh\n&quot;
               &quot;# pre-commit hook for scat wrapping\n&quot;
               &quot;\n&quot;
               &quot;set -e  # если любая команда упадёт — прерываем коммит\n&quot;
               &quot;\n&quot;
               &quot;# Опционально: не мешаемся, если scat не установлен\n&quot;
               &quot;if ! command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;
               &quot;    echo \&quot;pre-commit: 'scat' not found in PATH, skipping &quot;
               &quot;wrap\&quot;\n&quot;
               &quot;    exit 0\n&quot;
               &quot;fi\n&quot;
               &quot;\n&quot;
               &quot;echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;
               &quot;\n&quot;
               &quot;# Рабочая директория хуков по умолчанию — корень репозитория,\n&quot;
               &quot;# так что можно просто дернуть scat.\n&quot;
               &quot;scat --wrap .scatwrap\n&quot;
               &quot;\n&quot;
               &quot;# Добавляем всё из wrapped в индекс (новые, изменённые, &quot;
               &quot;удалённые)\n&quot;
               &quot;git add -A .scatwrap\n&quot;
               &quot;\n&quot;
               &quot;echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;
               &quot;\n&quot;
               &quot;exit 0\n&quot;;
    }
    else
    {
        // Уже есть какой-то хук — аккуратно добавляем наш блок в конец
        std::ofstream out(hook_path, std::ios::app);
        if (!out)
        {
            std::cerr &lt;&lt; &quot;scat: cannot append to hook file: &quot; &lt;&lt; hook_path
                      &lt;&lt; &quot;\n&quot;;
            return 1;
        }

        if (!existing.empty() &amp;&amp; existing.back() != '\n')
            out &lt;&lt; &quot;\n&quot;;

        out &lt;&lt; &quot;\n# ----- added by scat --hook-install -----\n&quot;
               &quot;if command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;
               &quot;    echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;
               &quot;    scat --wrap .scatwrap\n&quot;
               &quot;    git add -A .scatwrap\n&quot;
               &quot;    echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;
               &quot;fi\n&quot;
               &quot;# ----- end scat hook -----\n&quot;;
    }

#ifndef _WIN32
    // chmod +x на Unix-подобных
    std::string hp = hook_path.string();
    struct stat st;
    if (::stat(hp.c_str(), &amp;st) == 0)
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

    // Установка git pre-commit hook'а и выход
    if (opt.hook_install)
    {
        return install_pre_commit_hook();
    }

    // Git info mode: print repository metadata and exit
    if (opt.git_info)
    {
        GitInfo info = detect_git_info();

        if (!info.has_commit &amp;&amp; !info.has_remote)
        {
            std::cout &lt;&lt; &quot;Git: not a repository or git is not available\n&quot;;
        }
        else
        {
            if (info.has_commit)
                std::cout &lt;&lt; &quot;Git commit: &quot; &lt;&lt; info.commit &lt;&lt; &quot;\n&quot;;
            if (info.has_remote)
                std::cout &lt;&lt; &quot;Git remote: &quot; &lt;&lt; info.remote &lt;&lt; &quot;\n&quot;;
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
            std::cerr &lt;&lt; &quot;--ghmap: unable to detect GitHub remote/commit\n&quot;;
            return 1;
        }

        std::string prefix = &quot;https://raw.githubusercontent.com/&quot; + gh.user +
                             &quot;/&quot; + gh.repo + &quot;/&quot; + gh.commit + &quot;/.scatwrap/&quot;;

        if (!opt.config_file.empty())
        {
            // тут уже чисто логика MAPFORMAT + collect_from_rules
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
    // Chunk help
    // ------------------------------------------------------------
    if (opt.chunk_help)
    {
        print_chunk_help();
        return 0;
    }

    // ------------------------------------------------------------
    // Apply patch mode
    // ------------------------------------------------------------
    if (!opt.apply_file.empty() || opt.apply_stdin)
    {
        if (opt.apply_stdin)
        {
            namespace fs = std::filesystem;

            std::stringstream ss;
            ss &lt;&lt; std::cin.rdbuf();
            fs::path tmp = fs::temp_directory_path() / &quot;scat_stdin_patch.txt&quot;;
            {
                std::ofstream out(tmp);
                out &lt;&lt; ss.str();
            }

            std::string tmp_str = tmp.string();
            const char *args[] = {&quot;apply&quot;, tmp_str.c_str()};
            int r = apply_chunk_main(2, const_cast&lt;char **&gt;(args));
            fs::remove(tmp);
            return r;
        }
        else
        {
            std::string file = opt.apply_file;
            const char *args[] = {&quot;apply&quot;, file.c_str()};
            return apply_chunk_main(2, const_cast&lt;char **&gt;(args));
        }
    }

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

int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,
                       const Options &amp;opt)
{
    namespace fs = std::filesystem;

    if (opt.wrap_root.empty())
    {
        return 0;
    }

    fs::path root = opt.wrap_root;

    std::error_code ec;
    fs::create_directories(root, ec);

    for (const auto &amp;f : files)
    {
        // считаем относительный путь относительно текущего каталога
        std::error_code rec;
        fs::path rel = fs::relative(f, fs::current_path(), rec);
        if (rec)
        {
            // если не получилось — хотя бы имя файла
            rel = f.filename();
        }

        fs::path dst = root / rel;
        fs::create_directories(dst.parent_path(), ec);

        std::ifstream in(f, std::ios::binary);
        if (!in)
        {
            std::cerr &lt;&lt; &quot;Cannot open for wrap: &quot; &lt;&lt; f &lt;&lt; &quot;\n&quot;;
            continue;
        }

        std::ostringstream ss;
        ss &lt;&lt; in.rdbuf();

        std::string title = rel.generic_string();
        std::string html = wrap_cpp_as_html(ss.str(), title);

        std::ofstream out(dst, std::ios::binary);
        if (!out)
        {
            std::cerr &lt;&lt; &quot;Cannot write wrapped file: &quot; &lt;&lt; dst &lt;&lt; &quot;\n&quot;;
            continue;
        }

        out &lt;&lt; html;
    }

    return 0;
}

</code></pre>
</body>
</html>

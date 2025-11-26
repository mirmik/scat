<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/scat.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;scat.h&quot;<br>
#include &quot;clipboard.h&quot;<br>
#include &quot;collector.h&quot;<br>
#include &quot;git_info.h&quot;<br>
#include &quot;options.h&quot;<br>
#include &quot;parser.h&quot;<br>
#include &quot;rules.h&quot;<br>
#include &quot;util.h&quot;<br>
#include &lt;cstdio&gt;<br>
#include &lt;exception&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;functional&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;map&gt;<br>
#include &lt;set&gt;<br>
#include &lt;sstream&gt;<br>
<br>
#ifndef _WIN32<br>
#include &lt;sys/stat.h&gt;<br>
#include &lt;sys/types.h&gt;<br>
#endif<br>
<br>
int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                       const Options &amp;opt);<br>
<br>
namespace fs = std::filesystem;<br>
<br>
bool g_use_absolute_paths = false;<br>
<br>
int apply_chunk_main(int argc, char **argv);<br>
<br>
void print_tree(const std::vector&lt;std::filesystem::path&gt; &amp;files)<br>
{<br>
    // Собираем относительные пути<br>
    std::vector&lt;std::string&gt; rels;<br>
    rels.reserve(files.size());<br>
    for (auto &amp;p : files)<br>
        rels.push_back(make_display_path(p));<br>
<br>
    std::sort(rels.begin(), rels.end());<br>
<br>
    struct Node<br>
    {<br>
        std::map&lt;std::string, Node *&gt; children;<br>
        bool is_file = false;<br>
    };<br>
<br>
    Node root;<br>
<br>
    // ----------------------------<br>
    //  Построение дерева<br>
    // ----------------------------<br>
    for (auto &amp;r : rels)<br>
    {<br>
        fs::path p = r;<br>
        Node *cur = &amp;root;<br>
<br>
        // вытаскиваем компоненты p в список<br>
        std::vector&lt;std::string&gt; parts;<br>
        for (auto &amp;part : p)<br>
            parts.push_back(part.string());<br>
<br>
        int total = (int)parts.size();<br>
<br>
        for (int i = 0; i &lt; total; ++i)<br>
        {<br>
            const std::string &amp;name = parts[i];<br>
            bool last = (i == total - 1);<br>
<br>
            if (!cur-&gt;children.count(name))<br>
                cur-&gt;children[name] = new Node();<br>
<br>
            cur = cur-&gt;children[name];<br>
            if (last)<br>
                cur-&gt;is_file = true;<br>
        }<br>
    }<br>
<br>
    // ----------------------------<br>
    //  Рекурсивная печать<br>
    // ----------------------------<br>
    std::function&lt;void(Node *, const std::string &amp;, bool, const std::string &amp;)&gt;<br>
        go;<br>
<br>
    go = [&amp;](Node *node,<br>
             const std::string &amp;name,<br>
             bool last,<br>
             const std::string &amp;prefix)<br>
    {<br>
        if (!name.empty())<br>
        {<br>
            std::cout &lt;&lt; prefix &lt;&lt; (last ? &quot;└── &quot; : &quot;├── &quot;) &lt;&lt; name;<br>
<br>
            if (!node-&gt;is_file)<br>
                std::cout &lt;&lt; &quot;/&quot;;<br>
<br>
            std::cout &lt;&lt; &quot;\n&quot;;<br>
        }<br>
<br>
        // сортируем детей по имени<br>
        std::vector&lt;std::string&gt; keys;<br>
        keys.reserve(node-&gt;children.size());<br>
        for (auto &amp;[k, _] : node-&gt;children)<br>
            keys.push_back(k);<br>
        std::sort(keys.begin(), keys.end());<br>
<br>
        for (size_t i = 0; i &lt; keys.size(); ++i)<br>
        {<br>
            bool is_last = (i + 1 == keys.size());<br>
            Node *child = node-&gt;children[keys[i]];<br>
<br>
            go(child,<br>
               keys[i],<br>
               is_last,<br>
               prefix + (name.empty() ? &quot;&quot; : (last ? &quot;    &quot; : &quot;│   &quot;)));<br>
        }<br>
    };<br>
<br>
    std::cout &lt;&lt; &quot;===== PROJECT TREE =====\n&quot;;<br>
    go(&amp;root, &quot;&quot;, true, &quot;&quot;);<br>
    std::cout &lt;&lt; &quot;========================\n\n&quot;;<br>
}<br>
<br>
static void<br>
print_list_with_sizes_to(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                         const Options &amp;opt,<br>
                         std::ostream &amp;os)<br>
{<br>
    namespace fs = std::filesystem;<br>
<br>
    struct Item<br>
    {<br>
        fs::path path;<br>
        std::string display;<br>
        std::uintmax_t size;<br>
    };<br>
<br>
    std::vector&lt;Item&gt; items;<br>
    items.reserve(files.size());<br>
    std::uintmax_t total = 0;<br>
    std::size_t max_len = 0;<br>
<br>
    for (auto &amp;f : files)<br>
    {<br>
        auto disp = make_display_path(f);<br>
        auto sz = get_file_size(f);<br>
<br>
        total += sz;<br>
<br>
        std::size_t shown_len = opt.path_prefix.size() + disp.size();<br>
        if (shown_len &gt; max_len)<br>
            max_len = shown_len;<br>
<br>
        items.push_back(Item{f, std::move(disp), sz});<br>
    }<br>
<br>
    if (opt.sorted)<br>
    {<br>
        std::sort(items.begin(),<br>
                  items.end(),<br>
                  [](const Item &amp;a, const Item &amp;b)<br>
                  {<br>
                      if (a.size != b.size)<br>
                          return a.size &gt; b.size; // по убыванию<br>
                      return a.display &lt; b.display;<br>
                  });<br>
    }<br>
<br>
    const bool show_size = opt.show_size;<br>
    for (const auto &amp;it : items)<br>
    {<br>
        std::string shown = opt.path_prefix + it.display;<br>
        os &lt;&lt; shown;<br>
<br>
        if (show_size)<br>
        {<br>
            if (max_len &gt; shown.size())<br>
            {<br>
                std::size_t pad = max_len - shown.size();<br>
                for (std::size_t i = 0; i &lt; pad; ++i)<br>
                    os &lt;&lt; ' ';<br>
            }<br>
            os &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)&quot;;<br>
        }<br>
<br>
        os &lt;&lt; &quot;\n&quot;;<br>
    }<br>
<br>
    if (show_size)<br>
    {<br>
        os &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
    }<br>
}<br>
<br>
// старая функция теперь просто обёртка вокруг новой<br>
static void<br>
print_list_with_sizes(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                      const Options &amp;opt)<br>
{<br>
    print_list_with_sizes_to(files, opt, std::cout);<br>
}<br>
<br>
// Вывод всех файлов и подсчёт суммарного размера<br>
static std::uintmax_t<br>
dump_files_and_total(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                     const Options &amp;opt)<br>
{<br>
    std::uintmax_t total = 0;<br>
    bool first = true;<br>
<br>
    for (auto &amp;f : files)<br>
    {<br>
        dump_file(f, first, opt);<br>
        first = false;<br>
        total += get_file_size(f);<br>
    }<br>
<br>
    return total;<br>
}<br>
<br>
int run_server(const Options &amp;opt);<br>
<br>
// общий каркас обработки списка файлов<br>
static int process_files(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                         const Options &amp;opt,<br>
                         const std::function&lt;void(std::uintmax_t)&gt; &amp;after_dump)<br>
{<br>
    if (files.empty())<br>
    {<br>
        std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
        return 0;<br>
    }<br>
<br>
    if (!opt.wrap_root.empty())<br>
        return wrap_files_to_html(files, opt);<br>
<br>
    if (opt.list_only)<br>
    {<br>
        print_list_with_sizes(files, opt);<br>
        return 0;<br>
    }<br>
<br>
    std::uintmax_t total = dump_files_and_total(files, opt);<br>
<br>
    if (after_dump)<br>
        after_dump(total);<br>
<br>
    return 0;<br>
}<br>
<br>
// =========================<br>
// CONFIG MODE<br>
// =========================<br>
<br>
static int run_config_mode(const Options &amp;opt)<br>
{<br>
    Config cfg;<br>
    try<br>
    {<br>
        cfg = parse_config(opt.config_file);<br>
    }<br>
    catch (const std::exception &amp;e)<br>
    {<br>
        std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
<br>
    auto after_dump = [&amp;](std::uintmax_t total)<br>
    {<br>
        // TREE rules (если есть)<br>
        if (!cfg.tree_rules.empty())<br>
        {<br>
            auto tree_files = collect_from_rules(cfg.tree_rules, opt);<br>
            if (!tree_files.empty())<br>
            {<br>
                std::cout &lt;&lt; &quot;\n&quot;;<br>
                print_tree(tree_files);<br>
            }<br>
        }<br>
<br>
        if (opt.chunk_trailer)<br>
        {<br>
            std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
            print_chunk_help();<br>
        }<br>
<br>
        std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
    };<br>
<br>
    return process_files(text_files, opt, after_dump);<br>
}<br>
<br>
// =========================<br>
// NORMAL MODE<br>
// =========================<br>
<br>
static int run_normal_mode(const Options &amp;opt)<br>
{<br>
    std::vector&lt;std::filesystem::path&gt; files =<br>
        collect_from_paths(opt.paths, opt);<br>
<br>
    auto after_dump = [&amp;](std::uintmax_t total)<br>
    {<br>
        // В обычном режиме дерево не печатаем<br>
        if (opt.chunk_trailer)<br>
        {<br>
            std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
            print_chunk_help();<br>
        }<br>
<br>
        std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
    };<br>
<br>
    return process_files(files, opt, after_dump);<br>
}<br>
<br>
static std::string substitute_rawmap(const std::string &amp;tmpl,<br>
                                     const std::string &amp;rawmap)<br>
{<br>
    const std::string token = &quot;{RAWMAP}&quot;;<br>
    if (tmpl.empty())<br>
        return rawmap;<br>
<br>
    std::string out;<br>
    out.reserve(tmpl.size() + rawmap.size() + 16);<br>
<br>
    std::size_t pos = 0;<br>
    while (true)<br>
    {<br>
        std::size_t p = tmpl.find(token, pos);<br>
        if (p == std::string::npos)<br>
        {<br>
            out.append(tmpl, pos, std::string::npos);<br>
            break;<br>
        }<br>
        out.append(tmpl, pos, p - pos);<br>
        out.append(rawmap);<br>
        pos = p + token.size();<br>
    }<br>
    return out;<br>
}<br>
<br>
static int install_pre_commit_hook()<br>
{<br>
    // Узнаём .git-директорию через git<br>
    std::string git_dir = detect_git_dir();<br>
    if (git_dir.empty())<br>
    {<br>
        std::cerr<br>
            &lt;&lt; &quot;--hook-install: not a git repository or git not available\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    fs::path git_path = fs::path(git_dir);<br>
    fs::path hooks_dir = git_path / &quot;hooks&quot;;<br>
    std::error_code ec;<br>
    fs::create_directories(hooks_dir, ec);<br>
<br>
    fs::path hook_path = hooks_dir / &quot;pre-commit&quot;;<br>
<br>
    std::cout &lt;&lt; &quot;===== .git/hooks/pre-commit =====\n&quot;;<br>
<br>
    std::string existing;<br>
    if (fs::exists(hook_path))<br>
    {<br>
        std::ifstream in(hook_path);<br>
        if (in)<br>
        {<br>
            std::ostringstream ss;<br>
            ss &lt;&lt; in.rdbuf();<br>
            existing = ss.str();<br>
        }<br>
    }<br>
<br>
    const std::string marker = &quot;# pre-commit hook for scat wrapping&quot;;<br>
<br>
    // Если наш хук уже есть — ничего не делаем<br>
    if (!existing.empty() &amp;&amp; existing.find(marker) != std::string::npos)<br>
    {<br>
        std::cout &lt;&lt; &quot;scat: pre-commit hook already contains scat wrapper\n&quot;;<br>
        return 0;<br>
    }<br>
<br>
    if (existing.empty())<br>
    {<br>
        // Создаём новый pre-commit с твоим скриптом<br>
        std::ofstream out(hook_path, std::ios::trunc);<br>
        if (!out)<br>
        {<br>
            std::cerr &lt;&lt; &quot;scat: cannot write hook file: &quot; &lt;&lt; hook_path &lt;&lt; &quot;\n&quot;;<br>
            return 1;<br>
        }<br>
<br>
        out &lt;&lt; &quot;#!/bin/sh\n&quot;<br>
               &quot;# pre-commit hook for scat wrapping\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;set -e  # если любая команда упадёт — прерываем коммит\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;# Опционально: не мешаемся, если scat не установлен\n&quot;<br>
               &quot;if ! command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
               &quot;    echo \&quot;pre-commit: 'scat' not found in PATH, skipping &quot;<br>
               &quot;wrap\&quot;\n&quot;<br>
               &quot;    exit 0\n&quot;<br>
               &quot;fi\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;# Рабочая директория хуков по умолчанию — корень репозитория,\n&quot;<br>
               &quot;# так что можно просто дернуть scat.\n&quot;<br>
               &quot;scat --wrap .scatwrap\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;# Добавляем всё из wrapped в индекс (новые, изменённые, &quot;<br>
               &quot;удалённые)\n&quot;<br>
               &quot;git add -A .scatwrap\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
               &quot;\n&quot;<br>
               &quot;exit 0\n&quot;;<br>
    }<br>
    else<br>
    {<br>
        // Уже есть какой-то хук — аккуратно добавляем наш блок в конец<br>
        std::ofstream out(hook_path, std::ios::app);<br>
        if (!out)<br>
        {<br>
            std::cerr &lt;&lt; &quot;scat: cannot append to hook file: &quot; &lt;&lt; hook_path<br>
                      &lt;&lt; &quot;\n&quot;;<br>
            return 1;<br>
        }<br>
<br>
        if (!existing.empty() &amp;&amp; existing.back() != '\n')<br>
            out &lt;&lt; &quot;\n&quot;;<br>
<br>
        out &lt;&lt; &quot;\n# ----- added by scat --hook-install -----\n&quot;<br>
               &quot;if command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
               &quot;    echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
               &quot;    scat --wrap .scatwrap\n&quot;<br>
               &quot;    git add -A .scatwrap\n&quot;<br>
               &quot;    echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
               &quot;fi\n&quot;<br>
               &quot;# ----- end scat hook -----\n&quot;;<br>
    }<br>
<br>
#ifndef _WIN32<br>
    // chmod +x на Unix-подобных<br>
    std::string hp = hook_path.string();<br>
    struct stat st;<br>
    if (::stat(hp.c_str(), &amp;st) == 0)<br>
    {<br>
        mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;<br>
        ::chmod(hp.c_str(), mode);<br>
    }<br>
#endif<br>
<br>
    return 0;<br>
}<br>
<br>
int scat_main(int argc, char **argv)<br>
{<br>
    Options opt = parse_options(argc, argv);<br>
    g_use_absolute_paths = opt.abs_paths;<br>
    CopyGuard copy_guard(opt.copy_out);<br>
<br>
    // Установка git pre-commit hook'а и выход<br>
    if (opt.hook_install)<br>
    {<br>
        return install_pre_commit_hook();<br>
    }<br>
<br>
    // Git info mode: print repository metadata and exit<br>
    if (opt.git_info)<br>
    {<br>
        GitInfo info = detect_git_info();<br>
<br>
        if (!info.has_commit &amp;&amp; !info.has_remote)<br>
        {<br>
            std::cout &lt;&lt; &quot;Git: not a repository or git is not available\n&quot;;<br>
        }<br>
        else<br>
        {<br>
            if (info.has_commit)<br>
                std::cout &lt;&lt; &quot;Git commit: &quot; &lt;&lt; info.commit &lt;&lt; &quot;\n&quot;;<br>
            if (info.has_remote)<br>
                std::cout &lt;&lt; &quot;Git remote: &quot; &lt;&lt; info.remote &lt;&lt; &quot;\n&quot;;<br>
        }<br>
<br>
        return 0;<br>
    }<br>
<br>
    // GH map mode: построить prefix = raw.githubusercontent/... и дальше<br>
    // работать как -l --prefix<br>
    if (opt.gh_map)<br>
    {<br>
        GitHubInfo gh = detect_github_info();<br>
        if (!gh.ok)<br>
        {<br>
            std::cerr &lt;&lt; &quot;--ghmap: unable to detect GitHub remote/commit\n&quot;;<br>
            return 1;<br>
        }<br>
<br>
        std::string prefix = &quot;https://raw.githubusercontent.com/&quot; + gh.user +<br>
                             &quot;/&quot; + gh.repo + &quot;/&quot; + gh.commit + &quot;/.scatwrap/&quot;;<br>
<br>
            if (!opt.config_file.empty())<br>
    {<br>
        Config cfg;<br>
        try {<br>
            cfg = parse_config(opt.config_file);<br>
        } catch (const std::exception&amp; e) {<br>
            std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
            return 1;<br>
        }<br>
<br>
        auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
        if (text_files.empty())<br>
        {<br>
            std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
            return 0;<br>
        }<br>
<br>
        // Собираем &quot;сырой&quot; список ссылок в строку — это и есть {RAWMAP}<br>
        Options list_opt = opt;<br>
        list_opt.path_prefix = prefix;<br>
<br>
        std::ostringstream oss;<br>
        print_list_with_sizes_to(text_files, list_opt, oss);<br>
        std::string rawmap = oss.str();<br>
<br>
        std::string output;<br>
        if (!cfg.map_format.empty())<br>
            output = substitute_rawmap(cfg.map_format, rawmap);<br>
        else<br>
            output = rawmap;<br>
<br>
        std::cout &lt;&lt; output;<br>
        return 0;<br>
    }<br>
        else<br>
        {<br>
            opt.list_only = true;<br>
            opt.wrap_root.clear();<br>
            opt.path_prefix = prefix;<br>
        }<br>
    }<br>
<br>
    // HTTP server mode<br>
    if (opt.server_port != 0)<br>
        return run_server(opt);<br>
<br>
    // ------------------------------------------------------------<br>
    // Chunk help<br>
    // ------------------------------------------------------------<br>
    if (opt.chunk_help)<br>
    {<br>
        print_chunk_help();<br>
        return 0;<br>
    }<br>
<br>
    // ------------------------------------------------------------<br>
    // Apply patch mode<br>
    // ------------------------------------------------------------<br>
    if (!opt.apply_file.empty() || opt.apply_stdin)<br>
    {<br>
        if (opt.apply_stdin)<br>
        {<br>
            namespace fs = std::filesystem;<br>
<br>
            std::stringstream ss;<br>
            ss &lt;&lt; std::cin.rdbuf();<br>
            fs::path tmp = fs::temp_directory_path() / &quot;scat_stdin_patch.txt&quot;;<br>
            {<br>
                std::ofstream out(tmp);<br>
                out &lt;&lt; ss.str();<br>
            }<br>
<br>
            std::string tmp_str = tmp.string();<br>
            const char *args[] = {&quot;apply&quot;, tmp_str.c_str()};<br>
            int r = apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
            fs::remove(tmp);<br>
            return r;<br>
        }<br>
        else<br>
        {<br>
            std::string file = opt.apply_file;<br>
            const char *args[] = {&quot;apply&quot;, file.c_str()};<br>
            return apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
        }<br>
    }<br>
<br>
    // ------------------------------------------------------------<br>
    // CONFIG MODE — uses scat.txt or --config F<br>
    // ------------------------------------------------------------<br>
    if (!opt.config_file.empty())<br>
        return run_config_mode(opt);<br>
<br>
    // ------------------------------------------------------------<br>
    // NORMAL MODE — user provided paths<br>
    // ------------------------------------------------------------<br>
    return run_normal_mode(opt);<br>
}<br>
<br>
int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
                       const Options &amp;opt)<br>
{<br>
    namespace fs = std::filesystem;<br>
<br>
    if (opt.wrap_root.empty())<br>
    {<br>
        return 0;<br>
    }<br>
<br>
    fs::path root = opt.wrap_root;<br>
<br>
    std::error_code ec;<br>
    fs::create_directories(root, ec);<br>
<br>
    for (const auto &amp;f : files)<br>
    {<br>
        // считаем относительный путь относительно текущего каталога<br>
        std::error_code rec;<br>
        fs::path rel = fs::relative(f, fs::current_path(), rec);<br>
        if (rec)<br>
        {<br>
            // если не получилось — хотя бы имя файла<br>
            rel = f.filename();<br>
        }<br>
<br>
        fs::path dst = root / rel;<br>
        fs::create_directories(dst.parent_path(), ec);<br>
<br>
        std::ifstream in(f, std::ios::binary);<br>
        if (!in)<br>
        {<br>
            std::cerr &lt;&lt; &quot;Cannot open for wrap: &quot; &lt;&lt; f &lt;&lt; &quot;\n&quot;;<br>
            continue;<br>
        }<br>
<br>
        std::ostringstream ss;<br>
        ss &lt;&lt; in.rdbuf();<br>
<br>
        std::string title = rel.generic_string();<br>
        std::string html = wrap_cpp_as_html(ss.str(), title);<br>
<br>
        std::ofstream out(dst, std::ios::binary);<br>
        if (!out)<br>
        {<br>
            std::cerr &lt;&lt; &quot;Cannot write wrapped file: &quot; &lt;&lt; dst &lt;&lt; &quot;\n&quot;;<br>
            continue;<br>
        }<br>
<br>
        out &lt;&lt; html;<br>
    }<br>
<br>
    return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

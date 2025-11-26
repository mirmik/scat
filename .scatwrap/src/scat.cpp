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
&#9;&#9;&#9;&#9;&#9;const Options &amp;opt);<br>
<br>
namespace fs = std::filesystem;<br>
<br>
bool g_use_absolute_paths = false;<br>
<br>
int apply_chunk_main(int argc, char **argv);<br>
<br>
void print_tree(const std::vector&lt;std::filesystem::path&gt; &amp;files)<br>
{<br>
&#9;// Собираем относительные пути<br>
&#9;std::vector&lt;std::string&gt; rels;<br>
&#9;rels.reserve(files.size());<br>
&#9;for (auto &amp;p : files)<br>
&#9;&#9;rels.push_back(make_display_path(p));<br>
<br>
&#9;std::sort(rels.begin(), rels.end());<br>
<br>
&#9;struct Node<br>
&#9;{<br>
&#9;&#9;std::map&lt;std::string, Node *&gt; children;<br>
&#9;&#9;bool is_file = false;<br>
&#9;};<br>
<br>
&#9;Node root;<br>
<br>
&#9;// ----------------------------<br>
&#9;//  Построение дерева<br>
&#9;// ----------------------------<br>
&#9;for (auto &amp;r : rels)<br>
&#9;{<br>
&#9;&#9;fs::path p = r;<br>
&#9;&#9;Node *cur = &amp;root;<br>
<br>
&#9;&#9;// вытаскиваем компоненты p в список<br>
&#9;&#9;std::vector&lt;std::string&gt; parts;<br>
&#9;&#9;for (auto &amp;part : p)<br>
&#9;&#9;&#9;parts.push_back(part.string());<br>
<br>
&#9;&#9;int total = (int)parts.size();<br>
<br>
&#9;&#9;for (int i = 0; i &lt; total; ++i)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;name = parts[i];<br>
&#9;&#9;&#9;bool last = (i == total - 1);<br>
<br>
&#9;&#9;&#9;if (!cur-&gt;children.count(name))<br>
&#9;&#9;&#9;&#9;cur-&gt;children[name] = new Node();<br>
<br>
&#9;&#9;&#9;cur = cur-&gt;children[name];<br>
&#9;&#9;&#9;if (last)<br>
&#9;&#9;&#9;&#9;cur-&gt;is_file = true;<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;// ----------------------------<br>
&#9;//  Рекурсивная печать<br>
&#9;// ----------------------------<br>
&#9;std::function&lt;void(Node *, const std::string &amp;, bool, const std::string &amp;)&gt;<br>
&#9;&#9;go;<br>
<br>
&#9;go = [&amp;](Node *node,<br>
&#9;&#9;&#9;const std::string &amp;name,<br>
&#9;&#9;&#9;bool last,<br>
&#9;&#9;&#9;const std::string &amp;prefix)<br>
&#9;{<br>
&#9;&#9;if (!name.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cout &lt;&lt; prefix &lt;&lt; (last ? &quot;└── &quot; : &quot;├── &quot;) &lt;&lt; name;<br>
<br>
&#9;&#9;&#9;if (!node-&gt;is_file)<br>
&#9;&#9;&#9;&#9;std::cout &lt;&lt; &quot;/&quot;;<br>
<br>
&#9;&#9;&#9;std::cout &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// сортируем детей по имени<br>
&#9;&#9;std::vector&lt;std::string&gt; keys;<br>
&#9;&#9;keys.reserve(node-&gt;children.size());<br>
&#9;&#9;for (auto &amp;[k, _] : node-&gt;children)<br>
&#9;&#9;&#9;keys.push_back(k);<br>
&#9;&#9;std::sort(keys.begin(), keys.end());<br>
<br>
&#9;&#9;for (size_t i = 0; i &lt; keys.size(); ++i)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;bool is_last = (i + 1 == keys.size());<br>
&#9;&#9;&#9;Node *child = node-&gt;children[keys[i]];<br>
<br>
&#9;&#9;&#9;go(child,<br>
&#9;&#9;&#9;keys[i],<br>
&#9;&#9;&#9;is_last,<br>
&#9;&#9;&#9;prefix + (name.empty() ? &quot;&quot; : (last ? &quot;    &quot; : &quot;│   &quot;)));<br>
&#9;&#9;}<br>
&#9;};<br>
<br>
&#9;std::cout &lt;&lt; &quot;===== PROJECT TREE =====\n&quot;;<br>
&#9;go(&amp;root, &quot;&quot;, true, &quot;&quot;);<br>
&#9;std::cout &lt;&lt; &quot;========================\n\n&quot;;<br>
}<br>
<br>
static void<br>
print_list_with_sizes_to(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&#9;&#9;&#9;&#9;&#9;&#9;const Options &amp;opt,<br>
&#9;&#9;&#9;&#9;&#9;&#9;std::ostream &amp;os)<br>
{<br>
&#9;namespace fs = std::filesystem;<br>
<br>
&#9;struct Item<br>
&#9;{<br>
&#9;&#9;fs::path path;<br>
&#9;&#9;std::string display;<br>
&#9;&#9;std::uintmax_t size;<br>
&#9;};<br>
<br>
&#9;std::vector&lt;Item&gt; items;<br>
&#9;items.reserve(files.size());<br>
&#9;std::uintmax_t total = 0;<br>
&#9;std::size_t max_len = 0;<br>
<br>
&#9;for (auto &amp;f : files)<br>
&#9;{<br>
&#9;&#9;auto disp = make_display_path(f);<br>
&#9;&#9;auto sz = get_file_size(f);<br>
<br>
&#9;&#9;total += sz;<br>
<br>
&#9;&#9;std::size_t shown_len = opt.path_prefix.size() + disp.size();<br>
&#9;&#9;if (shown_len &gt; max_len)<br>
&#9;&#9;&#9;max_len = shown_len;<br>
<br>
&#9;&#9;items.push_back(Item{f, std::move(disp), sz});<br>
&#9;}<br>
<br>
&#9;if (opt.sorted)<br>
&#9;{<br>
&#9;&#9;std::sort(items.begin(),<br>
&#9;&#9;&#9;&#9;items.end(),<br>
&#9;&#9;&#9;&#9;[](const Item &amp;a, const Item &amp;b)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;if (a.size != b.size)<br>
&#9;&#9;&#9;&#9;&#9;&#9;return a.size &gt; b.size; // по убыванию<br>
&#9;&#9;&#9;&#9;&#9;return a.display &lt; b.display;<br>
&#9;&#9;&#9;&#9;});<br>
&#9;}<br>
<br>
&#9;const bool show_size = opt.show_size;<br>
&#9;for (const auto &amp;it : items)<br>
&#9;{<br>
&#9;&#9;std::string shown = opt.path_prefix + it.display;<br>
&#9;&#9;os &lt;&lt; shown;<br>
<br>
&#9;&#9;if (show_size)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (max_len &gt; shown.size())<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::size_t pad = max_len - shown.size();<br>
&#9;&#9;&#9;&#9;for (std::size_t i = 0; i &lt; pad; ++i)<br>
&#9;&#9;&#9;&#9;&#9;os &lt;&lt; ' ';<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;os &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)&quot;;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;os &lt;&lt; &quot;\n&quot;;<br>
&#9;}<br>
<br>
&#9;if (show_size)<br>
&#9;{<br>
&#9;&#9;os &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&#9;}<br>
}<br>
<br>
// старая функция теперь просто обёртка вокруг новой<br>
static void<br>
print_list_with_sizes(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&#9;&#9;&#9;&#9;&#9;const Options &amp;opt)<br>
{<br>
&#9;print_list_with_sizes_to(files, opt, std::cout);<br>
}<br>
<br>
// Вывод всех файлов и подсчёт суммарного размера<br>
static std::uintmax_t<br>
dump_files_and_total(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&#9;&#9;&#9;&#9;&#9;const Options &amp;opt)<br>
{<br>
&#9;std::uintmax_t total = 0;<br>
&#9;bool first = true;<br>
<br>
&#9;for (auto &amp;f : files)<br>
&#9;{<br>
&#9;&#9;dump_file(f, first, opt);<br>
&#9;&#9;first = false;<br>
&#9;&#9;total += get_file_size(f);<br>
&#9;}<br>
<br>
&#9;return total;<br>
}<br>
<br>
int run_server(const Options &amp;opt);<br>
<br>
// общий каркас обработки списка файлов<br>
static int process_files(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&#9;&#9;&#9;&#9;&#9;&#9;const Options &amp;opt,<br>
&#9;&#9;&#9;&#9;&#9;&#9;const std::function&lt;void(std::uintmax_t)&gt; &amp;after_dump)<br>
{<br>
&#9;if (files.empty())<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;if (!opt.wrap_root.empty())<br>
&#9;&#9;return wrap_files_to_html(files, opt);<br>
<br>
&#9;if (opt.list_only)<br>
&#9;{<br>
&#9;&#9;print_list_with_sizes(files, opt);<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;std::uintmax_t total = dump_files_and_total(files, opt);<br>
<br>
&#9;if (after_dump)<br>
&#9;&#9;after_dump(total);<br>
<br>
&#9;return 0;<br>
}<br>
<br>
// =========================<br>
// CONFIG MODE<br>
// =========================<br>
<br>
static int run_config_mode(const Options &amp;opt)<br>
{<br>
&#9;Config cfg;<br>
&#9;try<br>
&#9;{<br>
&#9;&#9;cfg = parse_config(opt.config_file);<br>
&#9;}<br>
&#9;catch (const std::exception &amp;e)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
<br>
&#9;auto after_dump = [&amp;](std::uintmax_t total)<br>
&#9;{<br>
&#9;&#9;// TREE rules (если есть)<br>
&#9;&#9;if (!cfg.tree_rules.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;auto tree_files = collect_from_rules(cfg.tree_rules, opt);<br>
&#9;&#9;&#9;if (!tree_files.empty())<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cout &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;&#9;print_tree(tree_files);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (opt.chunk_trailer)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
&#9;&#9;&#9;print_chunk_help();<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&#9;};<br>
<br>
&#9;return process_files(text_files, opt, after_dump);<br>
}<br>
<br>
// =========================<br>
// NORMAL MODE<br>
// =========================<br>
<br>
static int run_normal_mode(const Options &amp;opt)<br>
{<br>
&#9;std::vector&lt;std::filesystem::path&gt; files =<br>
&#9;&#9;collect_from_paths(opt.paths, opt);<br>
<br>
&#9;auto after_dump = [&amp;](std::uintmax_t total)<br>
&#9;{<br>
&#9;&#9;// В обычном режиме дерево не печатаем<br>
&#9;&#9;if (opt.chunk_trailer)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
&#9;&#9;&#9;print_chunk_help();<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&#9;};<br>
<br>
&#9;return process_files(files, opt, after_dump);<br>
}<br>
<br>
static std::string substitute_rawmap(const std::string &amp;tmpl,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::string &amp;rawmap)<br>
{<br>
&#9;const std::string token = &quot;{RAWMAP}&quot;;<br>
&#9;if (tmpl.empty())<br>
&#9;&#9;return rawmap;<br>
<br>
&#9;std::string out;<br>
&#9;out.reserve(tmpl.size() + rawmap.size() + 16);<br>
<br>
&#9;std::size_t pos = 0;<br>
&#9;while (true)<br>
&#9;{<br>
&#9;&#9;std::size_t p = tmpl.find(token, pos);<br>
&#9;&#9;if (p == std::string::npos)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;out.append(tmpl, pos, std::string::npos);<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;}<br>
&#9;&#9;out.append(tmpl, pos, p - pos);<br>
&#9;&#9;out.append(rawmap);<br>
&#9;&#9;pos = p + token.size();<br>
&#9;}<br>
&#9;return out;<br>
}<br>
<br>
static int install_pre_commit_hook()<br>
{<br>
&#9;// Узнаём .git-директорию через git<br>
&#9;std::string git_dir = detect_git_dir();<br>
&#9;if (git_dir.empty())<br>
&#9;{<br>
&#9;&#9;std::cerr<br>
&#9;&#9;&#9;&lt;&lt; &quot;--hook-install: not a git repository or git not available\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;fs::path git_path = fs::path(git_dir);<br>
&#9;fs::path hooks_dir = git_path / &quot;hooks&quot;;<br>
&#9;std::error_code ec;<br>
&#9;fs::create_directories(hooks_dir, ec);<br>
<br>
&#9;fs::path hook_path = hooks_dir / &quot;pre-commit&quot;;<br>
<br>
&#9;std::cout &lt;&lt; &quot;===== .git/hooks/pre-commit =====\n&quot;;<br>
<br>
&#9;std::string existing;<br>
&#9;if (fs::exists(hook_path))<br>
&#9;{<br>
&#9;&#9;std::ifstream in(hook_path);<br>
&#9;&#9;if (in)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::ostringstream ss;<br>
&#9;&#9;&#9;ss &lt;&lt; in.rdbuf();<br>
&#9;&#9;&#9;existing = ss.str();<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;const std::string marker = &quot;# pre-commit hook for scat wrapping&quot;;<br>
<br>
&#9;// Если наш хук уже есть — ничего не делаем<br>
&#9;if (!existing.empty() &amp;&amp; existing.find(marker) != std::string::npos)<br>
&#9;{<br>
&#9;&#9;std::cout &lt;&lt; &quot;scat: pre-commit hook already contains scat wrapper\n&quot;;<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;if (existing.empty())<br>
&#9;{<br>
&#9;&#9;// Создаём новый pre-commit с твоим скриптом<br>
&#9;&#9;std::ofstream out(hook_path, std::ios::trunc);<br>
&#9;&#9;if (!out)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;scat: cannot write hook file: &quot; &lt;&lt; hook_path &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;return 1;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;out &lt;&lt; &quot;#!/bin/sh\n&quot;<br>
&#9;&#9;&#9;&quot;# pre-commit hook for scat wrapping\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;set -e  # если любая команда упадёт — прерываем коммит\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;# Опционально: не мешаемся, если scat не установлен\n&quot;<br>
&#9;&#9;&#9;&quot;if ! command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
&#9;&#9;&#9;&quot;    echo \&quot;pre-commit: 'scat' not found in PATH, skipping &quot;<br>
&#9;&#9;&#9;&quot;wrap\&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;    exit 0\n&quot;<br>
&#9;&#9;&#9;&quot;fi\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;# Рабочая директория хуков по умолчанию — корень репозитория,\n&quot;<br>
&#9;&#9;&#9;&quot;# так что можно просто дернуть scat.\n&quot;<br>
&#9;&#9;&#9;&quot;scat --wrap .scatwrap\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;# Добавляем всё из wrapped в индекс (новые, изменённые, &quot;<br>
&#9;&#9;&#9;&quot;удалённые)\n&quot;<br>
&#9;&#9;&#9;&quot;git add -A .scatwrap\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;exit 0\n&quot;;<br>
&#9;}<br>
&#9;else<br>
&#9;{<br>
&#9;&#9;// Уже есть какой-то хук — аккуратно добавляем наш блок в конец<br>
&#9;&#9;std::ofstream out(hook_path, std::ios::app);<br>
&#9;&#9;if (!out)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;scat: cannot append to hook file: &quot; &lt;&lt; hook_path<br>
&#9;&#9;&#9;&#9;&#9;&lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;return 1;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (!existing.empty() &amp;&amp; existing.back() != '\n')<br>
&#9;&#9;&#9;out &lt;&lt; &quot;\n&quot;;<br>
<br>
&#9;&#9;out &lt;&lt; &quot;\n# ----- added by scat --hook-install -----\n&quot;<br>
&#9;&#9;&#9;&quot;if command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
&#9;&#9;&#9;&quot;    echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;    scat --wrap .scatwrap\n&quot;<br>
&#9;&#9;&#9;&quot;    git add -A .scatwrap\n&quot;<br>
&#9;&#9;&#9;&quot;    echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
&#9;&#9;&#9;&quot;fi\n&quot;<br>
&#9;&#9;&#9;&quot;# ----- end scat hook -----\n&quot;;<br>
&#9;}<br>
<br>
#ifndef _WIN32<br>
&#9;// chmod +x на Unix-подобных<br>
&#9;std::string hp = hook_path.string();<br>
&#9;struct stat st;<br>
&#9;if (::stat(hp.c_str(), &amp;st) == 0)<br>
&#9;{<br>
&#9;&#9;mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;<br>
&#9;&#9;::chmod(hp.c_str(), mode);<br>
&#9;}<br>
#endif<br>
<br>
&#9;return 0;<br>
}<br>
<br>
int scat_main(int argc, char **argv)<br>
{<br>
&#9;Options opt = parse_options(argc, argv);<br>
&#9;g_use_absolute_paths = opt.abs_paths;<br>
&#9;CopyGuard copy_guard(opt.copy_out);<br>
<br>
&#9;// Установка git pre-commit hook'а и выход<br>
&#9;if (opt.hook_install)<br>
&#9;{<br>
&#9;&#9;return install_pre_commit_hook();<br>
&#9;}<br>
<br>
&#9;// Git info mode: print repository metadata and exit<br>
&#9;if (opt.git_info)<br>
&#9;{<br>
&#9;&#9;GitInfo info = detect_git_info();<br>
<br>
&#9;&#9;if (!info.has_commit &amp;&amp; !info.has_remote)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cout &lt;&lt; &quot;Git: not a repository or git is not available\n&quot;;<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (info.has_commit)<br>
&#9;&#9;&#9;&#9;std::cout &lt;&lt; &quot;Git commit: &quot; &lt;&lt; info.commit &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;if (info.has_remote)<br>
&#9;&#9;&#9;&#9;std::cout &lt;&lt; &quot;Git remote: &quot; &lt;&lt; info.remote &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;// GH map mode: построить prefix = raw.githubusercontent/... и дальше<br>
&#9;// работать как -l --prefix<br>
&#9;if (opt.gh_map)<br>
&#9;{<br>
&#9;&#9;GitHubInfo gh = detect_github_info();<br>
&#9;&#9;if (!gh.ok)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--ghmap: unable to detect GitHub remote/commit\n&quot;;<br>
&#9;&#9;&#9;return 1;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::string prefix = &quot;https://raw.githubusercontent.com/&quot; + gh.user +<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&quot;/&quot; + gh.repo + &quot;/&quot; + gh.commit + &quot;/.scatwrap/&quot;;<br>
<br>
&#9;&#9;&#9;if (!opt.config_file.empty())<br>
&#9;{<br>
&#9;&#9;Config cfg;<br>
&#9;&#9;try {<br>
&#9;&#9;&#9;cfg = parse_config(opt.config_file);<br>
&#9;&#9;} catch (const std::exception&amp; e) {<br>
&#9;&#9;&#9;std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;return 1;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
&#9;&#9;if (text_files.empty())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
&#9;&#9;&#9;return 0;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Собираем &quot;сырой&quot; список ссылок в строку — это и есть {RAWMAP}<br>
&#9;&#9;Options list_opt = opt;<br>
&#9;&#9;list_opt.path_prefix = prefix;<br>
<br>
&#9;&#9;std::ostringstream oss;<br>
&#9;&#9;print_list_with_sizes_to(text_files, list_opt, oss);<br>
&#9;&#9;std::string rawmap = oss.str();<br>
<br>
&#9;&#9;std::string output;<br>
&#9;&#9;if (!cfg.map_format.empty())<br>
&#9;&#9;&#9;output = substitute_rawmap(cfg.map_format, rawmap);<br>
&#9;&#9;else<br>
&#9;&#9;&#9;output = rawmap;<br>
<br>
&#9;&#9;std::cout &lt;&lt; output;<br>
&#9;&#9;return 0;<br>
&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.list_only = true;<br>
&#9;&#9;&#9;opt.wrap_root.clear();<br>
&#9;&#9;&#9;opt.path_prefix = prefix;<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;// HTTP server mode<br>
&#9;if (opt.server_port != 0)<br>
&#9;&#9;return run_server(opt);<br>
<br>
&#9;// ------------------------------------------------------------<br>
&#9;// Chunk help<br>
&#9;// ------------------------------------------------------------<br>
&#9;if (opt.chunk_help)<br>
&#9;{<br>
&#9;&#9;print_chunk_help();<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;// ------------------------------------------------------------<br>
&#9;// Apply patch mode<br>
&#9;// ------------------------------------------------------------<br>
&#9;if (!opt.apply_file.empty() || opt.apply_stdin)<br>
&#9;{<br>
&#9;&#9;if (opt.apply_stdin)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;namespace fs = std::filesystem;<br>
<br>
&#9;&#9;&#9;std::stringstream ss;<br>
&#9;&#9;&#9;ss &lt;&lt; std::cin.rdbuf();<br>
&#9;&#9;&#9;fs::path tmp = fs::temp_directory_path() / &quot;scat_stdin_patch.txt&quot;;<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::ofstream out(tmp);<br>
&#9;&#9;&#9;&#9;out &lt;&lt; ss.str();<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;std::string tmp_str = tmp.string();<br>
&#9;&#9;&#9;const char *args[] = {&quot;apply&quot;, tmp_str.c_str()};<br>
&#9;&#9;&#9;int r = apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
&#9;&#9;&#9;fs::remove(tmp);<br>
&#9;&#9;&#9;return r;<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::string file = opt.apply_file;<br>
&#9;&#9;&#9;const char *args[] = {&quot;apply&quot;, file.c_str()};<br>
&#9;&#9;&#9;return apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;// ------------------------------------------------------------<br>
&#9;// CONFIG MODE — uses scat.txt or --config F<br>
&#9;// ------------------------------------------------------------<br>
&#9;if (!opt.config_file.empty())<br>
&#9;&#9;return run_config_mode(opt);<br>
<br>
&#9;// ------------------------------------------------------------<br>
&#9;// NORMAL MODE — user provided paths<br>
&#9;// ------------------------------------------------------------<br>
&#9;return run_normal_mode(opt);<br>
}<br>
<br>
int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&#9;&#9;&#9;&#9;&#9;const Options &amp;opt)<br>
{<br>
&#9;namespace fs = std::filesystem;<br>
<br>
&#9;if (opt.wrap_root.empty())<br>
&#9;{<br>
&#9;&#9;return 0;<br>
&#9;}<br>
<br>
&#9;fs::path root = opt.wrap_root;<br>
<br>
&#9;std::error_code ec;<br>
&#9;fs::create_directories(root, ec);<br>
<br>
&#9;for (const auto &amp;f : files)<br>
&#9;{<br>
&#9;&#9;// считаем относительный путь относительно текущего каталога<br>
&#9;&#9;std::error_code rec;<br>
&#9;&#9;fs::path rel = fs::relative(f, fs::current_path(), rec);<br>
&#9;&#9;if (rec)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// если не получилось — хотя бы имя файла<br>
&#9;&#9;&#9;rel = f.filename();<br>
&#9;&#9;}<br>
<br>
&#9;&#9;fs::path dst = root / rel;<br>
&#9;&#9;fs::create_directories(dst.parent_path(), ec);<br>
<br>
&#9;&#9;std::ifstream in(f, std::ios::binary);<br>
&#9;&#9;if (!in)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;Cannot open for wrap: &quot; &lt;&lt; f &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::ostringstream ss;<br>
&#9;&#9;ss &lt;&lt; in.rdbuf();<br>
<br>
&#9;&#9;std::string title = rel.generic_string();<br>
&#9;&#9;std::string html = wrap_cpp_as_html(ss.str(), title);<br>
<br>
&#9;&#9;std::ofstream out(dst, std::ios::binary);<br>
&#9;&#9;if (!out)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;Cannot write wrapped file: &quot; &lt;&lt; dst &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;out &lt;&lt; html;<br>
&#9;}<br>
<br>
&#9;return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

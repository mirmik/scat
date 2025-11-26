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
&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt);<br>
<br>
namespace fs = std::filesystem;<br>
<br>
bool g_use_absolute_paths = false;<br>
<br>
int apply_chunk_main(int argc, char **argv);<br>
<br>
void print_tree(const std::vector&lt;std::filesystem::path&gt; &amp;files)<br>
{<br>
&emsp;// Собираем относительные пути<br>
&emsp;std::vector&lt;std::string&gt; rels;<br>
&emsp;rels.reserve(files.size());<br>
&emsp;for (auto &amp;p : files)<br>
&emsp;&emsp;rels.push_back(make_display_path(p));<br>
<br>
&emsp;std::sort(rels.begin(), rels.end());<br>
<br>
&emsp;struct Node<br>
&emsp;{<br>
&emsp;&emsp;std::map&lt;std::string, Node *&gt; children;<br>
&emsp;&emsp;bool is_file = false;<br>
&emsp;};<br>
<br>
&emsp;Node root;<br>
<br>
&emsp;// ----------------------------<br>
&emsp;//  Построение дерева<br>
&emsp;// ----------------------------<br>
&emsp;for (auto &amp;r : rels)<br>
&emsp;{<br>
&emsp;&emsp;fs::path p = r;<br>
&emsp;&emsp;Node *cur = &amp;root;<br>
<br>
&emsp;&emsp;// вытаскиваем компоненты p в список<br>
&emsp;&emsp;std::vector&lt;std::string&gt; parts;<br>
&emsp;&emsp;for (auto &amp;part : p)<br>
&emsp;&emsp;&emsp;parts.push_back(part.string());<br>
<br>
&emsp;&emsp;int total = (int)parts.size();<br>
<br>
&emsp;&emsp;for (int i = 0; i &lt; total; ++i)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;name = parts[i];<br>
&emsp;&emsp;&emsp;bool last = (i == total - 1);<br>
<br>
&emsp;&emsp;&emsp;if (!cur-&gt;children.count(name))<br>
&emsp;&emsp;&emsp;&emsp;cur-&gt;children[name] = new Node();<br>
<br>
&emsp;&emsp;&emsp;cur = cur-&gt;children[name];<br>
&emsp;&emsp;&emsp;if (last)<br>
&emsp;&emsp;&emsp;&emsp;cur-&gt;is_file = true;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;// ----------------------------<br>
&emsp;//  Рекурсивная печать<br>
&emsp;// ----------------------------<br>
&emsp;std::function&lt;void(Node *, const std::string &amp;, bool, const std::string &amp;)&gt;<br>
&emsp;&emsp;go;<br>
<br>
&emsp;go = [&amp;](Node *node,<br>
&emsp;&emsp;&emsp;const std::string &amp;name,<br>
&emsp;&emsp;&emsp;bool last,<br>
&emsp;&emsp;&emsp;const std::string &amp;prefix)<br>
&emsp;{<br>
&emsp;&emsp;if (!name.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; prefix &lt;&lt; (last ? &quot;└── &quot; : &quot;├── &quot;) &lt;&lt; name;<br>
<br>
&emsp;&emsp;&emsp;if (!node-&gt;is_file)<br>
&emsp;&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;/&quot;;<br>
<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// сортируем детей по имени<br>
&emsp;&emsp;std::vector&lt;std::string&gt; keys;<br>
&emsp;&emsp;keys.reserve(node-&gt;children.size());<br>
&emsp;&emsp;for (auto &amp;[k, _] : node-&gt;children)<br>
&emsp;&emsp;&emsp;keys.push_back(k);<br>
&emsp;&emsp;std::sort(keys.begin(), keys.end());<br>
<br>
&emsp;&emsp;for (size_t i = 0; i &lt; keys.size(); ++i)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;bool is_last = (i + 1 == keys.size());<br>
&emsp;&emsp;&emsp;Node *child = node-&gt;children[keys[i]];<br>
<br>
&emsp;&emsp;&emsp;go(child,<br>
&emsp;&emsp;&emsp;keys[i],<br>
&emsp;&emsp;&emsp;is_last,<br>
&emsp;&emsp;&emsp;prefix + (name.empty() ? &quot;&quot; : (last ? &quot;    &quot; : &quot;│   &quot;)));<br>
&emsp;&emsp;}<br>
&emsp;};<br>
<br>
&emsp;std::cout &lt;&lt; &quot;===== PROJECT TREE =====\n&quot;;<br>
&emsp;go(&amp;root, &quot;&quot;, true, &quot;&quot;);<br>
&emsp;std::cout &lt;&lt; &quot;========================\n\n&quot;;<br>
}<br>
<br>
static void<br>
print_list_with_sizes_to(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::ostream &amp;os)<br>
{<br>
&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;struct Item<br>
&emsp;{<br>
&emsp;&emsp;fs::path path;<br>
&emsp;&emsp;std::string display;<br>
&emsp;&emsp;std::uintmax_t size;<br>
&emsp;};<br>
<br>
&emsp;std::vector&lt;Item&gt; items;<br>
&emsp;items.reserve(files.size());<br>
&emsp;std::uintmax_t total = 0;<br>
&emsp;std::size_t max_len = 0;<br>
<br>
&emsp;for (auto &amp;f : files)<br>
&emsp;{<br>
&emsp;&emsp;auto disp = make_display_path(f);<br>
&emsp;&emsp;auto sz = get_file_size(f);<br>
<br>
&emsp;&emsp;total += sz;<br>
<br>
&emsp;&emsp;std::size_t shown_len = opt.path_prefix.size() + disp.size();<br>
&emsp;&emsp;if (shown_len &gt; max_len)<br>
&emsp;&emsp;&emsp;max_len = shown_len;<br>
<br>
&emsp;&emsp;items.push_back(Item{f, std::move(disp), sz});<br>
&emsp;}<br>
<br>
&emsp;if (opt.sorted)<br>
&emsp;{<br>
&emsp;&emsp;std::sort(items.begin(),<br>
&emsp;&emsp;&emsp;&emsp;items.end(),<br>
&emsp;&emsp;&emsp;&emsp;[](const Item &amp;a, const Item &amp;b)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (a.size != b.size)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;return a.size &gt; b.size; // по убыванию<br>
&emsp;&emsp;&emsp;&emsp;&emsp;return a.display &lt; b.display;<br>
&emsp;&emsp;&emsp;&emsp;});<br>
&emsp;}<br>
<br>
&emsp;const bool show_size = opt.show_size;<br>
&emsp;for (const auto &amp;it : items)<br>
&emsp;{<br>
&emsp;&emsp;std::string shown = opt.path_prefix + it.display;<br>
&emsp;&emsp;os &lt;&lt; shown;<br>
<br>
&emsp;&emsp;if (show_size)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (max_len &gt; shown.size())<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::size_t pad = max_len - shown.size();<br>
&emsp;&emsp;&emsp;&emsp;for (std::size_t i = 0; i &lt; pad; ++i)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;os &lt;&lt; ' ';<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;os &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)&quot;;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;os &lt;&lt; &quot;\n&quot;;<br>
&emsp;}<br>
<br>
&emsp;if (show_size)<br>
&emsp;{<br>
&emsp;&emsp;os &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&emsp;}<br>
}<br>
<br>
// старая функция теперь просто обёртка вокруг новой<br>
static void<br>
print_list_with_sizes(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt)<br>
{<br>
&emsp;print_list_with_sizes_to(files, opt, std::cout);<br>
}<br>
<br>
// Вывод всех файлов и подсчёт суммарного размера<br>
static std::uintmax_t<br>
dump_files_and_total(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt)<br>
{<br>
&emsp;std::uintmax_t total = 0;<br>
&emsp;bool first = true;<br>
<br>
&emsp;for (auto &amp;f : files)<br>
&emsp;{<br>
&emsp;&emsp;dump_file(f, first, opt);<br>
&emsp;&emsp;first = false;<br>
&emsp;&emsp;total += get_file_size(f);<br>
&emsp;}<br>
<br>
&emsp;return total;<br>
}<br>
<br>
int run_server(const Options &amp;opt);<br>
<br>
// общий каркас обработки списка файлов<br>
static int process_files(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::function&lt;void(std::uintmax_t)&gt; &amp;after_dump)<br>
{<br>
&emsp;if (files.empty())<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;if (!opt.wrap_root.empty())<br>
&emsp;&emsp;return wrap_files_to_html(files, opt);<br>
<br>
&emsp;if (opt.list_only)<br>
&emsp;{<br>
&emsp;&emsp;print_list_with_sizes(files, opt);<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;std::uintmax_t total = dump_files_and_total(files, opt);<br>
<br>
&emsp;if (after_dump)<br>
&emsp;&emsp;after_dump(total);<br>
<br>
&emsp;return 0;<br>
}<br>
<br>
// =========================<br>
// CONFIG MODE<br>
// =========================<br>
<br>
static int run_config_mode(const Options &amp;opt)<br>
{<br>
&emsp;Config cfg;<br>
&emsp;try<br>
&emsp;{<br>
&emsp;&emsp;cfg = parse_config(opt.config_file);<br>
&emsp;}<br>
&emsp;catch (const std::exception &amp;e)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
<br>
&emsp;auto after_dump = [&amp;](std::uintmax_t total)<br>
&emsp;{<br>
&emsp;&emsp;// TREE rules (если есть)<br>
&emsp;&emsp;if (!cfg.tree_rules.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;auto tree_files = collect_from_rules(cfg.tree_rules, opt);<br>
&emsp;&emsp;&emsp;if (!tree_files.empty())<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;print_tree(tree_files);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (opt.chunk_trailer)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
&emsp;&emsp;&emsp;print_chunk_help();<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&emsp;};<br>
<br>
&emsp;return process_files(text_files, opt, after_dump);<br>
}<br>
<br>
// =========================<br>
// NORMAL MODE<br>
// =========================<br>
<br>
static int run_normal_mode(const Options &amp;opt)<br>
{<br>
&emsp;std::vector&lt;std::filesystem::path&gt; files =<br>
&emsp;&emsp;collect_from_paths(opt.paths, opt);<br>
<br>
&emsp;auto after_dump = [&amp;](std::uintmax_t total)<br>
&emsp;{<br>
&emsp;&emsp;// В обычном режиме дерево не печатаем<br>
&emsp;&emsp;if (opt.chunk_trailer)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;<br>
&emsp;&emsp;&emsp;print_chunk_help();<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;<br>
&emsp;};<br>
<br>
&emsp;return process_files(files, opt, after_dump);<br>
}<br>
<br>
static std::string substitute_rawmap(const std::string &amp;tmpl,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;rawmap)<br>
{<br>
&emsp;const std::string token = &quot;{RAWMAP}&quot;;<br>
&emsp;if (tmpl.empty())<br>
&emsp;&emsp;return rawmap;<br>
<br>
&emsp;std::string out;<br>
&emsp;out.reserve(tmpl.size() + rawmap.size() + 16);<br>
<br>
&emsp;std::size_t pos = 0;<br>
&emsp;while (true)<br>
&emsp;{<br>
&emsp;&emsp;std::size_t p = tmpl.find(token, pos);<br>
&emsp;&emsp;if (p == std::string::npos)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;out.append(tmpl, pos, std::string::npos);<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;out.append(tmpl, pos, p - pos);<br>
&emsp;&emsp;out.append(rawmap);<br>
&emsp;&emsp;pos = p + token.size();<br>
&emsp;}<br>
&emsp;return out;<br>
}<br>
<br>
static int install_pre_commit_hook()<br>
{<br>
&emsp;// Узнаём .git-директорию через git<br>
&emsp;std::string git_dir = detect_git_dir();<br>
&emsp;if (git_dir.empty())<br>
&emsp;{<br>
&emsp;&emsp;std::cerr<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot;--hook-install: not a git repository or git not available\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;fs::path git_path = fs::path(git_dir);<br>
&emsp;fs::path hooks_dir = git_path / &quot;hooks&quot;;<br>
&emsp;std::error_code ec;<br>
&emsp;fs::create_directories(hooks_dir, ec);<br>
<br>
&emsp;fs::path hook_path = hooks_dir / &quot;pre-commit&quot;;<br>
<br>
&emsp;std::cout &lt;&lt; &quot;===== .git/hooks/pre-commit =====\n&quot;;<br>
<br>
&emsp;std::string existing;<br>
&emsp;if (fs::exists(hook_path))<br>
&emsp;{<br>
&emsp;&emsp;std::ifstream in(hook_path);<br>
&emsp;&emsp;if (in)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::ostringstream ss;<br>
&emsp;&emsp;&emsp;ss &lt;&lt; in.rdbuf();<br>
&emsp;&emsp;&emsp;existing = ss.str();<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;const std::string marker = &quot;# pre-commit hook for scat wrapping&quot;;<br>
<br>
&emsp;// Если наш хук уже есть — ничего не делаем<br>
&emsp;if (!existing.empty() &amp;&amp; existing.find(marker) != std::string::npos)<br>
&emsp;{<br>
&emsp;&emsp;std::cout &lt;&lt; &quot;scat: pre-commit hook already contains scat wrapper\n&quot;;<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;if (existing.empty())<br>
&emsp;{<br>
&emsp;&emsp;// Создаём новый pre-commit с твоим скриптом<br>
&emsp;&emsp;std::ofstream out(hook_path, std::ios::trunc);<br>
&emsp;&emsp;if (!out)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;scat: cannot write hook file: &quot; &lt;&lt; hook_path &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;return 1;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;out &lt;&lt; &quot;#!/bin/sh\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# pre-commit hook for scat wrapping\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;set -e  # если любая команда упадёт — прерываем коммит\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# Опционально: не мешаемся, если scat не установлен\n&quot;<br>
&emsp;&emsp;&emsp;&quot;if ! command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    echo \&quot;pre-commit: 'scat' not found in PATH, skipping &quot;<br>
&emsp;&emsp;&emsp;&quot;wrap\&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    exit 0\n&quot;<br>
&emsp;&emsp;&emsp;&quot;fi\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# Рабочая директория хуков по умолчанию — корень репозитория,\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# так что можно просто дернуть scat.\n&quot;<br>
&emsp;&emsp;&emsp;&quot;scat --wrap .scatwrap\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# Добавляем всё из wrapped в индекс (новые, изменённые, &quot;<br>
&emsp;&emsp;&emsp;&quot;удалённые)\n&quot;<br>
&emsp;&emsp;&emsp;&quot;git add -A .scatwrap\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;exit 0\n&quot;;<br>
&emsp;}<br>
&emsp;else<br>
&emsp;{<br>
&emsp;&emsp;// Уже есть какой-то хук — аккуратно добавляем наш блок в конец<br>
&emsp;&emsp;std::ofstream out(hook_path, std::ios::app);<br>
&emsp;&emsp;if (!out)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;scat: cannot append to hook file: &quot; &lt;&lt; hook_path<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;return 1;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (!existing.empty() &amp;&amp; existing.back() != '\n')<br>
&emsp;&emsp;&emsp;out &lt;&lt; &quot;\n&quot;;<br>
<br>
&emsp;&emsp;out &lt;&lt; &quot;\n# ----- added by scat --hook-install -----\n&quot;<br>
&emsp;&emsp;&emsp;&quot;if command -v scat &gt;/dev/null 2&gt;&amp;1; then\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    echo \&quot;pre-commit: running 'scat --wrap wrapped'...\&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    scat --wrap .scatwrap\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    git add -A .scatwrap\n&quot;<br>
&emsp;&emsp;&emsp;&quot;    echo \&quot;pre-commit: wrapped/ updated and added to commit\&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&quot;fi\n&quot;<br>
&emsp;&emsp;&emsp;&quot;# ----- end scat hook -----\n&quot;;<br>
&emsp;}<br>
<br>
#ifndef _WIN32<br>
&emsp;// chmod +x на Unix-подобных<br>
&emsp;std::string hp = hook_path.string();<br>
&emsp;struct stat st;<br>
&emsp;if (::stat(hp.c_str(), &amp;st) == 0)<br>
&emsp;{<br>
&emsp;&emsp;mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;<br>
&emsp;&emsp;::chmod(hp.c_str(), mode);<br>
&emsp;}<br>
#endif<br>
<br>
&emsp;return 0;<br>
}<br>
<br>
int scat_main(int argc, char **argv)<br>
{<br>
&emsp;Options opt = parse_options(argc, argv);<br>
&emsp;g_use_absolute_paths = opt.abs_paths;<br>
&emsp;CopyGuard copy_guard(opt.copy_out);<br>
<br>
&emsp;// Установка git pre-commit hook'а и выход<br>
&emsp;if (opt.hook_install)<br>
&emsp;{<br>
&emsp;&emsp;return install_pre_commit_hook();<br>
&emsp;}<br>
<br>
&emsp;// Git info mode: print repository metadata and exit<br>
&emsp;if (opt.git_info)<br>
&emsp;{<br>
&emsp;&emsp;GitInfo info = detect_git_info();<br>
<br>
&emsp;&emsp;if (!info.has_commit &amp;&amp; !info.has_remote)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;Git: not a repository or git is not available\n&quot;;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (info.has_commit)<br>
&emsp;&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;Git commit: &quot; &lt;&lt; info.commit &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;if (info.has_remote)<br>
&emsp;&emsp;&emsp;&emsp;std::cout &lt;&lt; &quot;Git remote: &quot; &lt;&lt; info.remote &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;// GH map mode: построить prefix = raw.githubusercontent/... и дальше<br>
&emsp;// работать как -l --prefix<br>
&emsp;if (opt.gh_map)<br>
&emsp;{<br>
&emsp;&emsp;GitHubInfo gh = detect_github_info();<br>
&emsp;&emsp;if (!gh.ok)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--ghmap: unable to detect GitHub remote/commit\n&quot;;<br>
&emsp;&emsp;&emsp;return 1;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::string prefix = &quot;https://raw.githubusercontent.com/&quot; + gh.user +<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;/&quot; + gh.repo + &quot;/&quot; + gh.commit + &quot;/.scatwrap/&quot;;<br>
<br>
&emsp;&emsp;&emsp;if (!opt.config_file.empty())<br>
&emsp;{<br>
&emsp;&emsp;Config cfg;<br>
&emsp;&emsp;try {<br>
&emsp;&emsp;&emsp;cfg = parse_config(opt.config_file);<br>
&emsp;&emsp;} catch (const std::exception&amp; e) {<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;return 1;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;auto text_files = collect_from_rules(cfg.text_rules, opt);<br>
&emsp;&emsp;if (text_files.empty())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;No files collected.\n&quot;;<br>
&emsp;&emsp;&emsp;return 0;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Собираем &quot;сырой&quot; список ссылок в строку — это и есть {RAWMAP}<br>
&emsp;&emsp;Options list_opt = opt;<br>
&emsp;&emsp;list_opt.path_prefix = prefix;<br>
<br>
&emsp;&emsp;std::ostringstream oss;<br>
&emsp;&emsp;print_list_with_sizes_to(text_files, list_opt, oss);<br>
&emsp;&emsp;std::string rawmap = oss.str();<br>
<br>
&emsp;&emsp;std::string output;<br>
&emsp;&emsp;if (!cfg.map_format.empty())<br>
&emsp;&emsp;&emsp;output = substitute_rawmap(cfg.map_format, rawmap);<br>
&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;output = rawmap;<br>
<br>
&emsp;&emsp;std::cout &lt;&lt; output;<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.list_only = true;<br>
&emsp;&emsp;&emsp;opt.wrap_root.clear();<br>
&emsp;&emsp;&emsp;opt.path_prefix = prefix;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;// HTTP server mode<br>
&emsp;if (opt.server_port != 0)<br>
&emsp;&emsp;return run_server(opt);<br>
<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;// Chunk help<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;if (opt.chunk_help)<br>
&emsp;{<br>
&emsp;&emsp;print_chunk_help();<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;// Apply patch mode<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;if (!opt.apply_file.empty() || opt.apply_stdin)<br>
&emsp;{<br>
&emsp;&emsp;if (opt.apply_stdin)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;&emsp;&emsp;std::stringstream ss;<br>
&emsp;&emsp;&emsp;ss &lt;&lt; std::cin.rdbuf();<br>
&emsp;&emsp;&emsp;fs::path tmp = fs::temp_directory_path() / &quot;scat_stdin_patch.txt&quot;;<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::ofstream out(tmp);<br>
&emsp;&emsp;&emsp;&emsp;out &lt;&lt; ss.str();<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;std::string tmp_str = tmp.string();<br>
&emsp;&emsp;&emsp;const char *args[] = {&quot;apply&quot;, tmp_str.c_str()};<br>
&emsp;&emsp;&emsp;int r = apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
&emsp;&emsp;&emsp;fs::remove(tmp);<br>
&emsp;&emsp;&emsp;return r;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::string file = opt.apply_file;<br>
&emsp;&emsp;&emsp;const char *args[] = {&quot;apply&quot;, file.c_str()};<br>
&emsp;&emsp;&emsp;return apply_chunk_main(2, const_cast&lt;char **&gt;(args));<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;// CONFIG MODE — uses scat.txt or --config F<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;if (!opt.config_file.empty())<br>
&emsp;&emsp;return run_config_mode(opt);<br>
<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;// NORMAL MODE — user provided paths<br>
&emsp;// ------------------------------------------------------------<br>
&emsp;return run_normal_mode(opt);<br>
}<br>
<br>
int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt; &amp;files,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt)<br>
{<br>
&emsp;namespace fs = std::filesystem;<br>
<br>
&emsp;if (opt.wrap_root.empty())<br>
&emsp;{<br>
&emsp;&emsp;return 0;<br>
&emsp;}<br>
<br>
&emsp;fs::path root = opt.wrap_root;<br>
<br>
&emsp;std::error_code ec;<br>
&emsp;fs::create_directories(root, ec);<br>
<br>
&emsp;for (const auto &amp;f : files)<br>
&emsp;{<br>
&emsp;&emsp;// считаем относительный путь относительно текущего каталога<br>
&emsp;&emsp;std::error_code rec;<br>
&emsp;&emsp;fs::path rel = fs::relative(f, fs::current_path(), rec);<br>
&emsp;&emsp;if (rec)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// если не получилось — хотя бы имя файла<br>
&emsp;&emsp;&emsp;rel = f.filename();<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;fs::path dst = root / rel;<br>
&emsp;&emsp;fs::create_directories(dst.parent_path(), ec);<br>
<br>
&emsp;&emsp;std::ifstream in(f, std::ios::binary);<br>
&emsp;&emsp;if (!in)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;Cannot open for wrap: &quot; &lt;&lt; f &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::ostringstream ss;<br>
&emsp;&emsp;ss &lt;&lt; in.rdbuf();<br>
<br>
&emsp;&emsp;std::string title = rel.generic_string();<br>
&emsp;&emsp;std::string html = wrap_cpp_as_html(ss.str(), title);<br>
<br>
&emsp;&emsp;std::ofstream out(dst, std::ios::binary);<br>
&emsp;&emsp;if (!out)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;Cannot write wrapped file: &quot; &lt;&lt; dst &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;out &lt;&lt; html;<br>
&emsp;}<br>
<br>
&emsp;return 0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

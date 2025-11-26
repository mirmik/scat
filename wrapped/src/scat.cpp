<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/scat.cpp</title>
</head>
<body>
<pre><code>
#include &quot;scat.h&quot;
#include &quot;collector.h&quot;
#include &quot;options.h&quot;
#include &quot;parser.h&quot;
#include &quot;rules.h&quot;
#include &quot;util.h&quot;
#include &lt;filesystem&gt;
#include &lt;fstream&gt;
#include &lt;functional&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;set&gt;
#include &lt;sstream&gt;
#include &lt;exception&gt;

int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt;&amp; files,
                              const Options&amp; opt);


namespace fs = std::filesystem;

bool g_use_absolute_paths = false;

void print_chunk_help()
{
    std::cout &lt;&lt; &quot;# Chunk v2 — Change Description Format\n&quot;
                 &quot;\n&quot;
                 &quot;Chunk v2 is a plain-text format for describing modifications to source files.\n&quot;
                 &quot;A patch consists of multiple sections, each describing a single operation:\n&quot;
                 &quot;line-based edits, text-based edits, or file-level operations.\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 1. Section structure\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: &lt;path&gt; ===\n&quot;
                 &quot;&lt;command&gt;\n&quot;
                 &quot;&lt;content...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;* &lt;path&gt; — relative file path\n&quot;
                 &quot;* Empty lines between sections are allowed\n&quot;
                 &quot;* Exactly one command per section\n&quot;
                 &quot;* &lt;content&gt; may contain any lines, including empty ones\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 3. Commands (text-based)\n&quot;
                 &quot;\n&quot;
                 &quot;Two formats are supported for text-based commands:\n&quot;
                 &quot;\n&quot;
                 &quot;### 3.1 Legacy format (simple)\n&quot;
                 &quot;These commands match an exact multi-line marker in the file.\n&quot;
                 &quot;Everything before the first `---` is the marker; everything after it is content.\n&quot;
                 &quot;\n&quot;
                 &quot;### Insert after marker\n&quot;
                 &quot;--- insert-after-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;inserted lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Insert before marker\n&quot;
                 &quot;--- insert-before-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;inserted lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Replace marker\n&quot;
                 &quot;--- replace-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;new lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Delete marker\n&quot;
                 &quot;--- delete-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;### 3.2 YAML strict format\n&quot;
                 &quot;\n&quot;
                 &quot;In YAML mode you can also specify strict context around the marker:\n&quot;
                 &quot;\n&quot;
                 &quot;--- replace-text\n&quot;
                 &quot;BEFORE:\n&quot;
                 &quot;  &lt;lines that must appear immediately above the marker&gt;\n&quot;
                 &quot;MARKER:\n&quot;
                 &quot;  &lt;marker lines&gt;\n&quot;
                 &quot;AFTER:\n&quot;
                 &quot;  &lt;lines that must appear immediately below the marker&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;payload lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;Rules:\n&quot;
                 &quot;* YAML mode is enabled only when the first non-empty line after the command\n&quot;
                 &quot;  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n&quot;
                 &quot;* Matching is strict: BEFORE lines must be directly above the marker block;\n&quot;
                 &quot;  AFTER lines must be directly below it.\n&quot;
                 &quot;* Whitespace differences are ignored (lines are trimmed before comparison).\n&quot;
                 &quot;* If BEFORE/AFTER are present, there is no fallback to the first occurrence\n&quot;
                 &quot;  of the marker.\n&quot;
                 &quot;* If more than one place matches the strict context, the patch fails as\n&quot;
                 &quot;  ambiguous.\n&quot;
                 &quot;* If no place matches the strict context, the patch fails with\n&quot;
                 &quot;  \&quot;strict marker context not found\&quot;.\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 4. File-level commands\n&quot;
                 &quot;These operations work on the whole file rather than its contents.\n&quot;
                 &quot;\n&quot;
                 &quot;### Create or overwrite file\n&quot;
                 &quot;--- create-file\n&quot;
                 &quot;&lt;file content...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Delete file\n&quot;
                 &quot;--- delete-file\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 5. Examples\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/a.cpp ===\n&quot;
                 &quot;--- replace 3:4\n&quot;
                 &quot;int value = 42;\n&quot;
                 &quot;std::cout &lt;&lt; value &lt;&lt; \&quot;\\\\n\&quot;;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/b.cpp ===\n&quot;
                 &quot;--- insert-after 12\n&quot;
                 &quot;log_debug(\&quot;checkpoint reached\&quot;);\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/c.cpp ===\n&quot;
                 &quot;--- delete 20:25\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: assets/config.json ===\n&quot;
                 &quot;--- create-file\n&quot;
                 &quot;{ \&quot;version\&quot;: 1 }\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: old/temp.txt ===\n&quot;
                 &quot;--- delete-file\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## Recommended usage\n&quot;
                 &quot;* Prefer text-based commands (`*-text`) — they are more stable when code moves.\n&quot;
                 &quot;* Use file-level commands when creating or removing entire files.\n&quot;
                 &quot;* Group modifications to multiple files into **one patch file**.\n&quot;
                 &quot;\n&quot;
                 &quot;*This cheat sheet is in the text for a reason. If you're asked to write a patch,\n&quot;
                 &quot; use the following format: chunk_v2.\n&quot;
                 &quot;*Try to strictly follow the rules described in this document, without making any\n&quot;
                 &quot; syntactic errors.\n&quot;
                 &quot;*When working with chunks, be careful that commands do not reference the same\n&quot;
                 &quot; text macros. Macros should never overlap.\n&quot;;
}

int apply_chunk_main(int argc, char** argv);

void print_tree(const std::vector&lt;std::filesystem::path&gt;&amp; files)
{
    // Собираем относительные пути
    std::vector&lt;std::string&gt; rels;
    rels.reserve(files.size());
    for (auto&amp; p : files)
        rels.push_back(make_display_path(p));

    std::sort(rels.begin(), rels.end());

    struct Node
    {
        std::map&lt;std::string, Node*&gt; children;
        bool is_file = false;
    };

    Node root;

    // ----------------------------
    //  Построение дерева
    // ----------------------------
    for (auto&amp; r : rels)
    {
        fs::path p = r;
        Node* cur = &amp;root;

        // вытаскиваем компоненты p в список
        std::vector&lt;std::string&gt; parts;
        for (auto&amp; part : p)
            parts.push_back(part.string());

        int total = (int)parts.size();

        for (int i = 0; i &lt; total; ++i)
        {
            const std::string&amp; name = parts[i];
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
    std::function&lt;void(Node*, const std::string&amp;, bool, const std::string&amp;)&gt; go;

    go = [&amp;](Node* node, const std::string&amp; name, bool last, const std::string&amp; prefix) {
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
        for (auto&amp; [k, _] : node-&gt;children)
            keys.push_back(k);
        std::sort(keys.begin(), keys.end());

        for (size_t i = 0; i &lt; keys.size(); ++i)
        {
            bool is_last = (i + 1 == keys.size());
            Node* child = node-&gt;children[keys[i]];

            go(child, keys[i], is_last, prefix + (name.empty() ? &quot;&quot; : (last ? &quot;    &quot; : &quot;│   &quot;)));
        }
    };

    std::cout &lt;&lt; &quot;===== PROJECT TREE =====\n&quot;;
    go(&amp;root, &quot;&quot;, true, &quot;&quot;);
    std::cout &lt;&lt; &quot;========================\n\n&quot;;
}

int run_server(const Options&amp; opt);

int scat_main(int argc, char** argv)
{
    Options opt = parse_options(argc, argv);
    g_use_absolute_paths = opt.abs_paths;

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
            const char* args[] = {&quot;apply&quot;, tmp_str.c_str()};
            int r = apply_chunk_main(2, const_cast&lt;char**&gt;(args));

            fs::remove(tmp);
            return r;
        }
        else
        {
            std::string file = opt.apply_file;
            const char* args[] = {&quot;apply&quot;, file.c_str()};
            return apply_chunk_main(2, const_cast&lt;char**&gt;(args));
        }
    }

    // ------------------------------------------------------------
    // CONFIG MODE — uses scat.txt or --config F
    // ------------------------------------------------------------
    if (!opt.config_file.empty())
    {
        Config cfg;
        try
        {
            cfg = parse_config(opt.config_file);
        }
        catch (const std::exception&amp; e)
        {
            std::cerr &lt;&lt; e.what() &lt;&lt; &quot;\n&quot;;
            return 1;
        }

        // 1) TEXT rules
        auto text_files = collect_from_rules(cfg.text_rules, opt);

        if (text_files.empty())
        {
            std::cerr &lt;&lt; &quot;No files collected.\n&quot;;
            return 0;
        }

            if (!opt.wrap_root.empty()) {
        // вместо вывода в stdout просто делаем HTML-копии
        return wrap_files_to_html(text_files, opt);
    }

        // LIST ONLY в config-режиме: только пути и размеры
        if (opt.list_only)
        {
            struct Item
            {
                fs::path path;
                std::string display;
                std::uintmax_t size;
            };

            std::vector&lt;Item&gt; items;
            items.reserve(text_files.size());

            std::uintmax_t total = 0;
            std::size_t max_name = 0;

            for (auto&amp; f : text_files)
            {
                auto disp = make_display_path(f);
                auto sz = get_file_size(f);

                total += sz;

                std::size_t shown_len = opt.path_prefix.size() + disp.size();
                if (shown_len &gt; max_name)
                    max_name = shown_len;

                items.push_back(Item{f, disp, sz});
            }

            if (opt.sorted)
            {
                std::sort(items.begin(), items.end(), [](const Item&amp; a, const Item&amp; b) {
                    if (a.size != b.size)
                        return a.size &gt; b.size; // по убыванию
                    return a.display &lt; b.display;
                });
            }

            for (const auto&amp; it : items)
            {
                std::string shown = opt.path_prefix + it.display;

                std::cout &lt;&lt; shown;
                if (max_name &gt; shown.size())
                {
                    std::size_t pad = max_name - shown.size();
                    for (std::size_t i = 0; i &lt; pad; ++i)
                        std::cout &lt;&lt; ' ';
                }
                std::cout &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)\n&quot;;
            }

            std::cout &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;
            return 0;
        }


        // 2) Печать файлов с подсчётом суммарного размера
        bool first = true;
        std::uintmax_t total = 0;
        for (auto&amp; f : text_files)
        {
            dump_file(f, first, opt);
            first = false;
            total += get_file_size(f);
        }

        // 3) TREE rules (optional)
        if (!cfg.tree_rules.empty())
        {
            auto tree_files = collect_from_rules(cfg.tree_rules, opt);
            if (!tree_files.empty())
            {
                std::cout &lt;&lt; &quot;\n&quot;;
                print_tree(tree_files); // выводим дерево строго в самом конце
            }
        }

        // 4) Chunk trailer, if needed
        if (opt.chunk_trailer)
        {
            std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;
            print_chunk_help();
        }

        std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;

        return 0;
    }

    // ------------------------------------------------------------
    // NORMAL MODE — user provided paths, not using a config file
    // ------------------------------------------------------------
    std::vector&lt;std::filesystem::path&gt; files = collect_from_paths(opt.paths, opt);

    if (files.empty())
    {
        std::cerr &lt;&lt; &quot;No files collected.\n&quot;;
        return 0;
    }

    if (!opt.wrap_root.empty()) {
    return wrap_files_to_html(files, opt);
}

    // LIST ONLY
    if (opt.list_only)
    {
        struct Item
        {
            fs::path path;
            std::string display;
            std::uintmax_t size;
        };

        std::vector&lt;Item&gt; items;
        items.reserve(files.size());

        std::uintmax_t total = 0;
        std::size_t max_name = 0;

        for (auto&amp; f : files)
        {
            auto disp = make_display_path(f);
            auto sz = get_file_size(f);

            total += sz;

            std::size_t shown_len = opt.path_prefix.size() + disp.size();
            if (shown_len &gt; max_name)
                max_name = shown_len;

            items.push_back(Item{f, disp, sz});
        }

        if (opt.sorted)
        {
            std::sort(items.begin(), items.end(), [](const Item&amp; a, const Item&amp; b) {
                if (a.size != b.size)
                    return a.size &gt; b.size; // по убыванию
                return a.display &lt; b.display;
            });
        }

        for (const auto&amp; it : items)
        {
            std::string shown = opt.path_prefix + it.display;

            std::cout &lt;&lt; shown;
            if (max_name &gt; shown.size())
            {
                std::size_t pad = max_name - shown.size();
                for (std::size_t i = 0; i &lt; pad; ++i)
                    std::cout &lt;&lt; ' ';
            }
            std::cout &lt;&lt; &quot; (&quot; &lt;&lt; it.size &lt;&lt; &quot; bytes)\n&quot;;
        }

        std::cout &lt;&lt; &quot;Total size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;
        return 0;
    }


    // Dump all collected files с подсчётом суммарного размера
    std::uintmax_t total = 0;
    {
        bool first = true;
        for (auto&amp; f : files)
        {
            dump_file(f, first, opt);
            first = false;
            total += get_file_size(f);
        }
    }

    // Notice:
    //  * В обычном режиме дерево НЕ выводим.
    //  * Теперь дерево выводится ТОЛЬКО через [TREE] секцию config mode.

    if (opt.chunk_trailer)
    {
        std::cout &lt;&lt; &quot;\n===== CHUNK FORMAT HELP =====\n\n&quot;;
        print_chunk_help();
    }

    std::cout &lt;&lt; &quot;\nTotal size: &quot; &lt;&lt; total &lt;&lt; &quot; bytes\n&quot;;

    return 0;
}


int wrap_files_to_html(const std::vector&lt;std::filesystem::path&gt;&amp; files,
                              const Options&amp; opt)
{
    namespace fs = std::filesystem;

    if (opt.wrap_root.empty()) {
        return 0;
    }

    fs::path root = opt.wrap_root;

    std::error_code ec;
    fs::create_directories(root, ec);

    for (const auto&amp; f : files) {
        // считаем относительный путь относительно текущего каталога
        std::error_code rec;
        fs::path rel = fs::relative(f, fs::current_path(), rec);
        if (rec) {
            // если не получилось — хотя бы имя файла
            rel = f.filename();
        }

        fs::path dst = root / rel;
        fs::create_directories(dst.parent_path(), ec);

        std::ifstream in(f, std::ios::binary);
        if (!in) {
            std::cerr &lt;&lt; &quot;Cannot open for wrap: &quot; &lt;&lt; f &lt;&lt; &quot;\n&quot;;
            continue;
        }

        std::ostringstream ss;
        ss &lt;&lt; in.rdbuf();

        std::string title = rel.generic_string();
        std::string html = wrap_cpp_as_html(ss.str(), title);

        std::ofstream out(dst, std::ios::binary);
        if (!out) {
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

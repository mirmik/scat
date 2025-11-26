#include "scat.h"
#include "collector.h"
#include "options.h"
#include "parser.h"
#include "rules.h"
#include "util.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <exception>

int wrap_files_to_html(const std::vector<std::filesystem::path>& files,
                              const Options& opt);


namespace fs = std::filesystem;

bool g_use_absolute_paths = false;

void print_chunk_help()
{
    std::cout << "# Chunk v2 — Change Description Format\n"
                 "\n"
                 "Chunk v2 is a plain-text format for describing modifications to source files.\n"
                 "A patch consists of multiple sections, each describing a single operation:\n"
                 "line-based edits, text-based edits, or file-level operations.\n"
                 "\n"
                 "---\n"
                 "\n"
                 "## 1. Section structure\n"
                 "\n"
                 "=== file: <path> ===\n"
                 "<command>\n"
                 "<content...>\n"
                 "=END=\n"
                 "\n"
                 "* <path> — relative file path\n"
                 "* Empty lines between sections are allowed\n"
                 "* Exactly one command per section\n"
                 "* <content> may contain any lines, including empty ones\n"
                 "\n"
                 "---\n"
                 "\n"
                 "## 3. Commands (text-based)\n"
                 "\n"
                 "Two formats are supported for text-based commands:\n"
                 "\n"
                 "### 3.1 Legacy format (simple)\n"
                 "These commands match an exact multi-line marker in the file.\n"
                 "Everything before the first `---` is the marker; everything after it is content.\n"
                 "\n"
                 "### Insert after marker\n"
                 "--- insert-after-text\n"
                 "<marker lines...>\n"
                 "---\n"
                 "<inserted lines...>\n"
                 "=END=\n"
                 "\n"
                 "### Insert before marker\n"
                 "--- insert-before-text\n"
                 "<marker lines...>\n"
                 "---\n"
                 "<inserted lines...>\n"
                 "=END=\n"
                 "\n"
                 "### Replace marker\n"
                 "--- replace-text\n"
                 "<marker lines...>\n"
                 "---\n"
                 "<new lines...>\n"
                 "=END=\n"
                 "\n"
                 "### Delete marker\n"
                 "--- delete-text\n"
                 "<marker lines...>\n"
                 "---\n"
                 "=END=\n"
                 "\n"
                 "---\n"
                 "\n"
                 "### 3.2 YAML strict format\n"
                 "\n"
                 "In YAML mode you can also specify strict context around the marker:\n"
                 "\n"
                 "--- replace-text\n"
                 "BEFORE:\n"
                 "  <lines that must appear immediately above the marker>\n"
                 "MARKER:\n"
                 "  <marker lines>\n"
                 "AFTER:\n"
                 "  <lines that must appear immediately below the marker>\n"
                 "---\n"
                 "<payload lines...>\n"
                 "=END=\n"
                 "\n"
                 "Rules:\n"
                 "* YAML mode is enabled only when the first non-empty line after the command\n"
                 "  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n"
                 "* Matching is strict: BEFORE lines must be directly above the marker block;\n"
                 "  AFTER lines must be directly below it.\n"
                 "* Whitespace differences are ignored (lines are trimmed before comparison).\n"
                 "* If BEFORE/AFTER are present, there is no fallback to the first occurrence\n"
                 "  of the marker.\n"
                 "* If more than one place matches the strict context, the patch fails as\n"
                 "  ambiguous.\n"
                 "* If no place matches the strict context, the patch fails with\n"
                 "  \"strict marker context not found\".\n"
                 "\n"
                 "---\n"
                 "\n"
                 "## 4. File-level commands\n"
                 "These operations work on the whole file rather than its contents.\n"
                 "\n"
                 "### Create or overwrite file\n"
                 "--- create-file\n"
                 "<file content...>\n"
                 "=END=\n"
                 "\n"
                 "### Delete file\n"
                 "--- delete-file\n"
                 "=END=\n"
                 "\n"
                 "---\n"
                 "\n"
                 "## 5. Examples\n"
                 "\n"
                 "=== file: src/a.cpp ===\n"
                 "--- replace 3:4\n"
                 "int value = 42;\n"
                 "std::cout << value << \"\\\\n\";\n"
                 "=END=\n"
                 "\n"
                 "=== file: src/b.cpp ===\n"
                 "--- insert-after 12\n"
                 "log_debug(\"checkpoint reached\");\n"
                 "=END=\n"
                 "\n"
                 "=== file: src/c.cpp ===\n"
                 "--- delete 20:25\n"
                 "=END=\n"
                 "\n"
                 "=== file: assets/config.json ===\n"
                 "--- create-file\n"
                 "{ \"version\": 1 }\n"
                 "=END=\n"
                 "\n"
                 "=== file: old/temp.txt ===\n"
                 "--- delete-file\n"
                 "=END=\n"
                 "\n"
                 "---\n"
                 "\n"
                 "## Recommended usage\n"
                 "* Prefer text-based commands (`*-text`) — they are more stable when code moves.\n"
                 "* Use file-level commands when creating or removing entire files.\n"
                 "* Group modifications to multiple files into **one patch file**.\n"
                 "\n"
                 "*This cheat sheet is in the text for a reason. If you're asked to write a patch,\n"
                 " use the following format: chunk_v2.\n"
                 "*Try to strictly follow the rules described in this document, without making any\n"
                 " syntactic errors.\n"
                 "*When working with chunks, be careful that commands do not reference the same\n"
                 " text macros. Macros should never overlap.\n";
}

int apply_chunk_main(int argc, char** argv);

void print_tree(const std::vector<std::filesystem::path>& files)
{
    // Собираем относительные пути
    std::vector<std::string> rels;
    rels.reserve(files.size());
    for (auto& p : files)
        rels.push_back(make_display_path(p));

    std::sort(rels.begin(), rels.end());

    struct Node
    {
        std::map<std::string, Node*> children;
        bool is_file = false;
    };

    Node root;

    // ----------------------------
    //  Построение дерева
    // ----------------------------
    for (auto& r : rels)
    {
        fs::path p = r;
        Node* cur = &root;

        // вытаскиваем компоненты p в список
        std::vector<std::string> parts;
        for (auto& part : p)
            parts.push_back(part.string());

        int total = (int)parts.size();

        for (int i = 0; i < total; ++i)
        {
            const std::string& name = parts[i];
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
    std::function<void(Node*, const std::string&, bool, const std::string&)> go;

    go = [&](Node* node, const std::string& name, bool last, const std::string& prefix) {
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
        for (auto& [k, _] : node->children)
            keys.push_back(k);
        std::sort(keys.begin(), keys.end());

        for (size_t i = 0; i < keys.size(); ++i)
        {
            bool is_last = (i + 1 == keys.size());
            Node* child = node->children[keys[i]];

            go(child, keys[i], is_last, prefix + (name.empty() ? "" : (last ? "    " : "│   ")));
        }
    };

    std::cout << "===== PROJECT TREE =====\n";
    go(&root, "", true, "");
    std::cout << "========================\n\n";
}

int run_server(const Options& opt);

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
            ss << std::cin.rdbuf();

            fs::path tmp = fs::temp_directory_path() / "scat_stdin_patch.txt";
            {
                std::ofstream out(tmp);
                out << ss.str();
            }

            std::string tmp_str = tmp.string();
            const char* args[] = {"apply", tmp_str.c_str()};
            int r = apply_chunk_main(2, const_cast<char**>(args));

            fs::remove(tmp);
            return r;
        }
        else
        {
            std::string file = opt.apply_file;
            const char* args[] = {"apply", file.c_str()};
            return apply_chunk_main(2, const_cast<char**>(args));
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
        catch (const std::exception& e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }

        // 1) TEXT rules
        auto text_files = collect_from_rules(cfg.text_rules, opt);

        if (text_files.empty())
        {
            std::cerr << "No files collected.\n";
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

            std::vector<Item> items;
            items.reserve(text_files.size());

            std::uintmax_t total = 0;
            std::size_t max_name = 0;

            for (auto& f : text_files)
            {
                auto disp = make_display_path(f);
                auto sz = get_file_size(f);

                total += sz;

                std::size_t shown_len = opt.path_prefix.size() + disp.size();
                if (shown_len > max_name)
                    max_name = shown_len;

                items.push_back(Item{f, disp, sz});
            }

            if (opt.sorted)
            {
                std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
                    if (a.size != b.size)
                        return a.size > b.size; // по убыванию
                    return a.display < b.display;
                });
            }

            for (const auto& it : items)
            {
                std::string shown = opt.path_prefix + it.display;

                std::cout << shown;
                if (max_name > shown.size())
                {
                    std::size_t pad = max_name - shown.size();
                    for (std::size_t i = 0; i < pad; ++i)
                        std::cout << ' ';
                }
                std::cout << " (" << it.size << " bytes)\n";
            }

            std::cout << "Total size: " << total << " bytes\n";
            return 0;
        }


        // 2) Печать файлов с подсчётом суммарного размера
        bool first = true;
        std::uintmax_t total = 0;
        for (auto& f : text_files)
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
                std::cout << "\n";
                print_tree(tree_files); // выводим дерево строго в самом конце
            }
        }

        // 4) Chunk trailer, if needed
        if (opt.chunk_trailer)
        {
            std::cout << "\n===== CHUNK FORMAT HELP =====\n\n";
            print_chunk_help();
        }

        std::cout << "\nTotal size: " << total << " bytes\n";

        return 0;
    }

    // ------------------------------------------------------------
    // NORMAL MODE — user provided paths, not using a config file
    // ------------------------------------------------------------
    std::vector<std::filesystem::path> files = collect_from_paths(opt.paths, opt);

    if (files.empty())
    {
        std::cerr << "No files collected.\n";
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

        std::vector<Item> items;
        items.reserve(files.size());

        std::uintmax_t total = 0;
        std::size_t max_name = 0;

        for (auto& f : files)
        {
            auto disp = make_display_path(f);
            auto sz = get_file_size(f);

            total += sz;

            std::size_t shown_len = opt.path_prefix.size() + disp.size();
            if (shown_len > max_name)
                max_name = shown_len;

            items.push_back(Item{f, disp, sz});
        }

        if (opt.sorted)
        {
            std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
                if (a.size != b.size)
                    return a.size > b.size; // по убыванию
                return a.display < b.display;
            });
        }

        for (const auto& it : items)
        {
            std::string shown = opt.path_prefix + it.display;

            std::cout << shown;
            if (max_name > shown.size())
            {
                std::size_t pad = max_name - shown.size();
                for (std::size_t i = 0; i < pad; ++i)
                    std::cout << ' ';
            }
            std::cout << " (" << it.size << " bytes)\n";
        }

        std::cout << "Total size: " << total << " bytes\n";
        return 0;
    }


    // Dump all collected files с подсчётом суммарного размера
    std::uintmax_t total = 0;
    {
        bool first = true;
        for (auto& f : files)
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
        std::cout << "\n===== CHUNK FORMAT HELP =====\n\n";
        print_chunk_help();
    }

    std::cout << "\nTotal size: " << total << " bytes\n";

    return 0;
}


int wrap_files_to_html(const std::vector<std::filesystem::path>& files,
                              const Options& opt)
{
    namespace fs = std::filesystem;

    if (opt.wrap_root.empty()) {
        return 0;
    }

    fs::path root = opt.wrap_root;

    std::error_code ec;
    fs::create_directories(root, ec);

    for (const auto& f : files) {
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
            std::cerr << "Cannot open for wrap: " << f << "\n";
            continue;
        }

        std::ostringstream ss;
        ss << in.rdbuf();

        std::string title = rel.generic_string();
        std::string html = wrap_cpp_as_html(ss.str(), title);

        std::ofstream out(dst, std::ios::binary);
        if (!out) {
            std::cerr << "Cannot write wrapped file: " << dst << "\n";
            continue;
        }

        out << html;
    }

    return 0;
}

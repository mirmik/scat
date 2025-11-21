#include "scat.h"
#include "collector.h"
#include "options.h"
#include "rules.h"
#include "util.h"
#include <iostream>
#include <map>
#include <set>

bool g_use_absolute_paths = false;
void print_chunk_help()
{
    std::cout <<
"# Chunk v2 — Change Description Format\n"
"\n"
"Chunk v2 is a plain-text format for describing modifications to source files.\n"
"A patch consists of multiple sections, each describing a single operation\n"
"(replace, insert, delete) applied to one file.\n"
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
"* Empty line between sections is allowed\n"
"* Exactly one command per section\n"
"* <content> may contain any lines, including empty ones\n"
"\n"
"---\n"
"\n"
"## 2. Commands\n"
"\n"
"### Replace a range of lines\n"
"\n"
"--- replace <start>:<end>\n"
"<new lines...>\n"
"=END=\n"
"\n"
"### Insert lines after a given line\n"
"\n"
"--- insert-after <line>\n"
"<inserted lines...>\n"
"=END=\n"
"\n"
"### Delete a range of lines\n"
"\n"
"--- delete <start>:<end>\n"
"=END=\n"
"\n"
"---\n"
"\n"
"## 3. Line numbering\n"
"\n"
"* Zero-based numbering\n"
"* Ranges are inclusive on both ends\n"
"\n"
"---\n"
"\n"
"## 4. Examples\n"
"\n"
"=== file: src/a.cpp ===\n"
"--- replace 3:4\n"
"int value = 42;\n"
"std::cout << value << \"\\n\";\n"
"=END=\n"
"\n"
"=== file: src/b.cpp ===\n"
"--- insert-after 12\n"
"log_debug(\"checkpoint reached\");\n"
"=END=\n"
"\n"
"=== file: src/c.cpp ===\n"
"--- delete 20:25\n"
"=END=\n";
}

int apply_chunk_main(int argc, char** argv);

// Prints a simple directory tree for collected files.
void print_tree(const std::vector<std::filesystem::path>& files)
{
    std::map<std::string, std::set<std::string>> tree;

    for (const auto& p : files)
    {
        auto rel = make_display_path(p);
        auto dir = std::filesystem::path(rel).parent_path().string();
        auto file = std::filesystem::path(rel).filename().string();
        tree[dir].insert(file);
    }

    std::cout << "===== PROJECT TREE =====\n";

    for (const auto& [dir, items] : tree)
    {
        if (!dir.empty() && dir != ".")
            std::cout << dir << "/\n";
        else
            std::cout << "./\n";

        for (auto& f : items)
            std::cout << "  " << f << "\n";
    }

    std::cout << "========================\n\n";
}
int scat_main(int argc, char** argv)
{
    Options opt = parse_options(argc, argv);

    if (opt.chunk_help) {
        print_chunk_help();
        return 0;
    }
    if (!opt.apply_file.empty()) {
        const char* args[] = {"apply", opt.apply_file.c_str()};
        return apply_chunk_main(2, const_cast<char**>(args));
    }

    std::vector<std::filesystem::path> files;

    if (!opt.config_file.empty())
    {
        auto rules = load_rules(opt.config_file);
        files = collect_from_rules(rules, opt);
    }
    else
    {
        files = collect_from_paths(opt.paths, opt);
    }

    if (files.empty())
    {
        std::cerr << "No files collected.\n";
        return 0;
    }

    if (opt.list_only)
    {
        for (auto& f : files)
            std::cout << make_display_path(f) << "\n";
        return 0;
    }

    bool first = true;
    for (auto& f : files)
    {
        dump_file(f, first, opt);
        first = false;
    }

    print_tree(files);
    return 0;
}

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

struct Section
{
    std::string filepath;
    std::string command;
    int a = -1;
    int b = -1;
    std::vector<std::string> payload;
    std::vector<std::string> marker;
    std::vector<std::string> before; // контекст до маркера (BEFORE:)
    std::vector<std::string> after;  // контекст после маркера (AFTER:)
    int seq = 0;
};

static std::vector<std::string> read_file_lines(const fs::path& p)
{
    std::ifstream in(p);
    if (!in)
        throw std::runtime_error("cannot open file: " + p.string());

    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line))
        out.push_back(line);

    return out;
}

static void write_file_lines(const fs::path& p, const std::vector<std::string>& lines)
{
    std::ofstream out(p, std::ios::trunc);
    if (!out)
        throw std::runtime_error("cannot write file: " + p.string());

    for (const auto& s : lines)
        out << s << "\n";
}

std::string trim(const std::string_view &view)
{
    if (view.size() == 0)
        return "";

    const char *left = view.data();
    const char *right = view.data() + view.size() - 1;
    const char *end = view.data() + view.size();

    while (left != end &&
           (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
        ++left;

    if (left == end)
        return "";

    while (left != right && (*right == ' ' || *right == '\n' ||
                             *right == '\r' || *right == '\t'))
        --right;

    return std::string(left, (right - left) + 1);
}

static bool is_text_command(const std::string& cmd)
{
    return cmd == "insert-after-text" ||
           cmd == "insert-before-text" ||
           cmd == "replace-text" ||
           cmd == "delete-text";
}

static int find_subsequence(const std::vector<std::string>& haystack,
                            const std::vector<std::string>& needle)
{
    if (needle.empty() || needle.size() > haystack.size())
        return -1;

    const std::size_t n = haystack.size();
    const std::size_t m = needle.size();

    for (std::size_t i = 0; i + m <= n; ++i)
    {
        bool ok = true;
        for (std::size_t j = 0; j < m; ++j)
        {
            std::string h = trim(haystack[i + j]);
            std::string nn = trim(needle[j]);
            if (h != nn)
            {
                ok = false;
                break;
            }
        }
        if (ok)
            return static_cast<int>(i);
    }

    return -1;
}

// Выбор лучшего совпадения по BEFORE:/AFTER:.
// Если before/after пусты — берём первый кандидат.
// Если score всех 0 — fall back к первому кандидату.
// Если несколько кандидатов с одинаковым максимальным ненулевым score — ошибка.
static int find_best_marker_match(const std::vector<std::string>& lines,
                                  const Section* s,
                                  const std::vector<int>& candidates)
{
    if (candidates.empty())
        return -1;

    if (s->before.empty() && s->after.empty())
        return candidates.front();

    struct CandScore
    {
        int pos;
        int score;
    };

    std::vector<CandScore> scored;
    scored.reserve(candidates.size());

    for (int pos : candidates)
    {
        int score = 0;

        // BEFORE: ищем любую строку из before в окне перед маркером
        if (!s->before.empty())
        {
            bool matched_before = false;
            for (const auto& b : s->before)
            {
                if (trim(b).empty())
                    continue;

                for (int i = pos - 1; i >= 0 && i >= pos - 20; --i)
                {
                    if (trim(lines[static_cast<std::size_t>(i)]) == trim(b))
                    {
                        matched_before = true;
                        break;
                    }
                }
                if (matched_before)
                    break;
            }
            if (matched_before)
                ++score;
        }

        // AFTER: ищем любую строку из after в окне после маркера
        if (!s->after.empty())
        {
            bool matched_after = false;
            int start = pos + static_cast<int>(s->marker.size());
            int end = std::min<int>(static_cast<int>(lines.size()), start + 20);

            for (const auto& a : s->after)
            {
                if (trim(a).empty())
                    continue;

                for (int i = start; i < end; ++i)
                {
                    if (trim(lines[static_cast<std::size_t>(i)]) == trim(a))
                    {
                        matched_after = true;
                        break;
                    }
                }
                if (matched_after)
                    break;
            }
            if (matched_after)
                ++score;
        }

        scored.push_back({pos, score});
    }

    std::sort(scored.begin(), scored.end(),
              [](const CandScore& x, const CandScore& y) {
                  return x.score > y.score;
              });

    if (scored.empty())
        return -1;

    // Если лучший не дал никакого контекста — работаем как раньше (первое вхождение)
    if (scored[0].score == 0)
        return candidates.front();

    // Если есть ещё один с тем же ненулевым score — неоднозначность
    if (scored.size() > 1 && scored[0].score == scored[1].score)
        throw std::runtime_error("ambiguous fuzzy marker match");

    return scored[0].pos;
}

static void apply_text_commands(const std::string& filepath,
                                std::vector<std::string>& lines,
                                const std::vector<const Section*>& sections)
{
    for (const Section* s : sections)
    {
        if (s->marker.empty())
            throw std::runtime_error("empty marker in text command for file: " + filepath);

        // Собираем все вхождения маркера
        std::vector<int> candidates;
        {
            int base = 0;
            while (base < static_cast<int>(lines.size()))
            {
                std::vector<std::string> sub(lines.begin() + base, lines.end());
                int idx = find_subsequence(sub, s->marker);
                if (idx < 0)
                    break;
                candidates.push_back(base + idx);
                base += idx + 1;
            }
        }

        if (candidates.empty())
            throw std::runtime_error(
                "text marker not found for file: " + filepath +
                "\ncommand: " + s->command + "\n");

        int idx = find_best_marker_match(lines, s, candidates);
        if (idx < 0)
            throw std::runtime_error("cannot locate marker uniquely");

        std::size_t pos = static_cast<std::size_t>(idx);

        if (s->command == "insert-after-text")
        {
            pos += s->marker.size();
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         s->payload.begin(), s->payload.end());
        }
        else if (s->command == "insert-before-text")
        {
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         s->payload.begin(), s->payload.end());
        }
        else if (s->command == "replace-text")
        {
            auto begin = lines.begin() + static_cast<std::ptrdiff_t>(pos);
            auto end = begin + static_cast<std::ptrdiff_t>(s->marker.size());
            lines.erase(begin, end);
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         s->payload.begin(), s->payload.end());
        }
        else if (s->command == "delete-text")
        {
            auto begin = lines.begin() + static_cast<std::ptrdiff_t>(pos);
            auto end = begin + static_cast<std::ptrdiff_t>(s->marker.size());
            lines.erase(begin, end);
        }
        else
        {
            throw std::runtime_error("unknown text command: " + s->command);
        }
    }
}

static Section parse_section(std::istream& in, const std::string& header)
{
    Section s;

    auto pos = header.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("bad section header: " + header);

    auto pos2 = header.find("===", pos);
    if (pos2 == std::string::npos)
        pos2 = header.size();

    auto raw = header.substr(pos + 1, pos2 - pos - 1);
    s.filepath = trim(raw);
    if (s.filepath.empty())
        throw std::runtime_error("empty filepath in header: " + header);

    std::string line;
    if (!std::getline(in, line))
        throw std::runtime_error("unexpected end after header");

    if (line.rfind("--- ", 0) != 0)
        throw std::runtime_error("expected command after header");

    {
        std::istringstream ss(line.substr(4));
        ss >> s.command;

        if (is_text_command(s.command))
        {
        }
        else if (s.command == "create-file" || s.command == "delete-file")
        {
        }
        else
        {
            throw std::runtime_error("index-based commands removed: " + s.command);
        }
    }

    bool found_end = false;
    while (std::getline(in, line))
    {
        if (line == "=END=")
        {
            found_end = true;
            break;
        }
        s.payload.push_back(line);
    }

    if (!found_end)
        throw std::runtime_error("missing =END=");

    if (is_text_command(s.command))
    {
        // Определяем, в YAML-режиме мы или в старом формате.
        // Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется старая логика.
        bool yaml_mode = false;
        std::size_t first_non_empty = 0;
        while (first_non_empty < s.payload.size() &&
               trim(s.payload[first_non_empty]).empty())
            ++first_non_empty;

        if (first_non_empty < s.payload.size())
        {
            const std::string t = trim(s.payload[first_non_empty]);
            if (t == "BEFORE:" || t == "MARKER:" || t == "AFTER:")
                yaml_mode = true;
        }

        if (!yaml_mode)
        {
            // Старый режим: всё до '---' — marker, после — payload
            auto it = std::find(s.payload.begin(), s.payload.end(), std::string("---"));
            if (it == s.payload.end())
                throw std::runtime_error("text command requires '---' separator");

            s.marker.assign(s.payload.begin(), it);

            std::vector<std::string> tail;
            if (std::next(it) != s.payload.end())
                tail.assign(std::next(it), s.payload.end());

            s.payload.swap(tail);

            if (s.marker.empty())
                throw std::runtime_error("empty text marker");
        }
        else
        {
            // YAML-подобный режим:
            // BEFORE:
            //   ...
            // MARKER:
            //   ...
            // AFTER:
            //   ...
            // ---
            // <payload>
            s.before.clear();
            s.marker.clear();
            s.after.clear();

            enum class Block
            {
                NONE,
                BEFORE,
                MARKER,
                AFTER
            };

            Block blk = Block::NONE;
            std::vector<std::string> new_payload;

            bool seen_separator = false;

            for (std::size_t i = first_non_empty; i < s.payload.size(); ++i)
            {
                const std::string& ln = s.payload[i];

                if (!seen_separator && ln == "---")
                {
                    seen_separator = true;
                    continue;
                }

                if (!seen_separator)
                {
                    const std::string t = trim(ln);
                    if (t == "BEFORE:")
                    {
                        blk = Block::BEFORE;
                        continue;
                    }
                    if (t == "MARKER:")
                    {
                        blk = Block::MARKER;
                        continue;
                    }
                    if (t == "AFTER:")
                    {
                        blk = Block::AFTER;
                        continue;
                    }

                    switch (blk)
                    {
                        case Block::BEFORE:
                            s.before.push_back(ln);
                            break;
                        case Block::MARKER:
                            s.marker.push_back(ln);
                            break;
                        case Block::AFTER:
                            s.after.push_back(ln);
                            break;
                        case Block::NONE:
                            throw std::runtime_error("unexpected content before YAML block tag");
                    }
                }
                else
                {
                    new_payload.push_back(ln);
                }
            }

            s.payload.swap(new_payload);

            if (s.marker.empty())
                throw std::runtime_error("YAML text command requires MARKER: section");
        }
    }

    return s;
}

static void apply_for_file(const std::string& filepath,
                           const std::vector<const Section*>& sections)
{
    fs::path p = filepath;
    std::vector<std::string> orig;
    bool existed = true;

    try
    {
        orig = read_file_lines(p);
    }
    catch (...)
    {
        existed = false;
        orig.clear();
    }

    for (const Section* s : sections)
    {
        if (!existed && s->command == "delete-file")
            throw std::runtime_error("delete-file: file does not exist");
    }

    for (const Section* s : sections)
    {
        if (s->command == "create-file")
        {
            write_file_lines(p, s->payload);
            return;
        }
        if (s->command == "delete-file")
        {
            std::error_code ec;
            fs::remove(p, ec);
            if (ec)
                throw std::runtime_error("delete-file failed");
            return;
        }
    }

    std::vector<const Section*> text_sections;
    for (const Section* s : sections)
    {
        if (is_text_command(s->command))
            text_sections.push_back(s);
        else
            throw std::runtime_error("unexpected non-text command");
    }

    if (!text_sections.empty())
    {
        apply_text_commands(filepath, orig, text_sections);
        write_file_lines(p, orig);
    }
}

static void apply_all(const std::vector<Section>& sections)
{
    namespace fs = std::filesystem;

    // 1. Собираем список всех файлов, которые будут затронуты
    std::vector<std::string> files;
    files.reserve(sections.size());
    for (auto& s : sections)
        files.push_back(s.filepath);

    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());

    struct Backup {
        bool existed = false;
        std::vector<std::string> lines;
    };

    std::map<std::string, Backup> backup;

    // 2. Делаем резервную копию всех файлов
    for (auto& f : files)
    {
        Backup b;
        fs::path p = f;

        std::error_code ec;

        if (fs::exists(p, ec))
        {
            b.existed = true;

            try {
                b.lines = read_file_lines(p);
            } catch (...) {
                throw std::runtime_error("cannot read original file: " + f);
            }
        }
        else
        {
            b.existed = false;
        }

        backup[f] = std::move(b);
    }

    // 3. Применяем секции с защитой (try/catch)
    try
    {
        for (auto& s : sections)
        {
            std::vector<const Section*> single { &s };
            apply_for_file(s.filepath, single);
        }
    }
    catch (...)
    {
        // 4. Откат (rollback)
        for (auto& [path, b] : backup)
        {
            fs::path p = path;
            std::error_code ec;

            if (b.existed)
            {
                try {
                    write_file_lines(p, b.lines);
                } catch (...) {
                    // если даже откат не удался — сдаёмся
                }
            }
            else
            {
                fs::remove(p, ec);
            }
        }

        throw;
    }
}

int apply_chunk_main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "usage: apply_patch <patchfile>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in)
    {
        std::cerr << "cannot open patch file: " << argv[1] << "\n";
        return 1;
    }

    std::string line;
    std::vector<Section> sections;
    int seq = 0;

    try
    {
        while (std::getline(in, line))
        {
            if (line.rfind("=== file:", 0) == 0)
            {
                Section s = parse_section(in, line);
                s.seq = seq++;
                sections.push_back(std::move(s));
            }
        }

        apply_all(sections);
    }
    catch (const std::exception& e)
    {
        std::cerr << "error while applying patch: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
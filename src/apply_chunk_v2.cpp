#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Section
{
    std::string filepath;
    std::string command;
    int a = -1;
    int b = -1;
    std::vector<std::string> payload;
    std::vector<std::string> marker; // для *-text команд
    int seq = 0; // порядок появления в патче
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

static std::string trim(std::string s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.pop_back();
    return s;
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
    std::string n = trim(needle[j]);
    if (h != n)
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

static void apply_text_commands(const std::string& filepath,
                                std::vector<std::string>& lines,
                                const std::vector<const Section*>& sections)
{
    for (const Section* s : sections)
    {
        if (s->marker.empty())
            throw std::runtime_error("empty marker in text command for file: " + filepath);

        int idx = find_subsequence(lines, s->marker);
        if (idx < 0)
            throw std::runtime_error("text marker not found for file: " + filepath);

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
            throw std::runtime_error("unknown text command while applying: " + s->command);
        }
    }
}

static Section parse_section(std::istream& in, const std::string& header)
{
    Section s;

    // header: "=== file: path ==="
    {
        auto pos = header.find(':');
        if (pos == std::string::npos)
            throw std::runtime_error("bad section header (no colon): " + header);

        auto pos2 = header.find("===", pos);
        if (pos2 == std::string::npos)
            pos2 = header.size();

        auto raw = header.substr(pos + 1, pos2 - pos - 1);
        s.filepath = trim(raw);
        if (s.filepath.empty())
            throw std::runtime_error("empty filepath in header: " + header);
    }

    std::string line;
    if (!std::getline(in, line))
        throw std::runtime_error("unexpected end after section header for file: " + s.filepath);

    // command line: "--- replace A:B" или "--- insert-after X" или "--- delete A:B" и новые *-text
    if (line.rfind("--- ", 0) != 0)
        throw std::runtime_error("expected command after header, got: " + line);

    {
        std::istringstream ss(line.substr(4));
        ss >> s.command;

        if (s.command == "replace" || s.command == "delete")
        {
            std::string range;
            ss >> range;
            auto pos = range.find(':');
            if (pos == std::string::npos)
                throw std::runtime_error("bad range: " + range);
            s.a = std::stoi(range.substr(0, pos));
            s.b = std::stoi(range.substr(pos + 1));
        }
        else if (s.command == "insert-after")
        {
            ss >> s.a;
        }
        else if (s.command == "create-file")
        {
            // no args
        }
        else if (s.command == "delete-file")
        {
            // no args
        }
        else if (is_text_command(s.command))
        {
            // для *-text аргументы после команды в строке не нужны
        }
        else
        {
            throw std::runtime_error("unknown command: " + s.command);
        }
    }

    // payload until "=END="
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
        throw std::runtime_error("missing =END= for section file: " + s.filepath);

    // разбор payload для *-text команд: marker --- payload
    if (is_text_command(s.command))
    {
        auto it = std::find(s.payload.begin(), s.payload.end(), std::string("---"));
        if (it == s.payload.end())
            throw std::runtime_error("text-based command requires '---' separator in payload for file: " + s.filepath);

        s.marker.assign(s.payload.begin(), it);

        std::vector<std::string> tail;
        if (std::next(it) != s.payload.end())
            tail.assign(std::next(it), s.payload.end());

        s.payload.swap(tail);

        if (s.marker.empty())
            throw std::runtime_error("empty text marker for file: " + s.filepath);
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
    catch (const std::runtime_error&)
    {
        existed = false;
        orig.clear();
    }

    // если файла нет — допускаем только insert-after с индексом -1 (создание нового файла)
    if (!existed)
    {
        for (const Section* s : sections)
        {
            if (s->command == "create-file")
                continue;

            if (s->command == "delete-file")
                throw std::runtime_error("delete-file: file does not exist: " + filepath);

            if (s->command != "insert-after")
                throw std::runtime_error("file does not exist: " + filepath);
        }
    }

    const int N = static_cast<int>(orig.size());

    std::vector<const Section*> ranges;                  // replace/delete по индексам
    std::vector<std::vector<const Section*>> inserts_after;
    std::vector<const Section*> inserts_before;          // insert-after -1
    std::vector<const Section*> text_sections;           // *-text команды

    inserts_after.resize(static_cast<std::size_t>(std::max(N, 0)));

    bool has_index_ops = false;
    bool has_text_ops = false;

    for (const Section* s : sections)
    {
        if (s->command == "create-file" || s->command == "delete-file")
            continue;

        if (is_text_command(s->command))
        if (is_text_command(s->command))
        {
            has_text_ops = true;
            text_sections.push_back(s);
            continue;
        }

        has_index_ops = true;

        if (s->command == "insert-after")
        {
            if (s->a < -1 || s->a >= N)
            {
                if (!existed && N == 0 && s->a == -1)
                {
                    // создание нового файла insert-after -1 — ок
                }
                else
                {
                    throw std::runtime_error("bad insert-after index for file: " + filepath);
                }
            }

            if (s->a == -1)
                inserts_before.push_back(s);
            else
                inserts_after[static_cast<std::size_t>(s->a)].push_back(s);
        }
        else if (s->command == "replace" || s->command == "delete")
        {
            ranges.push_back(s);
        }
        else
        {
            throw std::runtime_error("unknown command while applying: " + s->command);
        }
    }

    // пока что не разрешаем смешивать индексные и текстовые операции для одного файла —
    // иначе начинается весёлый сдвиг индексов. Можно будет спроектировать порядок позже.
    if (has_index_ops && has_text_ops)
        throw std::runtime_error("cannot mix index-based and text-based commands for file: " + filepath);

    // file-level ops
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
                throw std::runtime_error("delete-file failed: " + filepath);
            return;
        }
    }

    // если есть текстовые команды — применяем их к orig и всё
    if (has_text_ops)
    // если есть текстовые команды — применяем их к orig и всё
    if (has_text_ops)
    {
        apply_text_commands(filepath, orig, text_sections);
        write_file_lines(p, orig);
        return;
    }

    // дальше — старая логика индексных операций

    // сортируем replace/delete по началу диапазона и порядку появления
    std::sort(ranges.begin(), ranges.end(),
              [](const Section* lhs, const Section* rhs) {
                  if (lhs->a != rhs->a)
                      return lhs->a < rhs->a;
                  return lhs->seq < rhs->seq;
              });

    // проверка диапазонов и пересечений
    int last_end = -1;
    for (const Section* s : ranges)
    {
        if (s->a < 0 || s->b >= N || s->a > s->b)
            throw std::runtime_error("bad replace/delete range for file: " + filepath);

        if (s->a <= last_end)
            throw std::runtime_error("overlapping ranges for file: " + filepath);

        last_end = s->b;
    }

    std::vector<std::string> out;
    out.reserve(static_cast<std::size_t>(N) + 16);

    // вставки до первой строки (insert-after -1) — в порядке появления
    for (const Section* s : inserts_before)
        out.insert(out.end(), s->payload.begin(), s->payload.end());

    std::size_t i = 0;
    std::size_t ri = 0;

    while (i < static_cast<std::size_t>(N))
    {
        // range-операция, начинающаяся на i?
        if (ri < ranges.size() &&
            ranges[ri]->a == static_cast<int>(i))
        {
            const Section* s = ranges[ri];

            if (s->command == "replace")
            {
                out.insert(out.end(), s->payload.begin(), s->payload.end());
            }
            // delete просто пропускает диапазон

            i = static_cast<std::size_t>(s->b) + 1;
            ++ri;
            continue;
        }

        // обычная строка
        out.push_back(orig[i]);

        // вставки после этой строки
        for (const Section* ins : inserts_after[i])
            out.insert(out.end(), ins->payload.begin(), ins->payload.end());

        ++i;
    }

    write_file_lines(p, out);
}

static void apply_all(const std::vector<Section>& sections)
{
    std::map<std::string, std::vector<const Section*>> by_file;

    for (const auto& s : sections)
        by_file[s.filepath].push_back(&s);

    for (auto& [path, vec] : by_file)
        apply_for_file(path, vec);
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

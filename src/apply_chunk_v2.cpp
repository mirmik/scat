#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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

    // command line: "--- replace A:B" или "--- insert-after X" или "--- delete A:B"
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

    return s;
}

static void apply_section(const Section& s)
{
    fs::path p = s.filepath;
    std::vector<std::string> lines;

    // читаем файл; для insert-after допускаем создание нового файла
    try
    {
        lines = read_file_lines(p);
    }
    catch (const std::runtime_error&)
    {
        if (s.command == "insert-after")
        {
            lines.clear();
        }
        else
        {
            throw;
        }
    }

    if (s.command == "replace")
    {
        if (s.a < 0 || s.b >= static_cast<int>(lines.size()) || s.a > s.b)
            throw std::runtime_error("bad replace range for file: " + p.string());

        std::vector<std::string> out;
        out.reserve(lines.size() - (s.b - s.a + 1) + s.payload.size());

        out.insert(out.end(), lines.begin(), lines.begin() + s.a);
        out.insert(out.end(), s.payload.begin(), s.payload.end());
        out.insert(out.end(), lines.begin() + s.b + 1, lines.end());

        write_file_lines(p, out);
    }
    else if (s.command == "delete")
    {
        if (s.a < 0 || s.b >= static_cast<int>(lines.size()) || s.a > s.b)
            throw std::runtime_error("bad delete range for file: " + p.string());

        std::vector<std::string> out;
        out.reserve(lines.size() - (s.b - s.a + 1));

        out.insert(out.end(), lines.begin(), lines.begin() + s.a);
        out.insert(out.end(), lines.begin() + s.b + 1, lines.end());

        write_file_lines(p, out);
    }
    else if (s.command == "insert-after")
    {
        if (s.a < -1 || s.a >= static_cast<int>(lines.size()))
            throw std::runtime_error("bad insert-after index for file: " + p.string());

        std::vector<std::string> out;
        out.reserve(lines.size() + s.payload.size());

        // s.a == -1 → вставка в начало (до первой строки)
        out.insert(out.end(), lines.begin(), lines.begin() + (s.a + 1));
        out.insert(out.end(), s.payload.begin(), s.payload.end());
        out.insert(out.end(), lines.begin() + (s.a + 1), lines.end());

        write_file_lines(p, out);
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
    try
    {
        while (std::getline(in, line))
        {
            if (line.rfind("=== file:", 0) == 0)
            {
                Section s = parse_section(in, line);
                apply_section(s);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "error while applying patch: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

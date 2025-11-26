#include "util.h"
#include <filesystem>
#include <fstream>
#include <iostream> // у тебя уже есть для std::cerr
#include <sstream>

namespace fs = std::filesystem;

extern bool g_use_absolute_paths;

bool starts_with(const std::string &s, const std::string &prefix)
{
    return s.size() >= prefix.size() &&
           s.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string &s, const std::string &suffix)
{
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string make_display_path(const fs::path &p)
{
    if (g_use_absolute_paths)
        return fs::absolute(p).string();

    std::error_code ec;
    fs::path rel = fs::relative(p, fs::current_path(), ec);
    if (!ec)
        return rel.string();

    return p.string();
}

void dump_file(const fs::path &p, bool first, const Options &opt)
{
    std::ifstream in(p, std::ios::binary);
    if (!in)
    {
        std::cerr << "Cannot open: " << p << "\n";
        return;
    }

    if (!first)
        std::cout << "\n";

    std::cout << "===== " << make_display_path(p) << " =====\n";
    // std::cout << in.rdbuf() << "=EOF=\n";

    std::string line;
    size_t line_no = 0;
    while (std::getline(in, line))
    {
        if (opt.line_numbers)
            std::cout << line_no << ": " << line << "\n";
        else
            std::cout << line << "\n";
        ++line_no;
    }
}
std::uintmax_t get_file_size(const fs::path &p)
{
    std::error_code ec;
    auto sz = fs::file_size(p, ec);
    if (ec)
        return 0;
    return sz;
}

bool match_simple(const fs::path &p, const std::string &pat)
{
    std::string name = p.filename().string();

    if (pat == "*")
        return true;

    auto pos = pat.find('*');
    if (pos == std::string::npos)
        return name == pat;

    std::string a = pat.substr(0, pos);
    std::string b = pat.substr(pos + 1);

    return starts_with(name, a) && ends_with(name, b);
}

std::string html_escape(std::string_view src)
{
    std::string out;
    out.reserve(src.size() * 11 / 10 + 16); // чуть с запасом

    for (char ch : src)
    {
        switch (ch)
        {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out.push_back(ch);
            break;
        }
    }

    return out;
}

std::string wrap_cpp_as_html(std::string_view cpp_code, std::string_view title)
{
    std::string escaped = html_escape(cpp_code);

    std::string html;
    html.reserve(escaped.size() + 256);

    html += "<!DOCTYPE html>\n";
    html += "<html lang=\"en\">\n";
    html += "<head>\n";
    html += "  <meta charset=\"UTF-8\">\n";
    html += "  <title>";
    html += std::string(title);
    html += "</title>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<pre><code>\n";
    html += escaped;
    html += "\n</code></pre>\n";
    html += "</body>\n";
    html += "</html>\n";

    return html;
}

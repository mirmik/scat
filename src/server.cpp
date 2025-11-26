#include "options.h"
#include "collector.h"
#include "parser.h"
#include "util.h"

#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdint>

namespace fs = std::filesystem;

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

static std::string json_escape(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

int run_server(const Options& opt)
{
#ifdef _WIN32
    (void)opt;
    std::cerr << "--server is not supported on Windows yet.\n";
    return 1;
#else
    if (opt.server_port <= 0 || opt.server_port > 65535)
    {
        std::cerr << "Invalid port for --server: " << opt.server_port << "\n";
        return 1;
    }

    // Собираем список файлов так же, как в обычном scat:
    // при наличии config_file — через scat.txt/--config,
    // иначе — из paths.
    std::vector<fs::path> files;

    if (!opt.config_file.empty())
    {
        Config cfg = parse_config(opt.config_file);
        files = collect_from_rules(cfg.text_rules, opt);
    }
    else
    {
        files = collect_from_paths(opt.paths, opt);
    }

    if (files.empty())
    {
        std::cerr << "No files collected for server.\n";
        return 1;
    }

    // Ключ: строковый путь (как печатаем в scat), значение: реальный путь
    std::map<std::string, fs::path> file_map;
    for (auto& f : files)
    {
        std::string key = make_display_path(f);
        file_map[key] = f;
    }

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int yes = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(opt.server_port));

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        perror("bind");
        ::close(server_fd);
        return 1;
    }

    if (::listen(server_fd, 16) < 0)
    {
        perror("listen");
        ::close(server_fd);
        return 1;
    }

    std::cout << "Serving " << file_map.size()
              << " files on http://127.0.0.1:" << opt.server_port << "\n"
              << "Endpoints:\n"
              << "  /index.json\n"
              << "  /<path>\n";

    for (;;)
    {
        int client = ::accept(server_fd, nullptr, nullptr);
        if (client < 0)
        {
            perror("accept");
            continue;
        }

        std::string request;
        char buf[4096];
        ssize_t n = ::recv(client, buf, sizeof(buf), 0);
        if (n <= 0)
        {
            ::close(client);
            continue;
        }
        request.append(buf, static_cast<std::size_t>(n));

        std::istringstream req_stream(request);
        std::string method, target, version;
        req_stream >> method >> target >> version;

        auto send_response = [&](const std::string& status_line,
                                 const std::string& content_type,
                                 const std::string& body)
        {
            std::ostringstream out;
            out << status_line << "\r\n";
            out << "Content-Type: " << content_type << "\r\n";
            out << "Content-Length: " << body.size() << "\r\n";
            out << "Connection: close\r\n";
            out << "\r\n";
            out << body;
            std::string s = out.str();
            ::send(client, s.data(), s.size(), 0);
        };

        if (method != "GET")
        {
            send_response("HTTP/1.0 405 Method Not Allowed",
                          "text/plain; charset=utf-8",
                          "Only GET is supported\n");
            ::close(client);
            continue;
        }

        if (target.empty())
            target = "/";

        if (target == "/index.json")
        {
            std::ostringstream body;
            body << "{\n  \"files\": [\n";
            bool first = true;
            for (const auto& [name, _] : file_map)
            {
                if (!first)
                    body << ",\n";
                first = false;
                body << "    \"" << json_escape(name) << "\"";
            }
            body << "\n  ]\n}\n";

            send_response("HTTP/1.0 200 OK",
                          "application/json; charset=utf-8",
                          body.str());
            ::close(client);
            continue;
        }

        if (!target.empty() && target[0] == '/')
            target.erase(0, 1);

        auto it = file_map.find(target);
        if (it == file_map.end())
        {
            send_response("HTTP/1.0 404 Not Found",
                          "text/plain; charset=utf-8",
                          "Not found\n");
            ::close(client);
            continue;
        }

        std::ifstream in(it->second, std::ios::binary);
        if (!in)
        {
            send_response("HTTP/1.0 500 Internal Server Error",
                          "text/plain; charset=utf-8",
                          "Cannot open file\n");
            ::close(client);
            continue;
        }

        std::ostringstream body;
        body << in.rdbuf();

        send_response("HTTP/1.0 200 OK",
                      "text/plain; charset=utf-8",
                      body.str());
        ::close(client);
    }

    // теоретически недостижимо
    ::close(server_fd);
    return 0;
#endif
}

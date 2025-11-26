<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/server.cpp</title>
</head>
<body>
<pre><code>
#include &quot;options.h&quot;
#include &quot;collector.h&quot;
#include &quot;parser.h&quot;
#include &quot;util.h&quot;

#include &lt;filesystem&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;sstream&gt;
#include &lt;fstream&gt;
#include &lt;vector&gt;
#include &lt;cstdint&gt;

namespace fs = std::filesystem;

#ifndef _WIN32
#include &lt;sys/types.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;netinet/in.h&gt;
#include &lt;arpa/inet.h&gt;
#include &lt;unistd.h&gt;
#endif

static std::string json_escape(const std::string&amp; s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
            case '\\': out += &quot;\\\\&quot;; break;
            case '&quot;':  out += &quot;\\\&quot;&quot;; break;
            case '\n': out += &quot;\\n&quot;;  break;
            case '\r': out += &quot;\\r&quot;;  break;
            case '\t': out += &quot;\\t&quot;;  break;
            default:   out += c;      break;
        }
    }
    return out;
}

int run_server(const Options&amp; opt)
{
#ifdef _WIN32
    (void)opt;
    std::cerr &lt;&lt; &quot;--server is not supported on Windows yet.\n&quot;;
    return 1;
#else
    if (opt.server_port &lt;= 0 || opt.server_port &gt; 65535)
    {
        std::cerr &lt;&lt; &quot;Invalid port for --server: &quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;;
        return 1;
    }

    // Собираем список файлов так же, как в обычном scat:
    // при наличии config_file — через scat.txt/--config,
    // иначе — из paths.
    std::vector&lt;fs::path&gt; files;

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
        std::cerr &lt;&lt; &quot;No files collected for server.\n&quot;;
        return 1;
    }

    // Ключ: строковый путь (как печатаем в scat), значение: реальный путь
    std::map&lt;std::string, fs::path&gt; file_map;
    for (auto&amp; f : files)
    {
        std::string key = make_display_path(f);
        file_map[key] = f;
    }

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd &lt; 0)
    {
        perror(&quot;socket&quot;);
        return 1;
    }

    int yes = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &amp;yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast&lt;uint16_t&gt;(opt.server_port));

    if (::bind(server_fd, reinterpret_cast&lt;sockaddr*&gt;(&amp;addr), sizeof(addr)) &lt; 0)
    {
        perror(&quot;bind&quot;);
        ::close(server_fd);
        return 1;
    }

    if (::listen(server_fd, 16) &lt; 0)
    {
        perror(&quot;listen&quot;);
        ::close(server_fd);
        return 1;
    }

    std::cout &lt;&lt; &quot;Serving &quot; &lt;&lt; file_map.size()
              &lt;&lt; &quot; files on http://127.0.0.1:&quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;
              &lt;&lt; &quot;Endpoints:\n&quot;
              &lt;&lt; &quot;  /index.json\n&quot;
              &lt;&lt; &quot;  /&lt;path&gt;\n&quot;;

    for (;;)
    {
        int client = ::accept(server_fd, nullptr, nullptr);
        if (client &lt; 0)
        {
            perror(&quot;accept&quot;);
            continue;
        }

        std::string request;
        char buf[4096];
        ssize_t n = ::recv(client, buf, sizeof(buf), 0);
        if (n &lt;= 0)
        {
            ::close(client);
            continue;
        }
        request.append(buf, static_cast&lt;std::size_t&gt;(n));

        std::istringstream req_stream(request);
        std::string method, target, version;
        req_stream &gt;&gt; method &gt;&gt; target &gt;&gt; version;

        auto send_response = [&amp;](const std::string&amp; status_line,
                                 const std::string&amp; content_type,
                                 const std::string&amp; body)
        {
            std::ostringstream out;
            out &lt;&lt; status_line &lt;&lt; &quot;\r\n&quot;;
            out &lt;&lt; &quot;Content-Type: &quot; &lt;&lt; content_type &lt;&lt; &quot;\r\n&quot;;
            out &lt;&lt; &quot;Content-Length: &quot; &lt;&lt; body.size() &lt;&lt; &quot;\r\n&quot;;
            out &lt;&lt; &quot;Connection: close\r\n&quot;;
            out &lt;&lt; &quot;\r\n&quot;;
            out &lt;&lt; body;
            std::string s = out.str();
            ::send(client, s.data(), s.size(), 0);
        };

        if (method != &quot;GET&quot;)
        {
            send_response(&quot;HTTP/1.0 405 Method Not Allowed&quot;,
                          &quot;text/plain; charset=utf-8&quot;,
                          &quot;Only GET is supported\n&quot;);
            ::close(client);
            continue;
        }

        if (target.empty())
            target = &quot;/&quot;;

        // Корневая страница с краткой подсказкой
        if (target == &quot;/&quot;)
        {
            std::ostringstream body;
            body
                &lt;&lt; &quot;&lt;!DOCTYPE html&gt;\n&quot;
                &lt;&lt; &quot;&lt;html&gt;&lt;head&gt;&lt;meta charset=\&quot;utf-8\&quot;&gt;&quot;
                &lt;&lt; &quot;&lt;title&gt;scat server&lt;/title&gt;&lt;/head&gt;&lt;body&gt;\n&quot;
                &lt;&lt; &quot;&lt;h1&gt;scat server&lt;/h1&gt;\n&quot;
                &lt;&lt; &quot;&lt;p&gt;This is a read-only file server started by &quot;
                &lt;&lt; &quot;&lt;code&gt;scat --server PORT&lt;/code&gt;.&lt;/p&gt;\n&quot;
                &lt;&lt; &quot;&lt;ul&gt;\n&quot;
                &lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/index.json&lt;/code&gt; – list of files as JSON.&lt;/li&gt;\n&quot;
                &lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/{relative-path}&lt;/code&gt; – raw file contents. &quot;
                &lt;&lt; &quot;Path must match an entry from &lt;code&gt;index.json&lt;/code&gt;.&lt;/li&gt;\n&quot;
                &lt;&lt; &quot;&lt;/ul&gt;\n&quot;
                &lt;&lt; &quot;&lt;/body&gt;&lt;/html&gt;\n&quot;;

            send_response(&quot;HTTP/1.0 200 OK&quot;,
                          &quot;text/html; charset=utf-8&quot;,
                          body.str());
            ::close(client);
            continue;
        }

        if (target == &quot;/index.json&quot;)
        {
            std::ostringstream body;
            body &lt;&lt; &quot;{\n  \&quot;files\&quot;: [\n&quot;;
            bool first = true;
            for (const auto&amp; [name, _] : file_map)
            {
                if (!first)
                    body &lt;&lt; &quot;,\n&quot;;
                first = false;
                body &lt;&lt; &quot;    \&quot;&quot; &lt;&lt; json_escape(name) &lt;&lt; &quot;\&quot;&quot;;
            }
            body &lt;&lt; &quot;\n  ]\n}\n&quot;;

            send_response(&quot;HTTP/1.0 200 OK&quot;,
                          &quot;application/json; charset=utf-8&quot;,
                          body.str());
            ::close(client);
            continue;
        }

        if (!target.empty() &amp;&amp; target[0] == '/')
            target.erase(0, 1);

        auto it = file_map.find(target);
        if (it == file_map.end())
        {
            send_response(&quot;HTTP/1.0 404 Not Found&quot;,
                          &quot;text/plain; charset=utf-8&quot;,
                          &quot;Not found\n&quot;);
            ::close(client);
            continue;
        }

        std::ifstream in(it-&gt;second, std::ios::binary);
        if (!in)
        {
            send_response(&quot;HTTP/1.0 500 Internal Server Error&quot;,
                          &quot;text/plain; charset=utf-8&quot;,
                          &quot;Cannot open file\n&quot;);
            ::close(client);
            continue;
        }

        std::ostringstream body;
        body &lt;&lt; in.rdbuf();

        send_response(&quot;HTTP/1.0 200 OK&quot;,
                      &quot;text/plain; charset=utf-8&quot;,
                      body.str());
        ::close(client);
    }

    // теоретически недостижимо
    ::close(server_fd);
    return 0;
#endif
}

</code></pre>
</body>
</html>

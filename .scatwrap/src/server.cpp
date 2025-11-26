<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/server.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;collector.h&quot;<br>
#include &quot;options.h&quot;<br>
#include &quot;parser.h&quot;<br>
#include &quot;util.h&quot;<br>
<br>
#include &lt;cstdint&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;fstream&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;map&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
#ifndef _WIN32<br>
#include &lt;arpa/inet.h&gt;<br>
#include &lt;netinet/in.h&gt;<br>
#include &lt;sys/socket.h&gt;<br>
#include &lt;sys/types.h&gt;<br>
#include &lt;unistd.h&gt;<br>
#endif<br>
<br>
static std::string json_escape(const std::string &amp;s)<br>
{<br>
    std::string out;<br>
    out.reserve(s.size());<br>
    for (char c : s)<br>
    {<br>
        switch (c)<br>
        {<br>
        case '\\':<br>
            out += &quot;\\\\&quot;;<br>
            break;<br>
        case '&quot;':<br>
            out += &quot;\\\&quot;&quot;;<br>
            break;<br>
        case '\n':<br>
            out += &quot;\\n&quot;;<br>
            break;<br>
        case '\r':<br>
            out += &quot;\\r&quot;;<br>
            break;<br>
        case '\t':<br>
            out += &quot;\\t&quot;;<br>
            break;<br>
        default:<br>
            out += c;<br>
            break;<br>
        }<br>
    }<br>
    return out;<br>
}<br>
<br>
int run_server(const Options &amp;opt)<br>
{<br>
#ifdef _WIN32<br>
    (void)opt;<br>
    std::cerr &lt;&lt; &quot;--server is not supported on Windows yet.\n&quot;;<br>
    return 1;<br>
#else<br>
    if (opt.server_port &lt;= 0 || opt.server_port &gt; 65535)<br>
    {<br>
        std::cerr &lt;&lt; &quot;Invalid port for --server: &quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    // Собираем список файлов так же, как в обычном scat:<br>
    // при наличии config_file — через scat.txt/--config,<br>
    // иначе — из paths.<br>
    std::vector&lt;fs::path&gt; files;<br>
<br>
    if (!opt.config_file.empty())<br>
    {<br>
        Config cfg = parse_config(opt.config_file);<br>
        files = collect_from_rules(cfg.text_rules, opt);<br>
    }<br>
    else<br>
    {<br>
        files = collect_from_paths(opt.paths, opt);<br>
    }<br>
<br>
    if (files.empty())<br>
    {<br>
        std::cerr &lt;&lt; &quot;No files collected for server.\n&quot;;<br>
        return 1;<br>
    }<br>
<br>
    // Ключ: строковый путь (как печатаем в scat), значение: реальный путь<br>
    std::map&lt;std::string, fs::path&gt; file_map;<br>
    for (auto &amp;f : files)<br>
    {<br>
        std::string key = make_display_path(f);<br>
        file_map[key] = f;<br>
    }<br>
<br>
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);<br>
    if (server_fd &lt; 0)<br>
    {<br>
        perror(&quot;socket&quot;);<br>
        return 1;<br>
    }<br>
<br>
    int yes = 1;<br>
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &amp;yes, sizeof(yes));<br>
<br>
    sockaddr_in addr{};<br>
    addr.sin_family = AF_INET;<br>
    addr.sin_addr.s_addr = htonl(INADDR_ANY);<br>
    addr.sin_port = htons(static_cast&lt;uint16_t&gt;(opt.server_port));<br>
<br>
    if (::bind(server_fd, reinterpret_cast&lt;sockaddr *&gt;(&amp;addr), sizeof(addr)) &lt;<br>
        0)<br>
    {<br>
        perror(&quot;bind&quot;);<br>
        ::close(server_fd);<br>
        return 1;<br>
    }<br>
<br>
    if (::listen(server_fd, 16) &lt; 0)<br>
    {<br>
        perror(&quot;listen&quot;);<br>
        ::close(server_fd);<br>
        return 1;<br>
    }<br>
<br>
    std::cout &lt;&lt; &quot;Serving &quot; &lt;&lt; file_map.size()<br>
              &lt;&lt; &quot; files on http://127.0.0.1:&quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;<br>
              &lt;&lt; &quot;Endpoints:\n&quot;<br>
              &lt;&lt; &quot;  /index.json\n&quot;<br>
              &lt;&lt; &quot;  /&lt;path&gt;\n&quot;;<br>
<br>
    for (;;)<br>
    {<br>
        int client = ::accept(server_fd, nullptr, nullptr);<br>
        if (client &lt; 0)<br>
        {<br>
            perror(&quot;accept&quot;);<br>
            continue;<br>
        }<br>
<br>
        std::string request;<br>
        char buf[4096];<br>
        ssize_t n = ::recv(client, buf, sizeof(buf), 0);<br>
        if (n &lt;= 0)<br>
        {<br>
            ::close(client);<br>
            continue;<br>
        }<br>
        request.append(buf, static_cast&lt;std::size_t&gt;(n));<br>
<br>
        std::istringstream req_stream(request);<br>
        std::string method, target, version;<br>
        req_stream &gt;&gt; method &gt;&gt; target &gt;&gt; version;<br>
<br>
        auto send_response = [&amp;](const std::string &amp;status_line,<br>
                                 const std::string &amp;content_type,<br>
                                 const std::string &amp;body)<br>
        {<br>
            std::ostringstream out;<br>
            out &lt;&lt; status_line &lt;&lt; &quot;\r\n&quot;;<br>
            out &lt;&lt; &quot;Content-Type: &quot; &lt;&lt; content_type &lt;&lt; &quot;\r\n&quot;;<br>
            out &lt;&lt; &quot;Content-Length: &quot; &lt;&lt; body.size() &lt;&lt; &quot;\r\n&quot;;<br>
            out &lt;&lt; &quot;Connection: close\r\n&quot;;<br>
            out &lt;&lt; &quot;\r\n&quot;;<br>
            out &lt;&lt; body;<br>
            std::string s = out.str();<br>
            ::send(client, s.data(), s.size(), 0);<br>
        };<br>
<br>
        if (method != &quot;GET&quot;)<br>
        {<br>
            send_response(&quot;HTTP/1.0 405 Method Not Allowed&quot;,<br>
                          &quot;text/plain; charset=utf-8&quot;,<br>
                          &quot;Only GET is supported\n&quot;);<br>
            ::close(client);<br>
            continue;<br>
        }<br>
<br>
        if (target.empty())<br>
            target = &quot;/&quot;;<br>
<br>
        // Корневая страница с краткой подсказкой<br>
        if (target == &quot;/&quot;)<br>
        {<br>
            std::ostringstream body;<br>
            body &lt;&lt; &quot;&lt;!DOCTYPE html&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;html&gt;&lt;head&gt;&lt;meta charset=\&quot;utf-8\&quot;&gt;&quot;<br>
                 &lt;&lt; &quot;&lt;title&gt;scat server&lt;/title&gt;&lt;/head&gt;&lt;body&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;h1&gt;scat server&lt;/h1&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;p&gt;This is a read-only file server started by &quot;<br>
                 &lt;&lt; &quot;&lt;code&gt;scat --server PORT&lt;/code&gt;.&lt;/p&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;ul&gt;\n&quot;<br>
                 &lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/index.json&lt;/code&gt; – list of files as &quot;<br>
                    &quot;JSON.&lt;/li&gt;\n&quot;<br>
                 &lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/{relative-path}&lt;/code&gt; – raw file contents. &quot;<br>
                 &lt;&lt; &quot;Path must match an entry from &quot;<br>
                    &quot;&lt;code&gt;index.json&lt;/code&gt;.&lt;/li&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;/ul&gt;\n&quot;<br>
                 &lt;&lt; &quot;&lt;/body&gt;&lt;/html&gt;\n&quot;;<br>
<br>
            send_response(<br>
                &quot;HTTP/1.0 200 OK&quot;, &quot;text/html; charset=utf-8&quot;, body.str());<br>
            ::close(client);<br>
            continue;<br>
        }<br>
<br>
        if (target == &quot;/index.json&quot;)<br>
        {<br>
            std::ostringstream body;<br>
            body &lt;&lt; &quot;{\n  \&quot;files\&quot;: [\n&quot;;<br>
            bool first = true;<br>
            for (const auto &amp;[name, _] : file_map)<br>
            {<br>
                if (!first)<br>
                    body &lt;&lt; &quot;,\n&quot;;<br>
                first = false;<br>
                body &lt;&lt; &quot;    \&quot;&quot; &lt;&lt; json_escape(name) &lt;&lt; &quot;\&quot;&quot;;<br>
            }<br>
            body &lt;&lt; &quot;\n  ]\n}\n&quot;;<br>
<br>
            send_response(&quot;HTTP/1.0 200 OK&quot;,<br>
                          &quot;application/json; charset=utf-8&quot;,<br>
                          body.str());<br>
            ::close(client);<br>
            continue;<br>
        }<br>
<br>
        if (!target.empty() &amp;&amp; target[0] == '/')<br>
            target.erase(0, 1);<br>
<br>
        auto it = file_map.find(target);<br>
        if (it == file_map.end())<br>
        {<br>
            send_response(&quot;HTTP/1.0 404 Not Found&quot;,<br>
                          &quot;text/plain; charset=utf-8&quot;,<br>
                          &quot;Not found\n&quot;);<br>
            ::close(client);<br>
            continue;<br>
        }<br>
<br>
        std::ifstream in(it-&gt;second, std::ios::binary);<br>
        if (!in)<br>
        {<br>
            send_response(&quot;HTTP/1.0 500 Internal Server Error&quot;,<br>
                          &quot;text/plain; charset=utf-8&quot;,<br>
                          &quot;Cannot open file\n&quot;);<br>
            ::close(client);<br>
            continue;<br>
        }<br>
<br>
        std::ostringstream body;<br>
        body &lt;&lt; in.rdbuf();<br>
<br>
        send_response(<br>
            &quot;HTTP/1.0 200 OK&quot;, &quot;text/plain; charset=utf-8&quot;, body.str());<br>
        ::close(client);<br>
    }<br>
<br>
    // теоретически недостижимо<br>
    ::close(server_fd);<br>
    return 0;<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

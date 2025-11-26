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
&#9;std::string out;<br>
&#9;out.reserve(s.size());<br>
&#9;for (char c : s)<br>
&#9;{<br>
&#9;&#9;switch (c)<br>
&#9;&#9;{<br>
&#9;&#9;case '\\':<br>
&#9;&#9;&#9;out += &quot;\\\\&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '&quot;':<br>
&#9;&#9;&#9;out += &quot;\\\&quot;&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '\n':<br>
&#9;&#9;&#9;out += &quot;\\n&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '\r':<br>
&#9;&#9;&#9;out += &quot;\\r&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;case '\t':<br>
&#9;&#9;&#9;out += &quot;\\t&quot;;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;default:<br>
&#9;&#9;&#9;out += c;<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;}<br>
&#9;}<br>
&#9;return out;<br>
}<br>
<br>
int run_server(const Options &amp;opt)<br>
{<br>
#ifdef _WIN32<br>
&#9;(void)opt;<br>
&#9;std::cerr &lt;&lt; &quot;--server is not supported on Windows yet.\n&quot;;<br>
&#9;return 1;<br>
#else<br>
&#9;if (opt.server_port &lt;= 0 || opt.server_port &gt; 65535)<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;Invalid port for --server: &quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;// Собираем список файлов так же, как в обычном scat:<br>
&#9;// при наличии config_file — через scat.txt/--config,<br>
&#9;// иначе — из paths.<br>
&#9;std::vector&lt;fs::path&gt; files;<br>
<br>
&#9;if (!opt.config_file.empty())<br>
&#9;{<br>
&#9;&#9;Config cfg = parse_config(opt.config_file);<br>
&#9;&#9;files = collect_from_rules(cfg.text_rules, opt);<br>
&#9;}<br>
&#9;else<br>
&#9;{<br>
&#9;&#9;files = collect_from_paths(opt.paths, opt);<br>
&#9;}<br>
<br>
&#9;if (files.empty())<br>
&#9;{<br>
&#9;&#9;std::cerr &lt;&lt; &quot;No files collected for server.\n&quot;;<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;// Ключ: строковый путь (как печатаем в scat), значение: реальный путь<br>
&#9;std::map&lt;std::string, fs::path&gt; file_map;<br>
&#9;for (auto &amp;f : files)<br>
&#9;{<br>
&#9;&#9;std::string key = make_display_path(f);<br>
&#9;&#9;file_map[key] = f;<br>
&#9;}<br>
<br>
&#9;int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);<br>
&#9;if (server_fd &lt; 0)<br>
&#9;{<br>
&#9;&#9;perror(&quot;socket&quot;);<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;int yes = 1;<br>
&#9;::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &amp;yes, sizeof(yes));<br>
<br>
&#9;sockaddr_in addr{};<br>
&#9;addr.sin_family = AF_INET;<br>
&#9;addr.sin_addr.s_addr = htonl(INADDR_ANY);<br>
&#9;addr.sin_port = htons(static_cast&lt;uint16_t&gt;(opt.server_port));<br>
<br>
&#9;if (::bind(server_fd, reinterpret_cast&lt;sockaddr *&gt;(&amp;addr), sizeof(addr)) &lt;<br>
&#9;&#9;0)<br>
&#9;{<br>
&#9;&#9;perror(&quot;bind&quot;);<br>
&#9;&#9;::close(server_fd);<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;if (::listen(server_fd, 16) &lt; 0)<br>
&#9;{<br>
&#9;&#9;perror(&quot;listen&quot;);<br>
&#9;&#9;::close(server_fd);<br>
&#9;&#9;return 1;<br>
&#9;}<br>
<br>
&#9;std::cout &lt;&lt; &quot;Serving &quot; &lt;&lt; file_map.size()<br>
&#9;&#9;&#9;&lt;&lt; &quot; files on http://127.0.0.1:&quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;<br>
&#9;&#9;&#9;&lt;&lt; &quot;Endpoints:\n&quot;<br>
&#9;&#9;&#9;&lt;&lt; &quot;  /index.json\n&quot;<br>
&#9;&#9;&#9;&lt;&lt; &quot;  /&lt;path&gt;\n&quot;;<br>
<br>
&#9;for (;;)<br>
&#9;{<br>
&#9;&#9;int client = ::accept(server_fd, nullptr, nullptr);<br>
&#9;&#9;if (client &lt; 0)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;perror(&quot;accept&quot;);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::string request;<br>
&#9;&#9;char buf[4096];<br>
&#9;&#9;ssize_t n = ::recv(client, buf, sizeof(buf), 0);<br>
&#9;&#9;if (n &lt;= 0)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
&#9;&#9;request.append(buf, static_cast&lt;std::size_t&gt;(n));<br>
<br>
&#9;&#9;std::istringstream req_stream(request);<br>
&#9;&#9;std::string method, target, version;<br>
&#9;&#9;req_stream &gt;&gt; method &gt;&gt; target &gt;&gt; version;<br>
<br>
&#9;&#9;auto send_response = [&amp;](const std::string &amp;status_line,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::string &amp;content_type,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::string &amp;body)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::ostringstream out;<br>
&#9;&#9;&#9;out &lt;&lt; status_line &lt;&lt; &quot;\r\n&quot;;<br>
&#9;&#9;&#9;out &lt;&lt; &quot;Content-Type: &quot; &lt;&lt; content_type &lt;&lt; &quot;\r\n&quot;;<br>
&#9;&#9;&#9;out &lt;&lt; &quot;Content-Length: &quot; &lt;&lt; body.size() &lt;&lt; &quot;\r\n&quot;;<br>
&#9;&#9;&#9;out &lt;&lt; &quot;Connection: close\r\n&quot;;<br>
&#9;&#9;&#9;out &lt;&lt; &quot;\r\n&quot;;<br>
&#9;&#9;&#9;out &lt;&lt; body;<br>
&#9;&#9;&#9;std::string s = out.str();<br>
&#9;&#9;&#9;::send(client, s.data(), s.size(), 0);<br>
&#9;&#9;};<br>
<br>
&#9;&#9;if (method != &quot;GET&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;send_response(&quot;HTTP/1.0 405 Method Not Allowed&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;text/plain; charset=utf-8&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;Only GET is supported\n&quot;);<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (target.empty())<br>
&#9;&#9;&#9;target = &quot;/&quot;;<br>
<br>
&#9;&#9;// Корневая страница с краткой подсказкой<br>
&#9;&#9;if (target == &quot;/&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::ostringstream body;<br>
&#9;&#9;&#9;body &lt;&lt; &quot;&lt;!DOCTYPE html&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;html&gt;&lt;head&gt;&lt;meta charset=\&quot;utf-8\&quot;&gt;&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;title&gt;scat server&lt;/title&gt;&lt;/head&gt;&lt;body&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;h1&gt;scat server&lt;/h1&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;p&gt;This is a read-only file server started by &quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;code&gt;scat --server PORT&lt;/code&gt;.&lt;/p&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;ul&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/index.json&lt;/code&gt; – list of files as &quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;JSON.&lt;/li&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/{relative-path}&lt;/code&gt; – raw file contents. &quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;Path must match an entry from &quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;&lt;code&gt;index.json&lt;/code&gt;.&lt;/li&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;/ul&gt;\n&quot;<br>
&#9;&#9;&#9;&#9;&lt;&lt; &quot;&lt;/body&gt;&lt;/html&gt;\n&quot;;<br>
<br>
&#9;&#9;&#9;send_response(<br>
&#9;&#9;&#9;&#9;&quot;HTTP/1.0 200 OK&quot;, &quot;text/html; charset=utf-8&quot;, body.str());<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (target == &quot;/index.json&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::ostringstream body;<br>
&#9;&#9;&#9;body &lt;&lt; &quot;{\n  \&quot;files\&quot;: [\n&quot;;<br>
&#9;&#9;&#9;bool first = true;<br>
&#9;&#9;&#9;for (const auto &amp;[name, _] : file_map)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;if (!first)<br>
&#9;&#9;&#9;&#9;&#9;body &lt;&lt; &quot;,\n&quot;;<br>
&#9;&#9;&#9;&#9;first = false;<br>
&#9;&#9;&#9;&#9;body &lt;&lt; &quot;    \&quot;&quot; &lt;&lt; json_escape(name) &lt;&lt; &quot;\&quot;&quot;;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;body &lt;&lt; &quot;\n  ]\n}\n&quot;;<br>
<br>
&#9;&#9;&#9;send_response(&quot;HTTP/1.0 200 OK&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;application/json; charset=utf-8&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;body.str());<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (!target.empty() &amp;&amp; target[0] == '/')<br>
&#9;&#9;&#9;target.erase(0, 1);<br>
<br>
&#9;&#9;auto it = file_map.find(target);<br>
&#9;&#9;if (it == file_map.end())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;send_response(&quot;HTTP/1.0 404 Not Found&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;text/plain; charset=utf-8&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;Not found\n&quot;);<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::ifstream in(it-&gt;second, std::ios::binary);<br>
&#9;&#9;if (!in)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;send_response(&quot;HTTP/1.0 500 Internal Server Error&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;text/plain; charset=utf-8&quot;,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&quot;Cannot open file\n&quot;);<br>
&#9;&#9;&#9;::close(client);<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;std::ostringstream body;<br>
&#9;&#9;body &lt;&lt; in.rdbuf();<br>
<br>
&#9;&#9;send_response(<br>
&#9;&#9;&#9;&quot;HTTP/1.0 200 OK&quot;, &quot;text/plain; charset=utf-8&quot;, body.str());<br>
&#9;&#9;::close(client);<br>
&#9;}<br>
<br>
&#9;// теоретически недостижимо<br>
&#9;::close(server_fd);<br>
&#9;return 0;<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

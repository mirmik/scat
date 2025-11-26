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
&emsp;std::string out;<br>
&emsp;out.reserve(s.size());<br>
&emsp;for (char c : s)<br>
&emsp;{<br>
&emsp;&emsp;switch (c)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;case '\\':<br>
&emsp;&emsp;&emsp;out += &quot;\\\\&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '&quot;':<br>
&emsp;&emsp;&emsp;out += &quot;\\\&quot;&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '\n':<br>
&emsp;&emsp;&emsp;out += &quot;\\n&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '\r':<br>
&emsp;&emsp;&emsp;out += &quot;\\r&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;case '\t':<br>
&emsp;&emsp;&emsp;out += &quot;\\t&quot;;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;default:<br>
&emsp;&emsp;&emsp;out += c;<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;}<br>
&emsp;}<br>
&emsp;return out;<br>
}<br>
<br>
int run_server(const Options &amp;opt)<br>
{<br>
#ifdef _WIN32<br>
&emsp;(void)opt;<br>
&emsp;std::cerr &lt;&lt; &quot;--server is not supported on Windows yet.\n&quot;;<br>
&emsp;return 1;<br>
#else<br>
&emsp;if (opt.server_port &lt;= 0 || opt.server_port &gt; 65535)<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;Invalid port for --server: &quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;// Собираем список файлов так же, как в обычном scat:<br>
&emsp;// при наличии config_file — через scat.txt/--config,<br>
&emsp;// иначе — из paths.<br>
&emsp;std::vector&lt;fs::path&gt; files;<br>
<br>
&emsp;if (!opt.config_file.empty())<br>
&emsp;{<br>
&emsp;&emsp;Config cfg = parse_config(opt.config_file);<br>
&emsp;&emsp;files = collect_from_rules(cfg.text_rules, opt);<br>
&emsp;}<br>
&emsp;else<br>
&emsp;{<br>
&emsp;&emsp;files = collect_from_paths(opt.paths, opt);<br>
&emsp;}<br>
<br>
&emsp;if (files.empty())<br>
&emsp;{<br>
&emsp;&emsp;std::cerr &lt;&lt; &quot;No files collected for server.\n&quot;;<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;// Ключ: строковый путь (как печатаем в scat), значение: реальный путь<br>
&emsp;std::map&lt;std::string, fs::path&gt; file_map;<br>
&emsp;for (auto &amp;f : files)<br>
&emsp;{<br>
&emsp;&emsp;std::string key = make_display_path(f);<br>
&emsp;&emsp;file_map[key] = f;<br>
&emsp;}<br>
<br>
&emsp;int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);<br>
&emsp;if (server_fd &lt; 0)<br>
&emsp;{<br>
&emsp;&emsp;perror(&quot;socket&quot;);<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;int yes = 1;<br>
&emsp;::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &amp;yes, sizeof(yes));<br>
<br>
&emsp;sockaddr_in addr{};<br>
&emsp;addr.sin_family = AF_INET;<br>
&emsp;addr.sin_addr.s_addr = htonl(INADDR_ANY);<br>
&emsp;addr.sin_port = htons(static_cast&lt;uint16_t&gt;(opt.server_port));<br>
<br>
&emsp;if (::bind(server_fd, reinterpret_cast&lt;sockaddr *&gt;(&amp;addr), sizeof(addr)) &lt;<br>
&emsp;&emsp;0)<br>
&emsp;{<br>
&emsp;&emsp;perror(&quot;bind&quot;);<br>
&emsp;&emsp;::close(server_fd);<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;if (::listen(server_fd, 16) &lt; 0)<br>
&emsp;{<br>
&emsp;&emsp;perror(&quot;listen&quot;);<br>
&emsp;&emsp;::close(server_fd);<br>
&emsp;&emsp;return 1;<br>
&emsp;}<br>
<br>
&emsp;std::cout &lt;&lt; &quot;Serving &quot; &lt;&lt; file_map.size()<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot; files on http://127.0.0.1:&quot; &lt;&lt; opt.server_port &lt;&lt; &quot;\n&quot;<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot;Endpoints:\n&quot;<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot;  /index.json\n&quot;<br>
&emsp;&emsp;&emsp;&lt;&lt; &quot;  /&lt;path&gt;\n&quot;;<br>
<br>
&emsp;for (;;)<br>
&emsp;{<br>
&emsp;&emsp;int client = ::accept(server_fd, nullptr, nullptr);<br>
&emsp;&emsp;if (client &lt; 0)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;perror(&quot;accept&quot;);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::string request;<br>
&emsp;&emsp;char buf[4096];<br>
&emsp;&emsp;ssize_t n = ::recv(client, buf, sizeof(buf), 0);<br>
&emsp;&emsp;if (n &lt;= 0)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;request.append(buf, static_cast&lt;std::size_t&gt;(n));<br>
<br>
&emsp;&emsp;std::istringstream req_stream(request);<br>
&emsp;&emsp;std::string method, target, version;<br>
&emsp;&emsp;req_stream &gt;&gt; method &gt;&gt; target &gt;&gt; version;<br>
<br>
&emsp;&emsp;auto send_response = [&amp;](const std::string &amp;status_line,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;content_type,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;body)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::ostringstream out;<br>
&emsp;&emsp;&emsp;out &lt;&lt; status_line &lt;&lt; &quot;\r\n&quot;;<br>
&emsp;&emsp;&emsp;out &lt;&lt; &quot;Content-Type: &quot; &lt;&lt; content_type &lt;&lt; &quot;\r\n&quot;;<br>
&emsp;&emsp;&emsp;out &lt;&lt; &quot;Content-Length: &quot; &lt;&lt; body.size() &lt;&lt; &quot;\r\n&quot;;<br>
&emsp;&emsp;&emsp;out &lt;&lt; &quot;Connection: close\r\n&quot;;<br>
&emsp;&emsp;&emsp;out &lt;&lt; &quot;\r\n&quot;;<br>
&emsp;&emsp;&emsp;out &lt;&lt; body;<br>
&emsp;&emsp;&emsp;std::string s = out.str();<br>
&emsp;&emsp;&emsp;::send(client, s.data(), s.size(), 0);<br>
&emsp;&emsp;};<br>
<br>
&emsp;&emsp;if (method != &quot;GET&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;send_response(&quot;HTTP/1.0 405 Method Not Allowed&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;text/plain; charset=utf-8&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;Only GET is supported\n&quot;);<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (target.empty())<br>
&emsp;&emsp;&emsp;target = &quot;/&quot;;<br>
<br>
&emsp;&emsp;// Корневая страница с краткой подсказкой<br>
&emsp;&emsp;if (target == &quot;/&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::ostringstream body;<br>
&emsp;&emsp;&emsp;body &lt;&lt; &quot;&lt;!DOCTYPE html&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;html&gt;&lt;head&gt;&lt;meta charset=\&quot;utf-8\&quot;&gt;&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;title&gt;scat server&lt;/title&gt;&lt;/head&gt;&lt;body&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;h1&gt;scat server&lt;/h1&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;p&gt;This is a read-only file server started by &quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;code&gt;scat --server PORT&lt;/code&gt;.&lt;/p&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;ul&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/index.json&lt;/code&gt; – list of files as &quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;JSON.&lt;/li&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;  &lt;li&gt;&lt;code&gt;/{relative-path}&lt;/code&gt; – raw file contents. &quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;Path must match an entry from &quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;&lt;code&gt;index.json&lt;/code&gt;.&lt;/li&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;/ul&gt;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&lt;&lt; &quot;&lt;/body&gt;&lt;/html&gt;\n&quot;;<br>
<br>
&emsp;&emsp;&emsp;send_response(<br>
&emsp;&emsp;&emsp;&emsp;&quot;HTTP/1.0 200 OK&quot;, &quot;text/html; charset=utf-8&quot;, body.str());<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (target == &quot;/index.json&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::ostringstream body;<br>
&emsp;&emsp;&emsp;body &lt;&lt; &quot;{\n  \&quot;files\&quot;: [\n&quot;;<br>
&emsp;&emsp;&emsp;bool first = true;<br>
&emsp;&emsp;&emsp;for (const auto &amp;[name, _] : file_map)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;if (!first)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;body &lt;&lt; &quot;,\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;first = false;<br>
&emsp;&emsp;&emsp;&emsp;body &lt;&lt; &quot;    \&quot;&quot; &lt;&lt; json_escape(name) &lt;&lt; &quot;\&quot;&quot;;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;body &lt;&lt; &quot;\n  ]\n}\n&quot;;<br>
<br>
&emsp;&emsp;&emsp;send_response(&quot;HTTP/1.0 200 OK&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;application/json; charset=utf-8&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;body.str());<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (!target.empty() &amp;&amp; target[0] == '/')<br>
&emsp;&emsp;&emsp;target.erase(0, 1);<br>
<br>
&emsp;&emsp;auto it = file_map.find(target);<br>
&emsp;&emsp;if (it == file_map.end())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;send_response(&quot;HTTP/1.0 404 Not Found&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;text/plain; charset=utf-8&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;Not found\n&quot;);<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::ifstream in(it-&gt;second, std::ios::binary);<br>
&emsp;&emsp;if (!in)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;send_response(&quot;HTTP/1.0 500 Internal Server Error&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;text/plain; charset=utf-8&quot;,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&quot;Cannot open file\n&quot;);<br>
&emsp;&emsp;&emsp;::close(client);<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;std::ostringstream body;<br>
&emsp;&emsp;body &lt;&lt; in.rdbuf();<br>
<br>
&emsp;&emsp;send_response(<br>
&emsp;&emsp;&emsp;&quot;HTTP/1.0 200 OK&quot;, &quot;text/plain; charset=utf-8&quot;, body.str());<br>
&emsp;&emsp;::close(client);<br>
&emsp;}<br>
<br>
&emsp;// теоретически недостижимо<br>
&emsp;::close(server_fd);<br>
&emsp;return 0;<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

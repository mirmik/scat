<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;options.h&quot;<br>
#include &lt;cstdlib&gt;<br>
#include &lt;iostream&gt;<br>
<br>
static void print_help()<br>
{<br>
&#9;std::cout &lt;&lt; &quot;Usage: scat [options] [paths...]\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;Options:\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  -r            Recursive directory processing\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  -l            List files only\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --size        Show file sizes in -l output\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --sorted      Sort list (-l) by size (desc)\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  -n            Show line numbers\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --abs         Show absolute paths\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --config F    Read patterns from file F\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --apply F     Apply patch from file F\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --apply-stdin Apply patch from stdin\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --server P    Run HTTP server on port P\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  -c, --chunk   Print chunk trailer after output\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --chunk-help  Show chunk v2 help\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --wrap DIR    Wrap collected files as HTML into DIR\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --prefix P    Prepend P before file paths in -l output\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --git-info    Print git commit hash and remote origin\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --ghmap       List raw.githubusercontent.com URLs for &quot;<br>
&#9;&#9;&#9;&#9;&quot;current commit\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --copy        Copy all stdout output to system clipboard\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  --hook-install Install or update .git/hooks/pre-commit for &quot;<br>
&#9;&#9;&#9;&#9;&quot;scat wrap\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;  -h, --help    Show this help\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&#9;&quot;If no paths are given, scat reads patterns from scat.txt.\n&quot;;<br>
}<br>
<br>
Options parse_options(int argc, char **argv)<br>
{<br>
&#9;Options opt;<br>
<br>
&#9;for (int i = 1; i &lt; argc; ++i)<br>
&#9;{<br>
&#9;&#9;std::string a = argv[i];<br>
&#9;&#9;if (a == &quot;-r&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.recursive = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;-l&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.list_only = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--size&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.show_size = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;-n&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.line_numbers = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--abs&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.abs_paths = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--hook-install&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.hook_install = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--sorted&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.sorted = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a.rfind(&quot;--prefix=&quot;, 0) == 0)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.path_prefix = a.substr(std::string(&quot;--prefix=&quot;).size());<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--prefix&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (i + 1 &lt; argc)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;opt.path_prefix = argv[++i];<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--prefix requires value\n&quot;;<br>
&#9;&#9;&#9;&#9;std::exit(1);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--server&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (i + 1 &lt; argc)<br>
&#9;&#9;&#9;&#9;opt.server_port = std::atoi(argv[++i]);<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--server requires port\n&quot;;<br>
&#9;&#9;&#9;&#9;std::exit(1);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--chunk-help&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.chunk_help = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;-c&quot; || a == &quot;--chunk&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.chunk_trailer = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--apply-stdin&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.apply_stdin = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--apply&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (i + 1 &lt; argc)<br>
&#9;&#9;&#9;&#9;opt.apply_file = argv[++i];<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--apply requires file\n&quot;;<br>
&#9;&#9;&#9;&#9;std::exit(1);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--config&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (i + 1 &lt; argc)<br>
&#9;&#9;&#9;&#9;opt.config_file = argv[++i];<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--config requires file\n&quot;;<br>
&#9;&#9;&#9;&#9;std::exit(1);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--git-info&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.git_info = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--ghmap&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.gh_map = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--copy&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.copy_out = true;<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;-h&quot; || a == &quot;--help&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;print_help();<br>
&#9;&#9;&#9;std::exit(0);<br>
&#9;&#9;}<br>
&#9;&#9;else if (a == &quot;--wrap&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (i + 1 &lt; argc)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;opt.wrap_root = argv[++i];<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;std::cerr &lt;&lt; &quot;--wrap requires directory name\n&quot;;<br>
&#9;&#9;&#9;&#9;std::exit(1);<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;else<br>
&#9;&#9;{<br>
&#9;&#9;&#9;opt.paths.push_back(a);<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;// auto-config mode if no paths and no explicit config<br>
&#9;if (opt.paths.empty() &amp;&amp; opt.config_file.empty())<br>
&#9;&#9;opt.config_file = &quot;scat.txt&quot;;<br>
<br>
&#9;return opt;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

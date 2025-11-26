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
&emsp;std::cout &lt;&lt; &quot;Usage: scat [options] [paths...]\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;Options:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  -r            Recursive directory processing\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  -l            List files only\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --size        Show file sizes in -l output\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --sorted      Sort list (-l) by size (desc)\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  -n            Show line numbers\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --abs         Show absolute paths\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --config F    Read patterns from file F\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --apply F     Apply patch from file F\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --apply-stdin Apply patch from stdin\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --server P    Run HTTP server on port P\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  -c, --chunk   Print chunk trailer after output\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --chunk-help  Show chunk v2 help\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --wrap DIR    Wrap collected files as HTML into DIR\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --prefix P    Prepend P before file paths in -l output\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --git-info    Print git commit hash and remote origin\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --ghmap       List raw.githubusercontent.com URLs for &quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;current commit\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --copy        Copy all stdout output to system clipboard\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  --hook-install Install or update .git/hooks/pre-commit for &quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;scat wrap\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;  -h, --help    Show this help\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&quot;If no paths are given, scat reads patterns from scat.txt.\n&quot;;<br>
}<br>
<br>
Options parse_options(int argc, char **argv)<br>
{<br>
&emsp;Options opt;<br>
<br>
&emsp;for (int i = 1; i &lt; argc; ++i)<br>
&emsp;{<br>
&emsp;&emsp;std::string a = argv[i];<br>
&emsp;&emsp;if (a == &quot;-r&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.recursive = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;-l&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.list_only = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--size&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.show_size = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;-n&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.line_numbers = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--abs&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.abs_paths = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--hook-install&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.hook_install = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--sorted&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.sorted = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a.rfind(&quot;--prefix=&quot;, 0) == 0)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.path_prefix = a.substr(std::string(&quot;--prefix=&quot;).size());<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--prefix&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (i + 1 &lt; argc)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;opt.path_prefix = argv[++i];<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--prefix requires value\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;std::exit(1);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--server&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (i + 1 &lt; argc)<br>
&emsp;&emsp;&emsp;&emsp;opt.server_port = std::atoi(argv[++i]);<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--server requires port\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;std::exit(1);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--chunk-help&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.chunk_help = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;-c&quot; || a == &quot;--chunk&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.chunk_trailer = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--apply-stdin&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.apply_stdin = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--apply&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (i + 1 &lt; argc)<br>
&emsp;&emsp;&emsp;&emsp;opt.apply_file = argv[++i];<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--apply requires file\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;std::exit(1);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--config&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (i + 1 &lt; argc)<br>
&emsp;&emsp;&emsp;&emsp;opt.config_file = argv[++i];<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--config requires file\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;std::exit(1);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--git-info&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.git_info = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--ghmap&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.gh_map = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--copy&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.copy_out = true;<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;-h&quot; || a == &quot;--help&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;print_help();<br>
&emsp;&emsp;&emsp;std::exit(0);<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (a == &quot;--wrap&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (i + 1 &lt; argc)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;opt.wrap_root = argv[++i];<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;--wrap requires directory name\n&quot;;<br>
&emsp;&emsp;&emsp;&emsp;std::exit(1);<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;opt.paths.push_back(a);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;// auto-config mode if no paths and no explicit config<br>
&emsp;if (opt.paths.empty() &amp;&amp; opt.config_file.empty())<br>
&emsp;&emsp;opt.config_file = &quot;scat.txt&quot;;<br>
<br>
&emsp;return opt;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

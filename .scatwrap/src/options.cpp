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
    std::cout &lt;&lt; &quot;Usage: scat [options] [paths...]\n&quot;<br>
                 &quot;\n&quot;<br>
                 &quot;Options:\n&quot;<br>
                 &quot;  -r            Recursive directory processing\n&quot;<br>
                 &quot;  -l            List files only\n&quot;<br>
                 &quot;  --size        Show file sizes in -l output\n&quot;<br>
                 &quot;  --sorted      Sort list (-l) by size (desc)\n&quot;<br>
                 &quot;  -n            Show line numbers\n&quot;<br>
                 &quot;  --abs         Show absolute paths\n&quot;<br>
                 &quot;  --config F    Read patterns from file F\n&quot;<br>
                 &quot;  --apply F     Apply patch from file F\n&quot;<br>
                 &quot;  --apply-stdin Apply patch from stdin\n&quot;<br>
                 &quot;  --server P    Run HTTP server on port P\n&quot;<br>
                 &quot;  -c, --chunk   Print chunk trailer after output\n&quot;<br>
                 &quot;  --chunk-help  Show chunk v2 help\n&quot;<br>
                 &quot;  --wrap DIR    Wrap collected files as HTML into DIR\n&quot;<br>
                 &quot;  --prefix P    Prepend P before file paths in -l output\n&quot;<br>
                 &quot;  --git-info    Print git commit hash and remote origin\n&quot;<br>
                 &quot;  --ghmap       List raw.githubusercontent.com URLs for &quot;<br>
                 &quot;current commit\n&quot;<br>
                 &quot;  --copy        Copy all stdout output to system clipboard\n&quot;<br>
                 &quot;  --hook-install Install or update .git/hooks/pre-commit for &quot;<br>
                 &quot;scat wrap\n&quot;<br>
                 &quot;  -h, --help    Show this help\n&quot;<br>
                 &quot;\n&quot;<br>
                 &quot;If no paths are given, scat reads patterns from scat.txt.\n&quot;;<br>
}<br>
<br>
Options parse_options(int argc, char **argv)<br>
{<br>
    Options opt;<br>
<br>
    for (int i = 1; i &lt; argc; ++i)<br>
    {<br>
        std::string a = argv[i];<br>
        if (a == &quot;-r&quot;)<br>
        {<br>
            opt.recursive = true;<br>
        }<br>
        else if (a == &quot;-l&quot;)<br>
        {<br>
            opt.list_only = true;<br>
        }<br>
        else if (a == &quot;--size&quot;)<br>
        {<br>
            opt.show_size = true;<br>
        }<br>
        else if (a == &quot;-n&quot;)<br>
        {<br>
            opt.line_numbers = true;<br>
        }<br>
        else if (a == &quot;--abs&quot;)<br>
        {<br>
            opt.abs_paths = true;<br>
        }<br>
        else if (a == &quot;--hook-install&quot;)<br>
        {<br>
            opt.hook_install = true;<br>
        }<br>
        else if (a == &quot;--sorted&quot;)<br>
        {<br>
            opt.sorted = true;<br>
        }<br>
        else if (a.rfind(&quot;--prefix=&quot;, 0) == 0)<br>
        {<br>
            opt.path_prefix = a.substr(std::string(&quot;--prefix=&quot;).size());<br>
        }<br>
        else if (a == &quot;--prefix&quot;)<br>
        {<br>
            if (i + 1 &lt; argc)<br>
            {<br>
                opt.path_prefix = argv[++i];<br>
            }<br>
            else<br>
            {<br>
                std::cerr &lt;&lt; &quot;--prefix requires value\n&quot;;<br>
                std::exit(1);<br>
            }<br>
        }<br>
        else if (a == &quot;--server&quot;)<br>
        {<br>
            if (i + 1 &lt; argc)<br>
                opt.server_port = std::atoi(argv[++i]);<br>
            else<br>
            {<br>
                std::cerr &lt;&lt; &quot;--server requires port\n&quot;;<br>
                std::exit(1);<br>
            }<br>
        }<br>
        else if (a == &quot;--chunk-help&quot;)<br>
        {<br>
            opt.chunk_help = true;<br>
        }<br>
        else if (a == &quot;-c&quot; || a == &quot;--chunk&quot;)<br>
        {<br>
            opt.chunk_trailer = true;<br>
        }<br>
        else if (a == &quot;--apply-stdin&quot;)<br>
        {<br>
            opt.apply_stdin = true;<br>
        }<br>
        else if (a == &quot;--apply&quot;)<br>
        {<br>
            if (i + 1 &lt; argc)<br>
                opt.apply_file = argv[++i];<br>
            else<br>
            {<br>
                std::cerr &lt;&lt; &quot;--apply requires file\n&quot;;<br>
                std::exit(1);<br>
            }<br>
        }<br>
        else if (a == &quot;--config&quot;)<br>
        {<br>
            if (i + 1 &lt; argc)<br>
                opt.config_file = argv[++i];<br>
            else<br>
            {<br>
                std::cerr &lt;&lt; &quot;--config requires file\n&quot;;<br>
                std::exit(1);<br>
            }<br>
        }<br>
        else if (a == &quot;--git-info&quot;)<br>
        {<br>
            opt.git_info = true;<br>
        }<br>
        else if (a == &quot;--ghmap&quot;)<br>
        {<br>
            opt.gh_map = true;<br>
        }<br>
        else if (a == &quot;--copy&quot;)<br>
        {<br>
            opt.copy_out = true;<br>
        }<br>
        else if (a == &quot;-h&quot; || a == &quot;--help&quot;)<br>
        {<br>
            print_help();<br>
            std::exit(0);<br>
        }<br>
        else if (a == &quot;--wrap&quot;)<br>
        {<br>
            if (i + 1 &lt; argc)<br>
            {<br>
                opt.wrap_root = argv[++i];<br>
            }<br>
            else<br>
            {<br>
                std::cerr &lt;&lt; &quot;--wrap requires directory name\n&quot;;<br>
                std::exit(1);<br>
            }<br>
        }<br>
        else<br>
        {<br>
            opt.paths.push_back(a);<br>
        }<br>
    }<br>
<br>
    // auto-config mode if no paths and no explicit config<br>
    if (opt.paths.empty() &amp;&amp; opt.config_file.empty())<br>
        opt.config_file = &quot;scat.txt&quot;;<br>
<br>
    return opt;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

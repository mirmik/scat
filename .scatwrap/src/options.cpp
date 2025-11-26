<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.cpp</title>
</head>
<body>
<pre><code>
#include &quot;options.h&quot;
#include &lt;cstdlib&gt;
#include &lt;iostream&gt;

static void print_help()
{
    std::cout &lt;&lt; &quot;Usage: scat [options] [paths...]\n&quot;
                 &quot;\n&quot;
                 &quot;Options:\n&quot;
                 &quot;  -r            Recursive directory processing\n&quot;
                 &quot;  -l            List files only\n&quot;
                 &quot;  --size        Show file sizes in -l output\n&quot;
                 &quot;  --sorted      Sort list (-l) by size (desc)\n&quot;
                 &quot;  -n            Show line numbers\n&quot;
                 &quot;  --abs         Show absolute paths\n&quot;
                 &quot;  --config F    Read patterns from file F\n&quot;
                 &quot;  --apply F     Apply patch from file F\n&quot;
                 &quot;  --apply-stdin Apply patch from stdin\n&quot;
                 &quot;  --server P    Run HTTP server on port P\n&quot;
                 &quot;  -c, --chunk   Print chunk trailer after output\n&quot;
                 &quot;  --chunk-help  Show chunk v2 help\n&quot;
                 &quot;  --wrap DIR    Wrap collected files as HTML into DIR\n&quot;
                 &quot;  --prefix P    Prepend P before file paths in -l output\n&quot;
                 &quot;  --git-info    Print git commit hash and remote origin\n&quot;
                 &quot;  --ghmap       List raw.githubusercontent.com URLs for &quot;
                 &quot;current commit\n&quot;
                 &quot;  --copy        Copy all stdout output to system clipboard\n&quot;
                 &quot;  --hook-install Install or update .git/hooks/pre-commit for &quot;
                 &quot;scat wrap\n&quot;
                 &quot;  -h, --help    Show this help\n&quot;
                 &quot;\n&quot;
                 &quot;If no paths are given, scat reads patterns from scat.txt.\n&quot;;
}

Options parse_options(int argc, char **argv)
{
    Options opt;

    for (int i = 1; i &lt; argc; ++i)
    {
        std::string a = argv[i];
        if (a == &quot;-r&quot;)
        {
            opt.recursive = true;
        }
        else if (a == &quot;-l&quot;)
        {
            opt.list_only = true;
        }
        else if (a == &quot;--size&quot;)
        {
            opt.show_size = true;
        }
        else if (a == &quot;-n&quot;)
        {
            opt.line_numbers = true;
        }
        else if (a == &quot;--abs&quot;)
        {
            opt.abs_paths = true;
        }
        else if (a == &quot;--hook-install&quot;)
        {
            opt.hook_install = true;
        }
        else if (a == &quot;--sorted&quot;)
        {
            opt.sorted = true;
        }
        else if (a.rfind(&quot;--prefix=&quot;, 0) == 0)
        {
            opt.path_prefix = a.substr(std::string(&quot;--prefix=&quot;).size());
        }
        else if (a == &quot;--prefix&quot;)
        {
            if (i + 1 &lt; argc)
            {
                opt.path_prefix = argv[++i];
            }
            else
            {
                std::cerr &lt;&lt; &quot;--prefix requires value\n&quot;;
                std::exit(1);
            }
        }
        else if (a == &quot;--server&quot;)
        {
            if (i + 1 &lt; argc)
                opt.server_port = std::atoi(argv[++i]);
            else
            {
                std::cerr &lt;&lt; &quot;--server requires port\n&quot;;
                std::exit(1);
            }
        }
        else if (a == &quot;--chunk-help&quot;)
        {
            opt.chunk_help = true;
        }
        else if (a == &quot;-c&quot; || a == &quot;--chunk&quot;)
        {
            opt.chunk_trailer = true;
        }
        else if (a == &quot;--apply-stdin&quot;)
        {
            opt.apply_stdin = true;
        }
        else if (a == &quot;--apply&quot;)
        {
            if (i + 1 &lt; argc)
                opt.apply_file = argv[++i];
            else
            {
                std::cerr &lt;&lt; &quot;--apply requires file\n&quot;;
                std::exit(1);
            }
        }
        else if (a == &quot;--config&quot;)
        {
            if (i + 1 &lt; argc)
                opt.config_file = argv[++i];
            else
            {
                std::cerr &lt;&lt; &quot;--config requires file\n&quot;;
                std::exit(1);
            }
        }
        else if (a == &quot;--git-info&quot;)
        {
            opt.git_info = true;
        }
        else if (a == &quot;--ghmap&quot;)
        {
            opt.gh_map = true;
        }
        else if (a == &quot;--copy&quot;)
        {
            opt.copy_out = true;
        }
        else if (a == &quot;-h&quot; || a == &quot;--help&quot;)
        {
            print_help();
            std::exit(0);
        }
        else if (a == &quot;--wrap&quot;)
        {
            if (i + 1 &lt; argc)
            {
                opt.wrap_root = argv[++i];
            }
            else
            {
                std::cerr &lt;&lt; &quot;--wrap requires directory name\n&quot;;
                std::exit(1);
            }
        }
        else
        {
            opt.paths.push_back(a);
        }
    }

    // auto-config mode if no paths and no explicit config
    if (opt.paths.empty() &amp;&amp; opt.config_file.empty())
        opt.config_file = &quot;scat.txt&quot;;

    return opt;
}

</code></pre>
</body>
</html>

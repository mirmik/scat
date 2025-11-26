<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<pre><code>
#pragma once
    #include &lt;string&gt;
    #include &lt;vector&gt;

struct Options {
    bool chunk_trailer = false;
    bool recursive     = false;
    bool list_only     = false;
    bool abs_paths     = false;
    bool line_numbers  = false;
    bool sorted        = false;  // --sorted: sort list (-l) by size

    bool show_size     = false;  // --size: show file sizes in -l
    int  server_port   = 0;      // --server PORT

    std::string apply_file;
    bool        apply_stdin = false;

    std::string config_file;  // empty = no config mode
    bool        chunk_help = false;

    bool        git_info = false; // --git-info: print git meta info
    bool        gh_map   = false; // --ghmap: print GitHub raw URLs for current commit

    std::vector&lt;std::string&gt; paths; // positional paths

    std::string wrap_root;   // --wrap
    std::string path_prefix; // --prefix P
};


    Options parse_options(int argc, char** argv);

</code></pre>
</body>
</html>

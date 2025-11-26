<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma once<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
struct Options<br>
{<br>
    bool chunk_trailer = false;<br>
    bool recursive = false;<br>
    bool list_only = false;<br>
    bool abs_paths = false;<br>
    bool line_numbers = false;<br>
    bool sorted = false; // --sorted: sort list (-l) by size<br>
<br>
    bool show_size = false; // --size: show file sizes in -l<br>
    int server_port = 0;    // --server PORT<br>
    std::string apply_file;<br>
    bool apply_stdin = false;<br>
    std::string config_file; // empty = no config mode<br>
    bool chunk_help = false;<br>
<br>
    bool git_info = false; // --git-info: print git meta info<br>
    bool gh_map = false;   // --ghmap: print GitHub raw URLs for current commit<br>
    bool copy_out = false; // --copy: also send stdout to clipboard<br>
    bool hook_install =<br>
        false; // --hook-install: install/update git pre-commit hook<br>
<br>
    std::vector&lt;std::string&gt; paths; // positional paths<br>
<br>
    std::string wrap_root;   // --wrap<br>
    std::string path_prefix; // --prefix P<br>
};<br>
<br>
Options parse_options(int argc, char **argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

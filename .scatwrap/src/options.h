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
&#9;bool chunk_trailer = false;<br>
&#9;bool recursive = false;<br>
&#9;bool list_only = false;<br>
&#9;bool abs_paths = false;<br>
&#9;bool line_numbers = false;<br>
&#9;bool sorted = false; // --sorted: sort list (-l) by size<br>
<br>
&#9;bool show_size = false; // --size: show file sizes in -l<br>
&#9;int server_port = 0;    // --server PORT<br>
&#9;std::string apply_file;<br>
&#9;bool apply_stdin = false;<br>
&#9;std::string config_file; // empty = no config mode<br>
&#9;bool chunk_help = false;<br>
<br>
&#9;bool git_info = false; // --git-info: print git meta info<br>
&#9;bool gh_map = false;   // --ghmap: print GitHub raw URLs for current commit<br>
&#9;bool copy_out = false; // --copy: also send stdout to clipboard<br>
&#9;bool hook_install =<br>
&#9;&#9;false; // --hook-install: install/update git pre-commit hook<br>
<br>
&#9;std::vector&lt;std::string&gt; paths; // positional paths<br>
<br>
&#9;std::string wrap_root;   // --wrap<br>
&#9;std::string path_prefix; // --prefix P<br>
};<br>
<br>
Options parse_options(int argc, char **argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

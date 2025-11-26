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
&emsp;bool chunk_trailer = false;<br>
&emsp;bool recursive = false;<br>
&emsp;bool list_only = false;<br>
&emsp;bool abs_paths = false;<br>
&emsp;bool line_numbers = false;<br>
&emsp;bool sorted = false; // --sorted: sort list (-l) by size<br>
<br>
&emsp;bool show_size = false; // --size: show file sizes in -l<br>
&emsp;int server_port = 0;    // --server PORT<br>
&emsp;std::string apply_file;<br>
&emsp;bool apply_stdin = false;<br>
&emsp;std::string config_file; // empty = no config mode<br>
&emsp;bool chunk_help = false;<br>
<br>
&emsp;bool git_info = false; // --git-info: print git meta info<br>
&emsp;bool gh_map = false;   // --ghmap: print GitHub raw URLs for current commit<br>
&emsp;bool copy_out = false; // --copy: also send stdout to clipboard<br>
&emsp;bool hook_install =<br>
&emsp;&emsp;false; // --hook-install: install/update git pre-commit hook<br>
<br>
&emsp;std::vector&lt;std::string&gt; paths; // positional paths<br>
<br>
&emsp;std::string wrap_root;   // --wrap<br>
&emsp;std::string path_prefix; // --prefix P<br>
};<br>
<br>
Options parse_options(int argc, char **argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

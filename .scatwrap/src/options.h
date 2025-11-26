<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&#32;once<br>
#include&#32;&lt;string&gt;<br>
#include&#32;&lt;vector&gt;<br>
<br>
struct&#32;Options<br>
{<br>
&#32;&#32;&#32;&#32;bool&#32;chunk_trailer&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;recursive&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;list_only&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;abs_paths&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;line_numbers&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;bool&#32;sorted&#32;=&#32;false;&#32;//&#32;--sorted:&#32;sort&#32;list&#32;(-l)&#32;by&#32;size<br>
<br>
&#32;&#32;&#32;&#32;bool&#32;show_size&#32;=&#32;false;&#32;//&#32;--size:&#32;show&#32;file&#32;sizes&#32;in&#32;-l<br>
&#32;&#32;&#32;&#32;int&#32;server_port&#32;=&#32;0;&#32;&#32;&#32;&#32;//&#32;--server&#32;PORT<br>
&#32;&#32;&#32;&#32;std::string&#32;apply_file;<br>
&#32;&#32;&#32;&#32;bool&#32;apply_stdin&#32;=&#32;false;<br>
&#32;&#32;&#32;&#32;std::string&#32;config_file;&#32;//&#32;empty&#32;=&#32;no&#32;config&#32;mode<br>
&#32;&#32;&#32;&#32;bool&#32;chunk_help&#32;=&#32;false;<br>
<br>
&#32;&#32;&#32;&#32;bool&#32;git_info&#32;=&#32;false;&#32;//&#32;--git-info:&#32;print&#32;git&#32;meta&#32;info<br>
&#32;&#32;&#32;&#32;bool&#32;gh_map&#32;=&#32;false;&#32;&#32;&#32;//&#32;--ghmap:&#32;print&#32;GitHub&#32;raw&#32;URLs&#32;for&#32;current&#32;commit<br>
&#32;&#32;&#32;&#32;bool&#32;copy_out&#32;=&#32;false;&#32;//&#32;--copy:&#32;also&#32;send&#32;stdout&#32;to&#32;clipboard<br>
&#32;&#32;&#32;&#32;bool&#32;hook_install&#32;=<br>
&#32;&#32;&#32;&#32;&#32;&#32;&#32;&#32;false;&#32;//&#32;--hook-install:&#32;install/update&#32;git&#32;pre-commit&#32;hook<br>
<br>
&#32;&#32;&#32;&#32;std::vector&lt;std::string&gt;&#32;paths;&#32;//&#32;positional&#32;paths<br>
<br>
&#32;&#32;&#32;&#32;std::string&#32;wrap_root;&#32;&#32;&#32;//&#32;--wrap<br>
&#32;&#32;&#32;&#32;std::string&#32;path_prefix;&#32;//&#32;--prefix&#32;P<br>
};<br>
<br>
Options&#32;parse_options(int&#32;argc,&#32;char&#32;**argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

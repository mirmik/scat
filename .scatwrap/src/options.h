<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;Options<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;chunk_trailer&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;recursive&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;list_only&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;abs_paths&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;line_numbers&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;sorted&nbsp;=&nbsp;false;&nbsp;//&nbsp;--sorted:&nbsp;sort&nbsp;list&nbsp;(-l)&nbsp;by&nbsp;size<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;show_size&nbsp;=&nbsp;false;&nbsp;//&nbsp;--size:&nbsp;show&nbsp;file&nbsp;sizes&nbsp;in&nbsp;-l<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;server_port&nbsp;=&nbsp;0;&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;--server&nbsp;PORT<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;apply_file;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;apply_stdin&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;config_file;&nbsp;//&nbsp;empty&nbsp;=&nbsp;no&nbsp;config&nbsp;mode<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;chunk_help&nbsp;=&nbsp;false;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;git_info&nbsp;=&nbsp;false;&nbsp;//&nbsp;--git-info:&nbsp;print&nbsp;git&nbsp;meta&nbsp;info<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;gh_map&nbsp;=&nbsp;false;&nbsp;&nbsp;&nbsp;//&nbsp;--ghmap:&nbsp;print&nbsp;GitHub&nbsp;raw&nbsp;URLs&nbsp;for&nbsp;current&nbsp;commit<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;copy_out&nbsp;=&nbsp;false;&nbsp;//&nbsp;--copy:&nbsp;also&nbsp;send&nbsp;stdout&nbsp;to&nbsp;clipboard<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;hook_install&nbsp;=<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;false;&nbsp;//&nbsp;--hook-install:&nbsp;install/update&nbsp;git&nbsp;pre-commit&nbsp;hook<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;paths;&nbsp;//&nbsp;positional&nbsp;paths<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;wrap_root;&nbsp;&nbsp;&nbsp;//&nbsp;--wrap<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;path_prefix;&nbsp;//&nbsp;--prefix&nbsp;P<br>
};<br>
<br>
Options&nbsp;parse_options(int&nbsp;argc,&nbsp;char&nbsp;**argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

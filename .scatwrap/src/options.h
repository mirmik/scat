<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&quot;rules.h&quot;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;Options<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;recursive&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;list_only&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;abs_paths&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;line_numbers&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;sorted&nbsp;=&nbsp;false;&nbsp;//&nbsp;--sorted:&nbsp;sort&nbsp;list&nbsp;(-l)&nbsp;by&nbsp;size<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;show_size&nbsp;=&nbsp;false;&nbsp;//&nbsp;--size:&nbsp;show&nbsp;file&nbsp;sizes&nbsp;in&nbsp;-l<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;compact&nbsp;=&nbsp;false;&nbsp;&nbsp;&nbsp;//&nbsp;--compact:&nbsp;remove&nbsp;empty&nbsp;lines&nbsp;from&nbsp;file&nbsp;output<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;server_port&nbsp;=&nbsp;0;&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;--server&nbsp;PORT<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;config_file;&nbsp;//&nbsp;empty&nbsp;=&nbsp;no&nbsp;config&nbsp;mode<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;git_info&nbsp;=&nbsp;false;&nbsp;//&nbsp;--git-info:&nbsp;print&nbsp;git&nbsp;meta&nbsp;info<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;gh_map&nbsp;=&nbsp;false;&nbsp;&nbsp;&nbsp;//&nbsp;--ghmap:&nbsp;print&nbsp;GitHub&nbsp;raw&nbsp;URLs&nbsp;for&nbsp;current&nbsp;commit<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;copy_out&nbsp;=&nbsp;false;&nbsp;//&nbsp;--copy:&nbsp;also&nbsp;send&nbsp;stdout&nbsp;to&nbsp;clipboard<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;hook_install&nbsp;=<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;false;&nbsp;//&nbsp;--hook-install:&nbsp;install/update&nbsp;git&nbsp;pre-commit&nbsp;hook<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;show_version&nbsp;=&nbsp;false;&nbsp;//&nbsp;-V,&nbsp;--version:&nbsp;show&nbsp;version&nbsp;and&nbsp;exit<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;paths;&nbsp;//&nbsp;positional&nbsp;paths<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;exclude_paths;&nbsp;//&nbsp;--exclude&nbsp;P&nbsp;/&nbsp;@PATTERN&nbsp;(argument&nbsp;mode)<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;edit_config_name;&nbsp;//&nbsp;-e&nbsp;NAME:&nbsp;edit&nbsp;./.scatconfig/NAME<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;Rule&gt;&nbsp;arg_rules;&nbsp;//&nbsp;ordered&nbsp;CLI&nbsp;rules&nbsp;with&nbsp;excludes/includes<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;wrap_root;&nbsp;&nbsp;&nbsp;//&nbsp;--wrap<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;path_prefix;&nbsp;//&nbsp;--prefix&nbsp;P<br>
};<br>
<br>
Options&nbsp;parse_options(int&nbsp;argc,&nbsp;char&nbsp;**argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

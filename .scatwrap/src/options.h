<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma &#20;once<br>
#include &#20;&lt;string&gt;<br>
#include &#20;&lt;vector&gt;<br>
<br>
struct &#20;Options<br>
{<br>
 &#20; &#20; &#20; &#20;bool &#20;chunk_trailer &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;recursive &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;list_only &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;abs_paths &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;line_numbers &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;bool &#20;sorted &#20;= &#20;false; &#20;// &#20;--sorted: &#20;sort &#20;list &#20;(-l) &#20;by &#20;size<br>
<br>
 &#20; &#20; &#20; &#20;bool &#20;show_size &#20;= &#20;false; &#20;// &#20;--size: &#20;show &#20;file &#20;sizes &#20;in &#20;-l<br>
 &#20; &#20; &#20; &#20;int &#20;server_port &#20;= &#20;0; &#20; &#20; &#20; &#20;// &#20;--server &#20;PORT<br>
 &#20; &#20; &#20; &#20;std::string &#20;apply_file;<br>
 &#20; &#20; &#20; &#20;bool &#20;apply_stdin &#20;= &#20;false;<br>
 &#20; &#20; &#20; &#20;std::string &#20;config_file; &#20;// &#20;empty &#20;= &#20;no &#20;config &#20;mode<br>
 &#20; &#20; &#20; &#20;bool &#20;chunk_help &#20;= &#20;false;<br>
<br>
 &#20; &#20; &#20; &#20;bool &#20;git_info &#20;= &#20;false; &#20;// &#20;--git-info: &#20;print &#20;git &#20;meta &#20;info<br>
 &#20; &#20; &#20; &#20;bool &#20;gh_map &#20;= &#20;false; &#20; &#20; &#20;// &#20;--ghmap: &#20;print &#20;GitHub &#20;raw &#20;URLs &#20;for &#20;current &#20;commit<br>
 &#20; &#20; &#20; &#20;bool &#20;copy_out &#20;= &#20;false; &#20;// &#20;--copy: &#20;also &#20;send &#20;stdout &#20;to &#20;clipboard<br>
 &#20; &#20; &#20; &#20;bool &#20;hook_install &#20;=<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;false; &#20;// &#20;--hook-install: &#20;install/update &#20;git &#20;pre-commit &#20;hook<br>
<br>
 &#20; &#20; &#20; &#20;std::vector&lt;std::string&gt; &#20;paths; &#20;// &#20;positional &#20;paths<br>
<br>
 &#20; &#20; &#20; &#20;std::string &#20;wrap_root; &#20; &#20; &#20;// &#20;--wrap<br>
 &#20; &#20; &#20; &#20;std::string &#20;path_prefix; &#20;// &#20;--prefix &#20;P<br>
};<br>
<br>
Options &#20;parse_options(int &#20;argc, &#20;char &#20;**argv);<br>
<!-- END SCAT CODE -->
</body>
</html>

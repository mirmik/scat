<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/scat.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
struct&nbsp;Options;<br>
<br>
int&nbsp;scat_main(int&nbsp;argc,&nbsp;char&nbsp;**argv);<br>
void&nbsp;print_tree(const&nbsp;std::vector&lt;std::filesystem::path&gt;&nbsp;&amp;files);<br>
int&nbsp;wrap_files_to_html(const&nbsp;std::vector&lt;std::filesystem::path&gt;&nbsp;&amp;files,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;Options&nbsp;&amp;opt);<br>
<!-- END SCAT CODE -->
</body>
</html>

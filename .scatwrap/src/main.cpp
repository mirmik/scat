<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;scat.h&quot;<br>
#ifdef&nbsp;_WIN32<br>
#&nbsp;&nbsp;&nbsp;&nbsp;include&nbsp;&lt;windows.h&gt;<br>
#endif<br>
<br>
int&nbsp;main(int&nbsp;argc,&nbsp;char&nbsp;**argv)<br>
{<br>
#ifdef&nbsp;_WIN32<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;Переключаем&nbsp;кодовые&nbsp;страницы&nbsp;консоли&nbsp;на&nbsp;UTF-8,&nbsp;чтобы&nbsp;UTF-8&nbsp;текст<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;(включая&nbsp;кириллицу)&nbsp;нормально&nbsp;печатался&nbsp;в&nbsp;cmd/PowerShell.<br>
&nbsp;&nbsp;&nbsp;&nbsp;SetConsoleOutputCP(CP_UTF8);<br>
&nbsp;&nbsp;&nbsp;&nbsp;SetConsoleCP(CP_UTF8);<br>
#endif<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;scat_main(argc,&nbsp;argv);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>

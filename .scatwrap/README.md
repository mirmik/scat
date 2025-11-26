<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>README.md</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
Окей, &#20;делаем &#20;компактную &#20;версию, &#20;но &#20;с &#20;описанием &#20;нового &#20;symbol &#20;API. &#20;Вот &#20;целиком &#20;новый &#20;`README.md` &#20;(без &#20;chunk_v2-патча, &#20;просто &#20;файл):<br>
<br>
````markdown<br>
# &#20;scat<br>
<br>
`scat` &#20;is &#20;a &#20;small &#20;cross-platform &#20;utility &#20;for:<br>
<br>
* &#20;printing &#20;multiple &#20;files &#20;with &#20;clear &#20;headers &#20;(for &#20;copy-paste &#20;into &#20;chats),<br>
* &#20;listing &#20;files &#20;with &#20;sizes,<br>
* &#20;applying &#20;structured &#20;patches &#20;(chunk_v2),<br>
* &#20;replacing &#20;C++ &#20;/ &#20;Python &#20;classes &#20;and &#20;methods &#20;by &#20;name &#20;(symbol &#20;API).<br>
<br>
---<br>
<br>
## &#20;Basic &#20;usage<br>
<br>
Print &#20;files:<br>
<br>
```bash<br>
scat &#20;file1.cpp &#20;dir/file2.h<br>
scat &#20;src &#20;-r &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;# &#20;recursive<br>
scat &#20;src &#20;-r &#20;--abs &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;# &#20;absolute &#20;paths<br>
scat &#20;src &#20;-r &#20;-l &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;# &#20;list &#20;with &#20;sizes<br>
scat &#20;src &#20;-r &#20;-l &#20;--sorted &#20; &#20;# &#20;list, &#20;sorted &#20;by &#20;size &#20;(descending)<br>
````<br>
<br>
Output &#20;looks &#20;like:<br>
<br>
```text<br>
===== &#20;path/to/file.cpp &#20;=====<br>
&lt;file &#20;content&gt;<br>
```<br>
<br>
You &#20;can &#20;pipe &#20;into &#20;clipboard &#20;tools:<br>
<br>
```bash<br>
scat &#20;src &#20;-r &#20;| &#20;clip.exe &#20; &#20; &#20;# &#20;Windows &#20;/ &#20;WSL<br>
scat &#20;src &#20;-r &#20;| &#20;wl-copy &#20; &#20; &#20; &#20;# &#20;Linux &#20;/ &#20;WSL<br>
```<br>
<br>
---<br>
<br>
## &#20;Config &#20;file &#20;mode &#20;(`scat.txt` &#20;/ &#20;`--config`)<br>
<br>
If &#20;you &#20;run &#20;`scat` &#20;**without &#20;positional &#20;arguments**, &#20;it &#20;reads &#20;rules &#20;from &#20;`scat.txt` &#20;in &#20;the &#20;current &#20;directory:<br>
<br>
```bash<br>
scat &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;# &#20;uses &#20;scat.txt<br>
scat &#20;--config &#20;myrules.txt<br>
```<br>
<br>
Each &#20;non-empty &#20;line &#20;is &#20;a &#20;rule:<br>
<br>
* &#20;include: &#20;`src/*`, &#20;`tests/**`, &#20;`CMakeLists.txt`<br>
* &#20;exclude: &#20;starts &#20;with &#20;`!`, &#20;e.g. &#20;`!tests/doctest/**`<br>
<br>
Patterns:<br>
<br>
* &#20;`*` &#20;— &#20;match &#20;inside &#20;one &#20;directory<br>
* &#20;`**` &#20;— &#20;recursive &#20;directory &#20;traversal<br>
<br>
Example:<br>
<br>
```text<br>
src/*<br>
tests/*<br>
!tests/doctest/**<br>
CMakeLists.txt<br>
scat.txt<br>
```<br>
<br>
### &#20;Project &#20;tree &#20;(`[TREE]` &#20;section)<br>
<br>
Optional &#20;second &#20;rule &#20;set &#20;used &#20;only &#20;for &#20;a &#20;tree &#20;view &#20;at &#20;the &#20;end:<br>
<br>
```text<br>
src/*<br>
tests/*<br>
[TREE]<br>
src/**<br>
tests/**<br>
```<br>
<br>
Running &#20;`scat` &#20;will &#20;print &#20;files &#20;first, &#20;then:<br>
<br>
```text<br>
===== &#20;PROJECT &#20;TREE &#20;=====<br>
├── &#20;src/...<br>
└── &#20;tests/...<br>
========================<br>
```<br>
<br>
---<br>
<br>
## &#20;Patch &#20;mode &#20;(chunk_v2)<br>
<br>
`scat` &#20;can &#20;apply &#20;patches &#20;in &#20;the &#20;chunk_v2 &#20;format.<br>
<br>
```bash<br>
scat &#20;--apply &#20;patch.txt &#20; &#20; &#20; &#20; &#20; &#20;# &#20;apply &#20;from &#20;file<br>
wl-paste &#20;| &#20;scat &#20;--apply-stdin &#20; &#20;# &#20;apply &#20;from &#20;stdin<br>
```<br>
<br>
Patch &#20;structure:<br>
<br>
```text<br>
=== &#20;file: &#20;path/to/file.cpp &#20;===<br>
--- &#20;&lt;command&gt; &#20;[args...]<br>
&lt;payload &#20;lines...&gt;<br>
=END=<br>
```<br>
<br>
Commands:<br>
<br>
* &#20;text-based:<br>
<br>
 &#20; &#20;* &#20;`insert-after-text`<br>
 &#20; &#20;* &#20;`insert-before-text`<br>
 &#20; &#20;* &#20;`replace-text`<br>
 &#20; &#20;* &#20;`delete-text`<br>
* &#20;file-level:<br>
<br>
 &#20; &#20;* &#20;`create-file`<br>
 &#20; &#20;* &#20;`delete-file`<br>
* &#20;symbol-based:<br>
<br>
 &#20; &#20;* &#20;`replace-cpp-class`<br>
 &#20; &#20;* &#20;`replace-cpp-method`<br>
 &#20; &#20;* &#20;`replace-py-class`<br>
 &#20; &#20;* &#20;`replace-py-method`<br>
<br>
Text &#20;commands &#20;support:<br>
<br>
* &#20;legacy &#20;format: &#20;`marker` &#20;then &#20;`---` &#20;then &#20;payload<br>
* &#20;YAML-style &#20;context &#20;with &#20;`BEFORE:`, &#20;`MARKER:`, &#20;`AFTER:` &#20;blocks &#20;(strict &#20;match &#20;by &#20;trimmed &#20;lines)<br>
<br>
Patch &#20;application &#20;is &#20;transactional: &#20;if &#20;any &#20;section &#20;fails &#20;(marker &#20;not &#20;found, &#20;ambiguous, &#20;symbol &#20;not &#20;found, &#20;I/O &#20;error), &#20;all &#20;touched &#20;files &#20;are &#20;rolled &#20;back.<br>
<br>
---<br>
<br>
## &#20;Symbol &#20;API &#20;(C++ &#20;/ &#20;Python)<br>
<br>
Symbol &#20;commands &#20;work &#20;**without** &#20;raw &#20;text &#20;markers: &#20;they &#20;parse &#20;the &#20;file &#20;and &#20;locate &#20;classes &#20;/ &#20;methods.<br>
<br>
Available &#20;commands:<br>
<br>
```text<br>
replace-cpp-class &#20; &#20;&lt;ClassName&gt;<br>
replace-cpp-method &#20;&lt;ClassName&gt; &#20;&lt;methodName&gt;<br>
replace-cpp-method &#20;&lt;ClassName::methodName&gt;<br>
<br>
replace-py-class &#20; &#20; &#20;&lt;ClassName&gt;<br>
replace-py-method &#20; &#20;&lt;ClassName&gt; &#20;&lt;methodName&gt;<br>
replace-py-method &#20; &#20;&lt;ClassName.methodName&gt;<br>
```<br>
<br>
### &#20;C++ &#20;examples<br>
<br>
Replace &#20;a &#20;whole &#20;class:<br>
<br>
```text<br>
=== &#20;file: &#20;src/TEST.h &#20;===<br>
--- &#20;replace-cpp-class &#20;TargetClass<br>
class &#20;TargetClass<br>
{<br>
 &#20; &#20;public:<br>
 &#20; &#20; &#20; &#20;TargetClass() &#20;= &#20;default;<br>
 &#20; &#20; &#20; &#20;~TargetClass() &#20;= &#20;default;<br>
<br>
 &#20; &#20; &#20; &#20;int &#20;value() &#20;const &#20;{ &#20;return &#20;member_variable_; &#20;}<br>
 &#20; &#20; &#20; &#20;void &#20;set_value(int &#20;v) &#20;{ &#20;member_variable_ &#20;= &#20;v; &#20;}<br>
<br>
 &#20; &#20;private:<br>
 &#20; &#20; &#20; &#20;int &#20;member_variable_ &#20;= &#20;0;<br>
};<br>
=END=<br>
```<br>
<br>
Replace &#20;a &#20;method &#20;inside &#20;a &#20;class:<br>
<br>
```text<br>
=== &#20;file: &#20;src/TEST.h &#20;===<br>
--- &#20;replace-cpp-method &#20;ClassWithTargetMethod::TargetMethod<br>
 &#20; &#20; &#20; &#20;void &#20;TargetMethod()<br>
 &#20; &#20; &#20; &#20;{<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;// &#20;Updated &#20;implementation<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;member_variable_ &#20;+= &#20;1;<br>
 &#20; &#20; &#20; &#20;}<br>
=END=<br>
```<br>
<br>
The &#20;C++ &#20;finder:<br>
<br>
* &#20;skips &#20;comments, &#20;string &#20;/ &#20;char &#20;literals &#20;and &#20;preprocessor &#20;lines,<br>
* &#20;ignores &#20;forward &#20;declarations &#20;(`class &#20;Foo;`),<br>
* &#20;replaces &#20;exactly &#20;the &#20;region &#20;of &#20;the &#20;target &#20;class &#20;/ &#20;method.<br>
<br>
### &#20;Python &#20;examples<br>
<br>
Replace &#20;a &#20;whole &#20;class:<br>
<br>
```text<br>
=== &#20;file: &#20;src/foo.py &#20;===<br>
--- &#20;replace-py-class &#20;Foo<br>
class &#20;Foo:<br>
 &#20; &#20; &#20; &#20;def &#20;__init__(self):<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;self.x &#20;= &#20;2<br>
<br>
 &#20; &#20; &#20; &#20;def &#20;answer(self):<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;42<br>
=END=<br>
```<br>
<br>
Replace &#20;a &#20;method &#20;(sync &#20;or &#20;async) &#20;inside &#20;a &#20;class:<br>
<br>
```text<br>
=== &#20;file: &#20;src/weird.py &#20;===<br>
--- &#20;replace-py-method &#20;Weird &#20;run<br>
 &#20; &#20; &#20; &#20;def &#20;run(self):<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;100<br>
=END=<br>
```<br>
<br>
```text<br>
=== &#20;file: &#20;src/async_foo.py &#20;===<br>
--- &#20;replace-py-method &#20;Foo.bar<br>
 &#20; &#20; &#20; &#20;async &#20;def &#20;bar(self):<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;return &#20;2<br>
=END=<br>
```<br>
<br>
The &#20;Python &#20;finder:<br>
<br>
* &#20;uses &#20;indentation &#20;to &#20;determine &#20;class &#20;/ &#20;method &#20;bodies,<br>
* &#20;includes &#20;decorators &#20;(`@something`) &#20;above &#20;the &#20;method,<br>
* &#20;replaces &#20;the &#20;whole &#20;method &#20;body &#20;region.<br>
<br>
---<br>
<br>
## &#20;Build<br>
<br>
Linux &#20;/ &#20;macOS:<br>
<br>
```bash<br>
mkdir &#20;build &#20;&amp;&amp; &#20;cd &#20;build<br>
cmake &#20;-DCMAKE_BUILD_TYPE=Release &#20;..<br>
cmake &#20;--build &#20;.<br>
cmake &#20;--install &#20;.<br>
```<br>
<br>
Windows &#20;(Visual &#20;Studio &#20;generator):<br>
<br>
```powershell<br>
mkdir &#20;build<br>
cd &#20;build<br>
cmake &#20;..<br>
cmake &#20;--build &#20;. &#20;--config &#20;Release<br>
cmake &#20;--install &#20;. &#20;--config &#20;Release<br>
```<br>
<br>
---<br>
<br>
## &#20;License<br>
<br>
Do &#20;whatever &#20;you &#20;want &#20;with &#20;it.<br>
<!-- END SCAT CODE -->
</body>
</html>

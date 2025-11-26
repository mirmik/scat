<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>README.md</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
Окей, делаем компактную версию, но с описанием нового symbol API. Вот целиком новый `README.md` (без chunk_v2-патча, просто файл):<br>
<br>
````markdown<br>
# scat<br>
<br>
`scat` is a small cross-platform utility for:<br>
<br>
* printing multiple files with clear headers (for copy-paste into chats),<br>
* listing files with sizes,<br>
* applying structured patches (chunk_v2),<br>
* replacing C++ / Python classes and methods by name (symbol API).<br>
<br>
---<br>
<br>
## Basic usage<br>
<br>
Print files:<br>
<br>
```bash<br>
scat file1.cpp dir/file2.h<br>
scat src -r              # recursive<br>
scat src -r --abs        # absolute paths<br>
scat src -r -l           # list with sizes<br>
scat src -r -l --sorted  # list, sorted by size (descending)<br>
````<br>
<br>
Output looks like:<br>
<br>
```text<br>
===== path/to/file.cpp =====<br>
&lt;file content&gt;<br>
```<br>
<br>
You can pipe into clipboard tools:<br>
<br>
```bash<br>
scat src -r | clip.exe   # Windows / WSL<br>
scat src -r | wl-copy    # Linux / WSL<br>
```<br>
<br>
---<br>
<br>
## Config file mode (`scat.txt` / `--config`)<br>
<br>
If you run `scat` **without positional arguments**, it reads rules from `scat.txt` in the current directory:<br>
<br>
```bash<br>
scat          # uses scat.txt<br>
scat --config myrules.txt<br>
```<br>
<br>
Each non-empty line is a rule:<br>
<br>
* include: `src/*`, `tests/**`, `CMakeLists.txt`<br>
* exclude: starts with `!`, e.g. `!tests/doctest/**`<br>
<br>
Patterns:<br>
<br>
* `*` — match inside one directory<br>
* `**` — recursive directory traversal<br>
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
### Project tree (`[TREE]` section)<br>
<br>
Optional second rule set used only for a tree view at the end:<br>
<br>
```text<br>
src/*<br>
tests/*<br>
[TREE]<br>
src/**<br>
tests/**<br>
```<br>
<br>
Running `scat` will print files first, then:<br>
<br>
```text<br>
===== PROJECT TREE =====<br>
├── src/...<br>
└── tests/...<br>
========================<br>
```<br>
<br>
---<br>
<br>
## Patch mode (chunk_v2)<br>
<br>
`scat` can apply patches in the chunk_v2 format.<br>
<br>
```bash<br>
scat --apply patch.txt      # apply from file<br>
wl-paste | scat --apply-stdin  # apply from stdin<br>
```<br>
<br>
Patch structure:<br>
<br>
```text<br>
=== file: path/to/file.cpp ===<br>
--- &lt;command&gt; [args...]<br>
&lt;payload lines...&gt;<br>
=END=<br>
```<br>
<br>
Commands:<br>
<br>
* text-based:<br>
<br>
* `insert-after-text`<br>
* `insert-before-text`<br>
* `replace-text`<br>
* `delete-text`<br>
* file-level:<br>
<br>
* `create-file`<br>
* `delete-file`<br>
* symbol-based:<br>
<br>
* `replace-cpp-class`<br>
* `replace-cpp-method`<br>
* `replace-py-class`<br>
* `replace-py-method`<br>
<br>
Text commands support:<br>
<br>
* legacy format: `marker` then `---` then payload<br>
* YAML-style context with `BEFORE:`, `MARKER:`, `AFTER:` blocks (strict match by trimmed lines)<br>
<br>
Patch application is transactional: if any section fails (marker not found, ambiguous, symbol not found, I/O error), all touched files are rolled back.<br>
<br>
---<br>
<br>
## Symbol API (C++ / Python)<br>
<br>
Symbol commands work **without** raw text markers: they parse the file and locate classes / methods.<br>
<br>
Available commands:<br>
<br>
```text<br>
replace-cpp-class  &lt;ClassName&gt;<br>
replace-cpp-method &lt;ClassName&gt; &lt;methodName&gt;<br>
replace-cpp-method &lt;ClassName::methodName&gt;<br>
<br>
replace-py-class   &lt;ClassName&gt;<br>
replace-py-method  &lt;ClassName&gt; &lt;methodName&gt;<br>
replace-py-method  &lt;ClassName.methodName&gt;<br>
```<br>
<br>
### C++ examples<br>
<br>
Replace a whole class:<br>
<br>
```text<br>
=== file: src/TEST.h ===<br>
--- replace-cpp-class TargetClass<br>
class TargetClass<br>
{<br>
public:<br>
&emsp;TargetClass() = default;<br>
&emsp;~TargetClass() = default;<br>
<br>
&emsp;int value() const { return member_variable_; }<br>
&emsp;void set_value(int v) { member_variable_ = v; }<br>
<br>
private:<br>
&emsp;int member_variable_ = 0;<br>
};<br>
=END=<br>
```<br>
<br>
Replace a method inside a class:<br>
<br>
```text<br>
=== file: src/TEST.h ===<br>
--- replace-cpp-method ClassWithTargetMethod::TargetMethod<br>
&emsp;void TargetMethod()<br>
&emsp;{<br>
&emsp;&emsp;// Updated implementation<br>
&emsp;&emsp;member_variable_ += 1;<br>
&emsp;}<br>
=END=<br>
```<br>
<br>
The C++ finder:<br>
<br>
* skips comments, string / char literals and preprocessor lines,<br>
* ignores forward declarations (`class Foo;`),<br>
* replaces exactly the region of the target class / method.<br>
<br>
### Python examples<br>
<br>
Replace a whole class:<br>
<br>
```text<br>
=== file: src/foo.py ===<br>
--- replace-py-class Foo<br>
class Foo:<br>
&emsp;def __init__(self):<br>
&emsp;&emsp;self.x = 2<br>
<br>
&emsp;def answer(self):<br>
&emsp;&emsp;return 42<br>
=END=<br>
```<br>
<br>
Replace a method (sync or async) inside a class:<br>
<br>
```text<br>
=== file: src/weird.py ===<br>
--- replace-py-method Weird run<br>
&emsp;def run(self):<br>
&emsp;&emsp;return 100<br>
=END=<br>
```<br>
<br>
```text<br>
=== file: src/async_foo.py ===<br>
--- replace-py-method Foo.bar<br>
&emsp;async def bar(self):<br>
&emsp;&emsp;return 2<br>
=END=<br>
```<br>
<br>
The Python finder:<br>
<br>
* uses indentation to determine class / method bodies,<br>
* includes decorators (`@something`) above the method,<br>
* replaces the whole method body region.<br>
<br>
---<br>
<br>
## Build<br>
<br>
Linux / macOS:<br>
<br>
```bash<br>
mkdir build &amp;&amp; cd build<br>
cmake -DCMAKE_BUILD_TYPE=Release ..<br>
cmake --build .<br>
cmake --install .<br>
```<br>
<br>
Windows (Visual Studio generator):<br>
<br>
```powershell<br>
mkdir build<br>
cd build<br>
cmake ..<br>
cmake --build . --config Release<br>
cmake --install . --config Release<br>
```<br>
<br>
---<br>
<br>
## License<br>
<br>
Do whatever you want with it.<br>
<!-- END SCAT CODE -->
</body>
</html>

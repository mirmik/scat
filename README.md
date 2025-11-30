# scat

`scat` is a small cross-platform utility for:

* printing multiple files with clear headers (handy for copy/paste into chats),
* listing files with sizes,
* wrapping files as HTML snippets,
* generating GitHub raw links for the current commit (`--ghmap`).

---

## Basic usage

Print files:

```bash
scat file1.cpp dir/file2.h
scat src -r              # recursive
scat src -r --abs        # absolute paths
scat src -r -l           # list with sizes
scat src -r -l --sorted  # list, sorted by size (descending)
scat src -r @src/tests/**           # exclude tests (argument mode)
scat src -r --exclude src/tests/**  # same as @src/tests/**
```

> ⚠️ Обратите внимание: shell разворачивает `*` перед передачей в программу.  
> Для `--exclude` используйте кавычки/экранирование (`--exclude "src/*.cpp"` или `--exclude src/\\*.cpp`), чтобы правило обрабатывалось как паттерн, а не список файлов.

Output looks like:

```text
===== path/to/file.cpp =====
<file content>
```

You can pipe into clipboard tools:

```bash
scat src -r | clip.exe   # Windows / WSL
scat src -r | wl-copy    # Linux / WSL
```

---

## Config file mode (`scat.txt` / `--config`)

If you run `scat` **without positional arguments**, it reads rules from `scat.txt` in the current directory:

```bash
scat          # uses scat.txt
scat --config myrules.txt
```

Each non-empty line is a rule:

* include: `src/*`, `tests/**`, `CMakeLists.txt`
* exclude: starts with `!`, e.g. `!tests/doctest/**`

Patterns:

* `*` — match inside one directory
* `**` — recursive directory traversal

Example:

```text
src/*
tests/*
!tests/doctest/**
CMakeLists.txt
scat.txt
```

### Variants (`[VAR(name)]` sections)

You can define alternative sets of text rules in the same config file:

```text
[VAR(min)]
src/*
!src/bigfile.txt
tests/**
!tests/doctest/**
CMakeLists.txt
scat.txt
README.md
```

Run `scat` with `--variant` to use a specific variant instead of the top-level rules:

```bash
scat --variant min              # uses [VAR(min)] from scat.txt
scat --config myrules.txt --variant min
```

If `--variant` is not specified, scat uses the top-level rules defined before `[TREE]`, `[MAPFORMAT]` or any `[VAR(...)]` sections.

### Project tree (`[TREE]` section)
### Project tree (`[TREE]` section)

Optional second rule set used only for a tree view at the end:

```text
src/*
tests/*
[TREE]
src/**
tests/**
```

Running `scat` will print files first, then:

```text
===== PROJECT TREE =====
├── src/...
└── tests/...
========================
```

---

## HTML wrap mode

Wrap collected files as standalone HTML snippets:

```bash
scat src --wrap out_html
```

Each file is converted into HTML with a header and saved under `out_html` with the same relative path.

---

## Git helpers

* `--git-info` prints commit hash and remote origin.
* `--ghmap` switches to list mode and prepends GitHub raw URLs for the current commit (useful for sharing).
* `--hook-install` adds a pre-commit hook that regenerates `.scatwrap` via `scat --wrap`.

---

## Build

Linux / macOS:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --install .
```

Windows (Visual Studio generator):

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
cmake --install . --config Release
```

---

## License

Copyright © 2025 mirmik

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

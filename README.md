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
```

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

Do whatever you want with it.

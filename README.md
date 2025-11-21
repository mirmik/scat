# scat

A small cross-platform utility that prints multiple files with clear headers.
Useful for sharing entire code snippets or project trees with AI tools.

```
===== path/to/file.cpp =====
<file content>
```

## Features

* Collects files from command-line arguments
* Optional recursive directory scan (`-r`)
* Relative paths by default, absolute with `--abs`
* List files only (`-l`)
* No extra blank lines, clean output

## Usage

```bash
scat file1.cpp dir/file2.h
scat src -r
scat src -r -l
scat src -r --abs
scat src -r | clip.exe
```

## Build & Install

```bash
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```

## License

Do whatever you want with it.

# scat

A small cross-platform utility that prints multiple files with clear headers.
Useful for sharing code or project trees with AI tools.

```
===== path/to/file.cpp =====
<file content>
```

## Features

* Collects files from arguments
* Optional recursive scan (`-r`)
* Relative paths by default, absolute with `--abs`
* List files only (`-l`)
* Supports configuration files (`scat.txt` or `--config F`)
* Main source file: `src/main.cpp`

## Usage

```bash
scat file1.cpp dir/file2.h
scat src -r
scat src -r -l
scat src -r --abs
scat *                 # all files in the current directory
scat src -r | clip.exe # Windows including wsl2
scat src -r | wl-copy # Ubuntu GNOME 22.04+
```

## Configuration file mode

`scat` can also collect files based on rules defined in a config file.

### How it works

* If you run `scat` **without positional arguments**, it automatically loads rules from `scat.txt`.
* You can explicitly specify a config file:

  ```bash
  scat --config myrules.txt
  ```

### Rule format

Each line is a rule. Two types exist:

**Include rule**

```
src/*
tests/**
```

**Exclude rule** (starts with `!`)

```
!tests/doctest/**
```

### Supported patterns

* `*` — match inside one directory

  ```
  src/*.cpp
  ```
* `**` — recursive directory traversal

  ```
  assets/**
  ```

> Note: pattern matching is intentionally simple.
> Complex glob expressions like `src/**/*.cpp` are not supported. (COMMING SOON)

### Example `scat.txt`

```
src/*
tests/*
!tests/doctest/**
CMakeLists.txt
scat.txt
```

## Build & Install (Linux / macOS)

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --install .
```

## Build & Install (Windows, Visual Studio generator)

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
cmake --install . --config Release
```

## License

Do whatever you want with it.
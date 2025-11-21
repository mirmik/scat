# scat

A small cross-platform utility that prints multiple files with clear headers.  
Useful for sharing code or project trees with AI tools.

```
===== path/to/file.cpp ===== 
<file content>
```

## Features
- Collects files from arguments
- Optional recursive scan (`-r`)
- Relative paths by default, absolute with `--abs`
- List files only (`-l`)
- Main source file: `src/main.cpp`

## Usage
```bash
scat file1.cpp dir/file2.h
scat src -r
scat src -r -l
scat src -r --abs
scat *                 # all files in the current directory
scat src -r | clip.exe
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

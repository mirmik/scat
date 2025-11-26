Окей, делаем компактную версию, но с описанием нового symbol API. Вот целиком новый `README.md` (без chunk_v2-патча, просто файл):

````markdown
# scat

`scat` is a small cross-platform utility for:

* printing multiple files with clear headers (for copy-paste into chats),
* listing files with sizes,
* applying structured patches (chunk_v2),
* replacing C++ / Python classes and methods by name (symbol API).

---

## Basic usage

Print files:

```bash
scat file1.cpp dir/file2.h
scat src -r              # recursive
scat src -r --abs        # absolute paths
scat src -r -l           # list with sizes
scat src -r -l --sorted  # list, sorted by size (descending)
````

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

## Patch mode (chunk_v2)

`scat` can apply patches in the chunk_v2 format.

```bash
scat --apply patch.txt      # apply from file
wl-paste | scat --apply-stdin  # apply from stdin
```

Patch structure:

```text
=== file: path/to/file.cpp ===
--- <command> [args...]
<payload lines...>
=END=
```

Commands:

* text-based:

  * `insert-after-text`
  * `insert-before-text`
  * `replace-text`
  * `delete-text`
* file-level:

  * `create-file`
  * `delete-file`
* symbol-based:

  * `replace-cpp-class`
  * `replace-cpp-method`
  * `replace-py-class`
  * `replace-py-method`

Text commands support:

* legacy format: `marker` then `---` then payload
* YAML-style context with `BEFORE:`, `MARKER:`, `AFTER:` blocks (strict match by trimmed lines)

Patch application is transactional: if any section fails (marker not found, ambiguous, symbol not found, I/O error), all touched files are rolled back.

---

## Symbol API (C++ / Python)

Symbol commands work **without** raw text markers: they parse the file and locate classes / methods.

Available commands:

```text
replace-cpp-class  <ClassName>
replace-cpp-method <ClassName> <methodName>
replace-cpp-method <ClassName::methodName>

replace-py-class   <ClassName>
replace-py-method  <ClassName> <methodName>
replace-py-method  <ClassName.methodName>
```

### C++ examples

Replace a whole class:

```text
=== file: src/TEST.h ===
--- replace-cpp-class TargetClass
class TargetClass
{
  public:
    TargetClass() = default;
    ~TargetClass() = default;

    int value() const { return member_variable_; }
    void set_value(int v) { member_variable_ = v; }

  private:
    int member_variable_ = 0;
};
=END=
```

Replace a method inside a class:

```text
=== file: src/TEST.h ===
--- replace-cpp-method ClassWithTargetMethod::TargetMethod
    void TargetMethod()
    {
        // Updated implementation
        member_variable_ += 1;
    }
=END=
```

The C++ finder:

* skips comments, string / char literals and preprocessor lines,
* ignores forward declarations (`class Foo;`),
* replaces exactly the region of the target class / method.

### Python examples

Replace a whole class:

```text
=== file: src/foo.py ===
--- replace-py-class Foo
class Foo:
    def __init__(self):
        self.x = 2

    def answer(self):
        return 42
=END=
```

Replace a method (sync or async) inside a class:

```text
=== file: src/weird.py ===
--- replace-py-method Weird run
    def run(self):
        return 100
=END=
```

```text
=== file: src/async_foo.py ===
--- replace-py-method Foo.bar
    async def bar(self):
        return 2
=END=
```

The Python finder:

* uses indentation to determine class / method bodies,
* includes decorators (`@something`) above the method,
* replaces the whole method body region.

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

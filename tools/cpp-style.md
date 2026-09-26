# C++ formatting and static analysis

Use LLVM 20 for both tools. On macOS, install it with `brew install llvm@20`.
The project wrapper finds Homebrew's keg-only installation automatically. If LLVM
is installed elsewhere, set `HAYAKU_LLVM_BIN` to the directory containing both
`clang-format` and `clang-tidy`.

The existing `.clang-format` uses Google as its base but intentionally retains
the project's 4-space indentation and 100-column limit. `.clang-tidy` starts
with a small set of bug, performance, and Google-style checks. It does not
enforce Google naming on established public APIs.

Run checks on explicit files; there is no automatic whole-tree reformat:

```sh
python3 tools/cpp_style.py format hayaku_cpp/src/execution/OrderOrigin.cpp
python3 tools/cpp_style.py format --write hayaku_cpp/src/execution/OrderOrigin.cpp
```

For clang-tidy, first generate Xmake's compilation database after configuring
the project. If Xmake detects the wrong Python version, set `HAYAKU_PYTHON` to an
installed Python whose matching `pythonX.Y-config` command is available.

```sh
xmake project -k compile_commands --lsp=clangd
python3 tools/cpp_style.py tidy hayaku_cpp/src/execution/OrderOrigin.cpp
python3 tools/cpp_style.py tidy --strict hayaku_cpp/src/execution/OrderOrigin.cpp
```

For example, on a Mac where the project uses Homebrew Python 3.10 instead of
the default `python3`, generate the database with:

```sh
HAYAKU_PYTHON="$(command -v python3.10)" xmake project -k compile_commands --lsp=clangd
```

Regenerate `compile_commands.json` whenever Xmake options or source files change.
The file is ignored by Git. The default tidy command reports findings without
failing on existing warnings; `--strict` makes reported warnings fail the command.
Warnings from third-party or otherwise unselected headers can appear in the
suppressed-warning count. Neither command modifies source files unless
`format --write` is explicitly requested.

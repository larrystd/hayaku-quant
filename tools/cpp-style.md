# C++ formatting and static analysis

The project uses LLVM 20 (`clang-format` and `clang-tidy`) and the unmodified
Google C++ formatting style. On macOS, install `llvm@20` with Homebrew. The
wrapper finds Homebrew's keg-only tools automatically; set `HAYAKU_LLVM_BIN`
when they live elsewhere. `./op.sh doctor` prints the selected versions.

Only tracked handwritten `.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, and
`.hxx` files under `hayaku_cpp/src`, `hayaku_cpp/test`, `hayaku_cpp/demo`,
`hayaku_pywrap`, `hayaku_ingest_native`, and `hayaku_realtime_native` are in
the whole-tree format list. Generated files, third-party sources, and `.h.in`
templates are excluded. The list is regenerated from Git on every run.

```sh
./op.sh fmt-check
./op.sh fmt-check hayaku_cpp/src/execution/OrderOrigin.cpp
./op.sh fmt hayaku_cpp/src/execution/OrderOrigin.cpp
./op.sh fmt-all
```

`fmt-check` only reads files. `fmt` only writes its explicit arguments, while
`fmt-all` is the deliberate whole-tree rewrite. Build and test commands never
format files.

Generate Xmake's compilation database after configuring, and again whenever
the target or build options change. Tidy accepts only compilation units listed
in that database and never applies fixes:

```sh
HAYAKU_PYTHON="$(command -v python3.10)" xmake project -k compile_commands --lsp=clangd
./op.sh tidy hayaku_cpp/src/execution/OrderOrigin.cpp
./op.sh tidy-strict hayaku_cpp/src/execution/OrderOrigin.cpp
```

`.clang-tidy` starts with analyzer, bug, performance, and applicable Google
checks. Existing warnings are recorded before strict checks become a gate for
individual files. Public API names are handled in the interface review, not
automatically renamed by tidy.

`./op.sh asan-test` uses `build/asan` and instruments project targets while
reusing the ordinary third-party packages. It restores the prior Xmake
configuration on exit. On macOS, the command sets `detect_leaks=0` because
LeakSanitizer is unavailable; Linux keeps leak detection enabled. Rebuild the
ordinary `core`, `ingest`, and `realtime` targets after this command before
using the Python extensions, since Xmake copies built extensions into the
source package directories.

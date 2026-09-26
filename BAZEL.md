# Bazel build (macOS and Linux)

Install Bazelisk, CMake, and a C++20 compiler. Bazelisk reads the pinned Bazel
version from `.bazelversion`. On macOS, the deployment target is 13.3 because
the C++ standard library used here requires that version for floating point
`std::format`.

For a source-tree build and Python 3.10 import, run `./op.sh build`. The wrapper
builds the targets below and stages their native outputs in the Python packages.
Use `./op.sh test` for the full C++ suite and package smoke tests, or
`./op.sh wheel` to build the core wheel.
The optional wheels use `./op.sh wheel-ingest` and `./op.sh wheel-realtime`.
Their package definitions live in `tools/wheels/`; the core wheel uses the
standard root `setup.py` entry point.
Run `./op.sh compdb` after changing Bazel targets or options to refresh
`compile_commands.json` for clangd and clang-tidy.

```sh
bazel build //hayaku_cpp/src:core_shared //hayaku_cpp/src:realtime_shared //hayaku_cpp/src:ingest_shared \
  //hayaku_pywrap:core310 //hayaku_pywrap:realtime310 //hayaku_pywrap:ingest310
bazel test //hayaku_cpp/test:cval_test //hayaku_cpp/test:unit_test \
  //bazel:python_package_smoke_test --test_output=errors
```

The Python targets use a pinned Python 3.10 toolchain and produce versioned
extensions. To use them with the source tree's packages, copy the built files
to these paths:

| Bazel output | Python package path |
| --- | --- |
| `bazel-bin/hayaku_pywrap/core310.so` | `hayaku/cpp/core310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku.so` | `hayaku/cpp/libhayaku.so` |
| `bazel-bin/hayaku_pywrap/ingest310.so` | `hayaku_ingest_native/ingest310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku_ingest.so` | `hayaku_ingest_native/libhayaku_ingest.so` |
| `bazel-bin/hayaku_pywrap/realtime310.so` | `hayaku_realtime_native/realtime310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku_realtime.so` | `hayaku_realtime_native/libhayaku_realtime.so` |

The three native libraries share one process-wide core library. The package
smoke test copies the same layout to a temporary directory and imports all
three extensions.

The three C++ demos build with `bazel build //hayaku_cpp/demo:all`. The full
C++ test uses the checked-in `test_data`, `hayaku/plugin`, and `i18n` fixtures;
it runs 798 doctest cases on the current fixture set. Wheel payloads and
isolated imports are checked with `python3.10 bazel/check_wheels.py` after
building all three wheels.

The Bazel default configuration enables HDF5 (with zlib), SQLite, TDX, TA-Lib,
serialization, the ingest library, and the realtime library. MySQL and Windows
are outside this Bazel configuration. Feature macros are in `bazel/config/`.
The FlatBuffers header is generated from `spot.fbs` by the `spot_schema` rule.
The default HDF5 build is not thread-safe, so Factor calculations using that
driver read stocks serially; parallel-safe KData drivers retain parallel work.

Open-source dependencies are pinned in `MODULE.bazel`. Registry modules use
fixed versions recorded in `MODULE.bazel.lock`; source archives use immutable
commits and SHA-256 checksums. Update both the pin and the lockfile when
upgrading a dependency.

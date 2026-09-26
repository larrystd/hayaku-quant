# Bazel build (macOS and Linux)

Install Bazelisk, CMake, and a C++20 compiler. Bazelisk reads the pinned Bazel
version from `.bazelversion`. On macOS, the deployment target is 13.3 because
the C++ standard library used here requires that version for floating point
`std::format`.

Run `./op.sh build` and `./op.sh test` for the default C++ workflow. To build
the retained Python 3.10 integration, run `./op.sh python-build`, followed by
`./op.sh python-smoke` or `./op.sh python-test`.
Run `./op.sh ci` before submitting changes. It builds every target under
`//hayaku_cpp/...` and runs the focused CVAL and full C++ unit tests; the
macOS/Linux Bazel workflow uses the same command.
Use `./op.sh wheel` to build the core wheel.
The optional wheels use `./op.sh wheel-ingest` and `./op.sh wheel-realtime`.
Their package definitions live in `python/wheels/`; the core wheel uses
`python/setup.py`. All wheels are written to `python/dist/`.
Run `./op.sh compdb` after changing Bazel targets or options to refresh
`compile_commands.json` for clangd and clang-tidy.

```sh
bazel build //hayaku_cpp/src:core_shared //hayaku_cpp/src:realtime_shared //hayaku_cpp/src:ingest_shared
bazel test //hayaku_cpp/test:cval_test //hayaku_cpp/test:unit_test --test_output=errors
bazel build //python/hayaku_pywrap:core310 //python/hayaku_pywrap:realtime310 //python/hayaku_pywrap:ingest310
bazel test //python:python_package_smoke_test --test_output=errors
```

The Python targets use a pinned Python 3.10 toolchain and produce versioned
extensions. To use them with the source tree's packages, copy the built files
to these paths:

| Bazel output | Python package path |
| --- | --- |
| `bazel-bin/python/hayaku_pywrap/core310.so` | `python/hayaku/cpp/core310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku.so` | `python/hayaku/cpp/libhayaku.so` |
| `bazel-bin/python/hayaku_pywrap/ingest310.so` | `python/hayaku_ingest_native/ingest310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku_ingest.so` | `python/hayaku_ingest_native/libhayaku_ingest.so` |
| `bazel-bin/python/hayaku_pywrap/realtime310.so` | `python/hayaku_realtime_native/realtime310.so` |
| `bazel-bin/hayaku_cpp/src/libhayaku_realtime.so` | `python/hayaku_realtime_native/libhayaku_realtime.so` |

The three native libraries share one process-wide core library. The package
smoke test copies the same layout to a temporary directory and imports all
three extensions.

The three C++ demos build with `bazel build //hayaku_cpp/demo:all`. The full
C++ test uses the checked-in `test_data` fixtures, plus an empty
plugin directory created by its runner;
it runs 798 doctest cases on the current fixture set. Wheel payloads and
isolated imports are checked with `python3.10 python/tools/check_wheels.py` after
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

#!/usr/bin/env bash

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_PREFIX="${PYTHON_PREFIX:-/opt/homebrew/opt/python@3.10}"
PYTHON_BIN="${PYTHON_BIN:-${PYTHON_PREFIX}/bin/python3.10}"
PYTHON_LIBEXEC="${PYTHON_LIBEXEC:-${PYTHON_PREFIX}/libexec/bin}"
XMAKE_BIN="${XMAKE_BIN:-${HOME}/.local/bin/xmake}"
STYLE_PYTHON="${STYLE_PYTHON:-$(command -v python3 || true)}"
BUILD_KIND="${BUILD_KIND:-shared}"
BUILD_LIB="${PROJECT_DIR}/build/release/macosx/arm64/lib"

if [[ ! -x "${XMAKE_BIN}" ]]; then
    XMAKE_BIN="$(command -v xmake || true)"
fi

if [[ -x /usr/bin/xcrun ]]; then
    SDKROOT="${SDKROOT:-$(/usr/bin/xcrun --show-sdk-path)}"
else
    SDKROOT="${SDKROOT:-/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk}"
fi

export PATH="${PYTHON_LIBEXEC}:/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin"
export SDKROOT
export PYTHONPATH="${PROJECT_DIR}${PYTHONPATH:+:${PYTHONPATH}}"
export HAYAKU_PYTHON="${HAYAKU_PYTHON:-${PYTHON_BIN}}"

cd "${PROJECT_DIR}"

usage() {
    cat <<'EOF'
Usage: ./op.sh <command> [arguments]

Build commands:
  configure [shared|static]  Configure a release build (default: shared)
  build                      Build the C++ core and Python 3.10 extension
  build-optional             Build the ingest and realtime Python extensions
  rebuild [shared|static]    Configure, then build
  clean                      Remove xmake build outputs (keeps downloaded packages)

Test commands:
  small-test                 Build and run the small C++ test suite
  unit-test                  Build and run the complete C++ test suite
  python-test                Run the Python 3.10 test suite (build optional modules first)
  test                       Run all three test suites
  all [shared|static]        Configure, build, and run all tests
  asan-test                  Build and run C++ tests in an isolated ASan configuration

C++ quality commands (LLVM 20):
  fmt-check [files...]      Check named files, or all tracked handwritten C/C++ files
  fmt <files...>             Format only the named files
  fmt-all                    Format all tracked handwritten C/C++ files
  tidy <files...>            Inspect named compilation units without applying fixes
  tidy-strict <files...>     Fail on clang-tidy warnings in named units

Other commands:
  import-test                Import hayaku with Python 3.10 and print its version
  run-small-binary           Run the built small-test binary with its dylib path
  run-unit-binary            Run the built unit-test binary with its dylib path
  doctor                     Print the selected toolchain and artifact information
  help                       Show this help

Environment overrides:
  PYTHON_PREFIX              Homebrew Python prefix
  PYTHON_BIN                 Python executable
  PYTHON_LIBEXEC             Directory containing the `python` command
  HAYAKU_PYTHON              Python used by Xmake native extension targets
  XMAKE_BIN                  xmake executable (CI-compatible 3.0.8 recommended)
  BUILD_KIND                 Default library kind: shared or static
  STYLE_PYTHON               Python 3 executable for C++ quality commands
  HAYAKU_LLVM_BIN            Directory containing clang-format and clang-tidy 20

Examples:
  ./op.sh configure
  ./op.sh build
  ./op.sh build-optional
  ./op.sh small-test
  ./op.sh test
  ./op.sh all
  ./op.sh fmt-check
  ./op.sh fmt hayaku_cpp/src/execution/OrderOrigin.cpp
EOF
}

require_tools() {
    if [[ -z "${XMAKE_BIN}" || ! -x "${XMAKE_BIN}" ]]; then
        echo "Error: xmake was not found. Set XMAKE_BIN or install xmake." >&2
        exit 1
    fi
    if [[ ! -x "${PYTHON_BIN}" ]]; then
        echo "Error: Python 3.10 was not found at ${PYTHON_BIN}." >&2
        echo "Set PYTHON_PREFIX or PYTHON_BIN to the local Python 3.10 installation." >&2
        exit 1
    fi
}

configure() {
    local kind="${1:-${BUILD_KIND}}"
    if [[ "${kind}" != "shared" && "${kind}" != "static" ]]; then
        echo "Error: build kind must be 'shared' or 'static'." >&2
        exit 2
    fi
    "${XMAKE_BIN}" f -c -k "${kind}" -y --feedback=n
}

build() {
    "${XMAKE_BIN}" -b core
}

build_optional() {
    "${XMAKE_BIN}" -b ingest
    "${XMAKE_BIN}" -b realtime
}

small_test() {
    "${XMAKE_BIN}" r small-test
}

unit_test() {
    "${XMAKE_BIN}" r unit-test
}

python_test() {
    "${PYTHON_BIN}" tests/python/test.py
}

import_test() {
    "${PYTHON_BIN}" -c 'import sys, hayaku; print("python", sys.version.split()[0]); print("hayaku", hayaku.__version__); print("core", hayaku.Datetime(20240101))'
}

run_binary() {
    local name="$1"
    local binary="${BUILD_LIB}/${name}"
    if [[ ! -x "${binary}" ]]; then
        echo "Error: ${binary} does not exist. Run './op.sh ${name}' first." >&2
        exit 1
    fi
    (
        cd "${BUILD_LIB}"
        DYLD_LIBRARY_PATH="${BUILD_LIB}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}" "./${name}"
    )
}

doctor() {
    echo "Project:       ${PROJECT_DIR}"
    echo "Architecture:  $(uname -m)"
    echo "SDKROOT:       ${SDKROOT}"
    echo "Python:        ${PYTHON_BIN}"
    if [[ -x "${PYTHON_BIN}" ]]; then "${PYTHON_BIN}" --version; fi
    echo "xmake:         ${XMAKE_BIN}"
    if [[ -n "${XMAKE_BIN}" && -x "${XMAKE_BIN}" ]]; then
        "${XMAKE_BIN}" --version | head -n 1
    fi
    echo "LLVM tools:"
    if [[ -n "${STYLE_PYTHON}" ]]; then
        "${STYLE_PYTHON}" tools/cpp_style.py doctor
    else
        echo "  Python 3 not found"
    fi
    echo "Build kind:    ${BUILD_KIND}"
    echo "Build output:  ${BUILD_LIB}"
    if [[ -f "${PROJECT_DIR}/hayaku/cpp/core310.so" ]]; then
        ls -lh "${PROJECT_DIR}/hayaku/cpp/core310.so"
    else
        echo "Python core:   not built"
    fi
}

style_python() {
    if [[ -z "${STYLE_PYTHON}" || ! -x "${STYLE_PYTHON}" ]]; then
        echo "Error: Python 3 was not found. Set STYLE_PYTHON." >&2
        exit 1
    fi
    "${STYLE_PYTHON}" tools/cpp_style.py "$@"
}

asan_test() (
    if [[ "$(uname -s)" != "Darwin" && "$(uname -s)" != "Linux" ]]; then
        echo "Error: asan-test currently supports macOS and Linux." >&2
        exit 2
    fi
    local saved_config
    saved_config="$(mktemp)"
    "${XMAKE_BIN}" f --export="${saved_config}" -y >/dev/null
    restore_config() {
        "${XMAKE_BIN}" f --import="${saved_config}" -y >/dev/null
        rm -f "${saved_config}"
    }
    trap restore_config EXIT
    "${XMAKE_BIN}" f -m release -k shared --leak_check=y --builddir=build/asan -y --feedback=n
    local target
    for target in small-test unit-test ingest realtime core; do
        "${XMAKE_BIN}" -b "${target}"
    done
    if [[ "$(uname -s)" == "Darwin" ]]; then
        export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
    else
        export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1}"
    fi
    "${XMAKE_BIN}" r small-test
    "${XMAKE_BIN}" r unit-test
)

command_name="${1:-help}"
shift || true

case "${command_name}" in
    configure|build|build-optional|rebuild|clean|small-test|unit-test|python-test|test|all|import-test|run-small-binary|run-unit-binary|asan-test)
        require_tools
        ;;
esac

case "${command_name}" in
    configure)
        configure "${1:-${BUILD_KIND}}"
        ;;
    build)
        build
        ;;
    build-optional)
        build_optional
        ;;
    rebuild)
        configure "${1:-${BUILD_KIND}}"
        build
        ;;
    clean)
        "${XMAKE_BIN}" clean
        ;;
    small-test)
        small_test
        ;;
    unit-test)
        unit_test
        ;;
    python-test)
        python_test
        ;;
    test)
        small_test
        unit_test
        build_optional
        python_test
        ;;
    all)
        configure "${1:-${BUILD_KIND}}"
        build
        small_test
        unit_test
        build_optional
        python_test
        ;;
    fmt-check)
        if [[ $# -eq 0 ]]; then
            style_python format --all
        else
            style_python format "$@"
        fi
        ;;
    fmt)
        if [[ $# -eq 0 ]]; then echo "Error: fmt requires explicit files." >&2; exit 2; fi
        style_python format --write "$@"
        ;;
    fmt-all)
        if [[ $# -ne 0 ]]; then echo "Error: fmt-all takes no files." >&2; exit 2; fi
        style_python format --all --write
        ;;
    tidy)
        if [[ $# -eq 0 ]]; then echo "Error: tidy requires explicit files." >&2; exit 2; fi
        style_python tidy "$@"
        ;;
    tidy-strict)
        if [[ $# -eq 0 ]]; then echo "Error: tidy-strict requires explicit files." >&2; exit 2; fi
        style_python tidy --strict "$@"
        ;;
    asan-test)
        asan_test
        ;;
    import-test)
        import_test
        ;;
    run-small-binary)
        run_binary small-test
        ;;
    run-unit-binary)
        run_binary unit-test
        ;;
    doctor)
        doctor
        ;;
    help|-h|--help)
        usage
        ;;
    *)
        echo "Error: unknown command '${command_name}'." >&2
        usage >&2
        exit 2
        ;;
esac

#!/usr/bin/env bash

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_PREFIX="${PYTHON_PREFIX:-/opt/homebrew/opt/python@3.10}"
PYTHON_BIN="${PYTHON_BIN:-${PYTHON_PREFIX}/bin/python3.10}"
PYTHON_LIBEXEC="${PYTHON_LIBEXEC:-${PYTHON_PREFIX}/libexec/bin}"
XMAKE_BIN="${XMAKE_BIN:-${HOME}/.local/bin/xmake}"
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

cd "${PROJECT_DIR}"

usage() {
    cat <<'EOF'
Usage: ./op.sh <command> [arguments]

Build commands:
  configure [shared|static]  Configure a release build (default: shared)
  build                      Build the C++ core and Python 3.10 extension
  rebuild [shared|static]    Configure, then build
  clean                      Remove xmake build outputs (keeps downloaded packages)

Test commands:
  small-test                 Build and run the small C++ test suite
  unit-test                  Build and run the complete C++ test suite
  python-test                Run the Python 3.10 test suite
  test                       Run all three test suites
  all [shared|static]        Configure, build, and run all tests

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
  XMAKE_BIN                  xmake executable (CI-compatible 3.0.8 recommended)
  BUILD_KIND                 Default library kind: shared or static

Examples:
  ./op.sh configure
  ./op.sh build
  ./op.sh small-test
  ./op.sh test
  ./op.sh all
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
    "${PYTHON_BIN}" --version
    echo "xmake:         ${XMAKE_BIN}"
    "${XMAKE_BIN}" --version | head -n 1
    echo "Build kind:    ${BUILD_KIND}"
    echo "Build output:  ${BUILD_LIB}"
    if [[ -f "${PROJECT_DIR}/hayaku/cpp/core310.so" ]]; then
        ls -lh "${PROJECT_DIR}/hayaku/cpp/core310.so"
    else
        echo "Python core:   not built"
    fi
}

require_tools

command_name="${1:-help}"
shift || true

case "${command_name}" in
    configure)
        configure "${1:-${BUILD_KIND}}"
        ;;
    build)
        build
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
        python_test
        ;;
    all)
        configure "${1:-${BUILD_KIND}}"
        build
        small_test
        unit_test
        python_test
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

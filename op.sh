#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAZEL_BIN="${BAZEL_BIN:-$(command -v bazelisk || command -v bazel || true)}"
PYTHON_BIN="${PYTHON_BIN:-$(command -v python3.10 || true)}"
STAGE_PYTHON="${STAGE_PYTHON:-$(command -v python3 || true)}"
STYLE_PYTHON="${STYLE_PYTHON:-${STAGE_PYTHON}}"
CPP_TEST_TARGETS=(
    //hayaku_cpp/test:cval_test
    //hayaku_cpp/test:unit_test
)

cd "${PROJECT_DIR}"

usage() {
    cat <<'HELP'
Usage: ./op.sh <command> [arguments]

Build and package:
  build             Build the macOS/Linux libraries and Python 3.10 extensions
  clean             Remove Bazel build outputs
  wheel             Build a core Python wheel with native libraries and C++ headers
  wheel-ingest      Build the optional ingestion wheel
  wheel-realtime    Build the optional realtime wheel

Tests:
  ci                Build all C++ targets and run the required C++ tests
  test              Run Bazel C++ and Python package smoke tests
  python-test       Run the source-tree Python regression suite
  all               Build, run Bazel tests, then run Python tests
  import-test       Import the staged Python package and print its version

C++ quality:
  fmt-check [files...]   Check named files, or all tracked handwritten C++ files
  fmt <files...>          Format only named files
  fmt-all                 Format all tracked handwritten C++ files
  compdb                  Generate compile_commands.json from Bazel targets
  tidy <files...>         Run clang-tidy with an existing compile_commands.json
  tidy-strict <files...>  Fail on clang-tidy warnings
  doctor                  Print the selected tools and artifact paths

Overrides: BAZEL_BIN, PYTHON_BIN, STAGE_PYTHON, STYLE_PYTHON, HAYAKU_LLVM_BIN
HELP
}

require_bazel() {
    if [[ -z "${BAZEL_BIN}" || ! -x "${BAZEL_BIN}" ]]; then
        echo "Bazelisk or Bazel was not found; set BAZEL_BIN." >&2
        exit 1
    fi
}

require_stage_python() {
    if [[ -z "${STAGE_PYTHON}" || ! -x "${STAGE_PYTHON}" ]]; then
        echo "Python 3 was not found; set STAGE_PYTHON." >&2
        exit 1
    fi
}

require_python310() {
    if [[ -z "${PYTHON_BIN}" || ! -x "${PYTHON_BIN}" ]]; then
        echo "Python 3.10 was not found; set PYTHON_BIN." >&2
        exit 1
    fi
}

build() {
    require_bazel
    require_stage_python
    "${BAZEL_BIN}" build \
        //hayaku_cpp/src:core_shared \
        //hayaku_cpp/src:ingest_shared \
        //hayaku_cpp/src:realtime_shared \
        //hayaku_pywrap:core310 \
        //hayaku_pywrap:ingest310 \
        //hayaku_pywrap:realtime310
    "${STAGE_PYTHON}" bazel/stage_python.py
}

bazel_test() {
    require_bazel
    "${BAZEL_BIN}" test \
        "${CPP_TEST_TARGETS[@]}" \
        //bazel:python_package_smoke_test \
        --test_output=errors
}

cpp_ci() {
    require_bazel
    "${BAZEL_BIN}" build //hayaku_cpp/...
    "${BAZEL_BIN}" test "${CPP_TEST_TARGETS[@]}" --test_output=errors
}

python_test() {
    require_python310
    PYTHONPATH="${PROJECT_DIR}${PYTHONPATH:+:${PYTHONPATH}}" \
        "${PYTHON_BIN}" tests/python/test.py
}

style_python() {
    if [[ -z "${STYLE_PYTHON}" || ! -x "${STYLE_PYTHON}" ]]; then
        echo "Python 3 was not found; set STYLE_PYTHON." >&2
        exit 1
    fi
    "${STYLE_PYTHON}" tools/cpp_style.py "$@"
}

case "${1:-help}" in
    ci)
        cpp_ci
        ;;
    build)
        build
        ;;
    clean)
        require_bazel
        "${BAZEL_BIN}" clean
        ;;
    test)
        bazel_test
        ;;
    python-test)
        python_test
        ;;
    all)
        build
        bazel_test
        python_test
        ;;
    import-test)
        require_python310
        PYTHONPATH="${PROJECT_DIR}${PYTHONPATH:+:${PYTHONPATH}}" \
            "${PYTHON_BIN}" -c 'import hayaku; print(hayaku.__version__)'
        ;;
    wheel|wheel-ingest|wheel-realtime)
        require_python310
        build
        case "$1" in
            wheel)
                "${STAGE_PYTHON}" bazel/stage_python.py --headers
                "${PYTHON_BIN}" setup.py bdist_wheel
                ;;
            wheel-ingest)
                "${PYTHON_BIN}" tools/wheels/ingest_setup.py bdist_wheel
                ;;
            wheel-realtime)
                "${PYTHON_BIN}" tools/wheels/realtime_setup.py bdist_wheel
                ;;
        esac
        ;;
    fmt-check)
        shift
        if [[ $# -eq 0 ]]; then style_python format --all; else style_python format "$@"; fi
        ;;
    fmt)
        shift
        if [[ $# -eq 0 ]]; then echo "fmt requires explicit files." >&2; exit 2; fi
        style_python format --write "$@"
        ;;
    fmt-all)
        shift
        if [[ $# -ne 0 ]]; then echo "fmt-all takes no files." >&2; exit 2; fi
        style_python format --all --write
        ;;
    compdb)
        require_bazel
        require_stage_python
        BAZEL_BIN="${BAZEL_BIN}" "${STAGE_PYTHON}" bazel/compdb.py
        ;;
    tidy)
        shift
        if [[ $# -eq 0 ]]; then echo "tidy requires explicit files." >&2; exit 2; fi
        style_python tidy "$@"
        ;;
    tidy-strict)
        shift
        if [[ $# -eq 0 ]]; then echo "tidy-strict requires explicit files." >&2; exit 2; fi
        style_python tidy --strict "$@"
        ;;
    doctor)
        echo "Project: ${PROJECT_DIR}"
        echo "Bazel: ${BAZEL_BIN:-not found}"
        if [[ -n "${BAZEL_BIN}" && -x "${BAZEL_BIN}" ]]; then "${BAZEL_BIN}" --version; fi
        echo "Python 3.10: ${PYTHON_BIN:-not found}"
        if [[ -n "${PYTHON_BIN}" && -x "${PYTHON_BIN}" ]]; then "${PYTHON_BIN}" --version; fi
        echo "Native outputs: ${PROJECT_DIR}/bazel-bin"
        style_python doctor
        ;;
    help|-h|--help)
        usage
        ;;
    *)
        echo "Unknown command: $1" >&2
        usage >&2
        exit 2
        ;;
esac

#!/usr/bin/env python3
"""Run the project's pinned LLVM formatter and linter on explicit C++ files."""

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
LLVM_MAJOR = 20
FORMAT_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
TIDY_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx"}


def find_llvm_tool(name):
    """Find LLVM 20, including Homebrew's keg-only macOS installation."""
    tool_dir = os.environ.get("HAYAKU_LLVM_BIN")
    if tool_dir:
        candidates = [Path(tool_dir) / name]
    else:
        candidates = []
        versioned = shutil.which(f"{name}-{LLVM_MAJOR}")
        if versioned:
            candidates.append(Path(versioned))
        for prefix in ("/opt/homebrew", "/usr/local"):
            candidates.append(Path(prefix) / f"opt/llvm@{LLVM_MAJOR}/bin/{name}")
        unversioned = shutil.which(name)
        if unversioned:
            candidates.append(Path(unversioned))

    for candidate in candidates:
        if not candidate.is_file():
            continue
        result = subprocess.run([str(candidate), "--version"], capture_output=True, text=True)
        if result.returncode == 0 and re.search(
            rf"\bversion\s+{LLVM_MAJOR}(?:\.|\b)", result.stdout, re.IGNORECASE
        ):
            return candidate

    raise RuntimeError(
        f"{name} {LLVM_MAJOR} not found. On macOS: brew install llvm@{LLVM_MAJOR}; "
        "for a custom install, set HAYAKU_LLVM_BIN to its bin directory."
    )


def resolve_files(names, extensions):
    files = []
    for name in names:
        path = Path(name).resolve()
        if not path.is_file():
            raise ValueError(f"not a file: {name}")
        if path.suffix not in extensions:
            raise ValueError(f"not a supported C++ file: {name}")
        if not path.is_relative_to(PROJECT_ROOT):
            raise ValueError(f"file is outside the project: {name}")
        files.append(path)
    return files


def compile_database_files():
    database = PROJECT_ROOT / "compile_commands.json"
    if not database.is_file():
        raise RuntimeError(
            "compile_commands.json is missing; run "
            "'xmake project -k compile_commands --lsp=clangd' first."
        )
    entries = json.loads(database.read_text(encoding="utf-8"))
    return {
        (Path(entry["directory"]) / entry["file"]).resolve()
        for entry in entries
    }


def run_format(tool, files, write):
    options = ["-i"] if write else ["--dry-run", "--Werror"]
    status = 0
    for path in files:
        result = subprocess.run([str(tool), "--style=file", *options, str(path)], cwd=PROJECT_ROOT)
        status |= result.returncode
    return status


def run_tidy(tool, files, strict):
    available = compile_database_files()
    missing = [path for path in files if path not in available]
    if missing:
        raise ValueError(
            "not found in compile_commands.json (check the Xmake configuration): "
            + ", ".join(str(path.relative_to(PROJECT_ROOT)) for path in missing)
        )

    options = ["--warnings-as-errors=*"] if strict else []
    status = 0
    for path in files:
        result = subprocess.run(
            [str(tool), "-p", str(PROJECT_ROOT), "--config-file",
             str(PROJECT_ROOT / ".clang-tidy"),
             *options, str(path)],
            cwd=PROJECT_ROOT,
        )
        status |= result.returncode
    return status


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    format_command = commands.add_parser("format", help="check formatting (or write with --write)")
    format_command.add_argument("--write", action="store_true", help="format the named files in place")
    format_command.add_argument("files", nargs="+", help="explicit project C/C++ files")
    tidy_command = commands.add_parser("tidy", help="analyze compiled C/C++ source files")
    tidy_command.add_argument("--strict", action="store_true", help="fail on clang-tidy warnings")
    tidy_command.add_argument("files", nargs="+", help="explicit project C/C++ source files")
    args = parser.parse_args(argv)

    try:
        if args.command == "format":
            files = resolve_files(args.files, FORMAT_EXTENSIONS)
            return run_format(find_llvm_tool("clang-format"), files, args.write)
        files = resolve_files(args.files, TIDY_EXTENSIONS)
        return run_tidy(find_llvm_tool("clang-tidy"), files, args.strict)
    except (OSError, ValueError, RuntimeError, json.JSONDecodeError) as error:
        print(f"cpp_style: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Reject direct application dependencies in lower-level C/C++ sources.

This is a source-level guard, not a C++ preprocessor or dependency graph.
Run from any directory: python3 tools/arch/check_app_dependencies.py
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_DIRS = (
    "hayaku_cpp/src/data",
    "hayaku_cpp/src/operators",
    "hayaku_cpp/src/metrics",
)
CPP_SUFFIXES = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl", ".ipp", ".tpp"
}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
APP_INCLUDE_RE = re.compile(r"(?:^|/)application/")
APP_PATH_RE = re.compile(r"(?<![A-Za-z0-9_])src/application(?:/|\b)")


def without_comments(source: str) -> str:
    """Mask comments while preserving strings, line numbers, and source offsets."""
    output: list[str] = []
    index = 0
    block_comment = False
    while index < len(source):
        if block_comment:
            if source.startswith("*/", index):
                output.extend("  ")
                index += 2
                block_comment = False
            else:
                output.append("\n" if source[index] == "\n" else " ")
                index += 1
            continue

        if source.startswith("//", index):
            end = source.find("\n", index)
            if end < 0:
                output.extend(" " * (len(source) - index))
                break
            output.extend(" " * (end - index))
            index = end
            continue
        if source.startswith("/*", index):
            output.extend("  ")
            index += 2
            block_comment = True
            continue

        if source.startswith('R"', index):
            open_paren = source.find("(", index + 2)
            if open_paren >= 0 and "\n" not in source[index:open_paren]:
                delimiter = source[index + 2 : open_paren]
                if len(delimiter) <= 16 and not any(char.isspace() for char in delimiter):
                    closing = source.find(")" + delimiter + '"', open_paren + 1)
                    if closing >= 0:
                        end = closing + len(delimiter) + 2
                        output.append(source[index:end])
                        index = end
                        continue

        if source[index] in {'"', "'"}:
            quote = source[index]
            start = index
            index += 1
            while index < len(source):
                if source[index] == "\\":
                    index += 2
                elif source[index] == quote:
                    index += 1
                    break
                else:
                    index += 1
            output.append(source[start:index])
            continue

        output.append(source[index])
        index += 1

    return "".join(output)


def scan(root: Path) -> tuple[int, list[str]]:
    violations: list[str] = []
    scanned = 0
    for directory_name in SOURCE_DIRS:
        directory = root / directory_name
        if not directory.is_dir():
            raise FileNotFoundError(f"missing source directory: {directory}")
        for path in sorted(directory.rglob("*")):
            if not path.is_file() or path.suffix.lower() not in CPP_SUFFIXES:
                continue
            scanned += 1
            source = path.read_text(encoding="utf-8")
            visible = without_comments(source)
            for line_number, line in enumerate(visible.splitlines(), 1):
                reasons: list[str] = []
                include = INCLUDE_RE.match(line)
                if include and APP_INCLUDE_RE.search(include.group(1).replace("\\", "/")):
                    reasons.append("application include")
                if APP_PATH_RE.search(line.replace("\\", "/")):
                    reasons.append("src/application path")
                if reasons:
                    relative_path = path.relative_to(root)
                    violations.append(f"{relative_path}:{line_number}: {', '.join(reasons)}")
    return scanned, violations


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root", type=Path, default=ROOT, help="repository root (default: script location)"
    )
    args = parser.parse_args()
    try:
        scanned, violations = scan(args.root.resolve())
    except (OSError, UnicodeError) as error:
        parser.exit(2, f"dependency scan failed: {error}\n")

    if violations:
        print("\n".join(violations))
        print(f"Found {len(violations)} app dependency line(s) in {scanned} C/C++ files.")
        return 1
    print(f"OK: scanned {scanned} C/C++ files; no app dependencies found.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

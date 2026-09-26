#!/usr/bin/env python3
"""Generate compile_commands.json from Bazel C++ compile actions."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
QUERY = 'mnemonic("CppCompile", //... + //hayaku_cpp/test:unit_test_bin)'
SOURCE_ROOTS = ("hayaku_cpp/", "python/hayaku_pywrap/")


def bazel_output(bazel: str, *arguments: str) -> str:
    result = subprocess.run(
        [bazel, *arguments], cwd=ROOT, text=True, capture_output=True
    )
    if result.returncode:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def main() -> None:
    bazel = os.environ.get("BAZEL_BIN") or shutil.which("bazelisk") or shutil.which("bazel")
    if not bazel:
        raise RuntimeError("Bazelisk or Bazel was not found; set BAZEL_BIN")
    execution_root = Path(bazel_output(bazel, "info", "execution_root").strip())
    actions = json.loads(
        bazel_output(bazel, "aquery", QUERY, "--output=jsonproto")
    )["actions"]

    entries = []
    for action in actions:
        arguments = action.get("arguments", [])
        if "-c" not in arguments:
            continue
        source = arguments[arguments.index("-c") + 1]
        if not source.startswith(SOURCE_ROOTS):
            continue
        path = ROOT / source
        if not path.is_file():
            continue
        entries.append({
            "directory": str(execution_root),
            "file": str(path),
            "arguments": arguments,
        })

    if not entries:
        raise RuntimeError("Bazel returned no local C++ compile actions")
    entries.sort(key=lambda entry: entry["file"])
    destination = ROOT / "compile_commands.json"
    temporary = destination.with_suffix(".json.tmp")
    try:
        temporary.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
        temporary.replace(destination)
    finally:
        temporary.unlink(missing_ok=True)
    print(f"Wrote {len(entries)} Bazel compile commands to {destination}")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, ValueError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)

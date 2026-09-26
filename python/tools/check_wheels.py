#!/usr/bin/env python3
"""Check Bazel wheel payloads and import them outside the source tree."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
from zipfile import ZipFile


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_NATIVE = {
    "hayaku-": {"hayaku/cpp/core310.so", "hayaku/cpp/libhayaku.so"},
    "hayaku_ingest-": {
        "hayaku_ingest_native/ingest310.so",
        "hayaku_ingest_native/libhayaku_ingest.so",
    },
    "hayaku_realtime-": {
        "hayaku_realtime_native/realtime310.so",
        "hayaku_realtime_native/libhayaku_realtime.so",
    },
}
NATIVE_SUFFIXES = (".so", ".dylib", ".dll", ".pyd")


def main() -> None:
    wheels = []
    for prefix, expected in EXPECTED_NATIVE.items():
        matches = list((ROOT / "dist").glob(f"{prefix}*.whl"))
        if len(matches) != 1:
            raise RuntimeError(f"Expected one {prefix} wheel, found {matches}")
        wheel = matches[0]
        with ZipFile(wheel) as archive:
            names = set(archive.namelist())
            native = {name for name in names if name.endswith(NATIVE_SUFFIXES)}
        if native != expected:
            raise RuntimeError(f"Unexpected native payload in {wheel.name}: {sorted(native)}")
        if prefix == "hayaku-" and "hayaku/include/hayaku/strategy/Strategy.h" not in names:
            raise RuntimeError(f"Core C++ headers are missing from {wheel.name}")
        print(f"{wheel.name}: {len(native)} Bazel libraries")
        wheels.append(wheel)

    with tempfile.TemporaryDirectory(prefix="hayaku-wheel-check-") as directory:
        destination = Path(directory)
        for wheel in wheels:
            with ZipFile(wheel) as archive:
                archive.extractall(destination)
        environment = dict(os.environ, PYTHONPATH=directory)
        check = (
            "import hayaku, hayaku_ingest_native.ingest310, "
            "hayaku_realtime_native.realtime310; "
            "print(hayaku.__version__); print(hayaku.__file__)"
        )
        result = subprocess.run(
            [sys.executable, "-c", check],
            cwd=destination,
            env=environment,
            text=True,
            capture_output=True,
        )
        if result.returncode != 0 or directory not in result.stdout:
            raise RuntimeError(result.stdout + result.stderr)
        print("Isolated wheel imports passed")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Copy Bazel native outputs into the source-tree Python package layout."""

import argparse
import os
from pathlib import Path
import shutil
import tempfile


ROOT = Path(__file__).resolve().parents[1]
NATIVE_FILES = {
    "hayaku/cpp/core310.so": "hayaku_pywrap/core310.so",
    "hayaku/cpp/libhayaku.so": "hayaku_cpp/src/libhayaku.so",
    "hayaku_ingest_native/ingest310.so": "hayaku_pywrap/ingest310.so",
    "hayaku_ingest_native/libhayaku_ingest.so": "hayaku_cpp/src/libhayaku_ingest.so",
    "hayaku_realtime_native/realtime310.so": "hayaku_pywrap/realtime310.so",
    "hayaku_realtime_native/libhayaku_realtime.so": "hayaku_cpp/src/libhayaku_realtime.so",
}
EXCLUDED_HEADERS = {
    Path("config.h"),
    Path("version.h"),
    Path("common/Config.h"),
    Path("extensions/realtime/spot_generated.h"),
}


def copy_file(source: Path, destination: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(f"Missing Bazel output: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary_name = tempfile.mkstemp(prefix=f".{destination.name}.", dir=destination.parent)
    os.close(fd)
    temporary = Path(temporary_name)
    try:
        shutil.copy2(source, temporary)
        temporary.replace(destination)
    finally:
        temporary.unlink(missing_ok=True)


def stage_headers(destination_root: Path) -> None:
    source_root = ROOT / "hayaku_cpp/src"
    include_root = destination_root / "hayaku/include"
    for source in source_root.rglob("*.h"):
        relative = source.relative_to(source_root)
        if relative not in EXCLUDED_HEADERS:
            copy_file(source, include_root / "hayaku" / relative)

    generated = {
        ROOT / "bazel/config/config.h": include_root / "hayaku/config.h",
        ROOT / "bazel/config/version.h": include_root / "hayaku/version.h",
        ROOT / "bazel/config/common/Config.h": include_root / "hayaku/common/Config.h",
        ROOT / "bazel-bin/hayaku_cpp/src/extensions/realtime/spot_generated.h":
            include_root / "hayaku/extensions/realtime/spot_generated.h",
        ROOT / "hayaku_pywrap/common/PybindSupport.h":
            include_root / "hayaku/python/PybindSupport.h",
        ROOT / "hayaku_pywrap/common/PickleSupport.h":
            include_root / "hayaku/python/PickleSupport.h",
        ROOT / "hayaku_pywrap/common/AnyConversion.h":
            include_root / "hayaku/python/AnyConversion.h",
    }
    for source, destination in generated.items():
        copy_file(source, destination)

    (include_root / "__init__.py").touch()
    for directory in (include_root / "hayaku").rglob("*"):
        if directory.is_dir():
            (directory / "__init__.py").touch()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-root", type=Path, default=ROOT)
    parser.add_argument("--headers", action="store_true", help="stage C++ headers for wheels")
    args = parser.parse_args()
    output_root = args.output_root.resolve()
    for destination, source in NATIVE_FILES.items():
        copy_file(ROOT / "bazel-bin" / source, output_root / destination)
        print(destination)
    if args.headers:
        stage_headers(output_root)
        print("hayaku/include")


if __name__ == "__main__":
    main()

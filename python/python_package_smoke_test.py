"""Load the Bazel extensions from the directory layout used by the Python packages."""

import importlib
import os
from pathlib import Path
import shutil
import sys
import tempfile


runfiles = Path(os.environ["TEST_SRCDIR"]) / os.environ["TEST_WORKSPACE"]

with tempfile.TemporaryDirectory() as temporary:
    package_root = Path(temporary)
    files = {
        "hayaku/core.py": "python/hayaku/core.py",
        "hayaku/cpp/core310.so": "python/hayaku_pywrap/core310.so",
        "hayaku/cpp/libhayaku.so": "hayaku_cpp/src/libhayaku.so",
        "hayaku_ingest_native/__init__.py": "python/hayaku_ingest_native/__init__.py",
        "hayaku_ingest_native/ingest310.so": "python/hayaku_pywrap/ingest310.so",
        "hayaku_ingest_native/libhayaku_ingest.so": "hayaku_cpp/src/libhayaku_ingest.so",
        "hayaku_realtime_native/__init__.py": "python/hayaku_realtime_native/__init__.py",
        "hayaku_realtime_native/realtime310.so": "python/hayaku_pywrap/realtime310.so",
        "hayaku_realtime_native/libhayaku_realtime.so": "hayaku_cpp/src/libhayaku_realtime.so",
    }
    for destination, source in files.items():
        target = package_root / destination
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(runfiles / source, target)
    (package_root / "hayaku/__init__.py").touch()
    (package_root / "hayaku/cpp/__init__.py").touch()

    sys.path.insert(0, str(package_root))
    core = importlib.import_module("hayaku.core")
    ingest = importlib.import_module("hayaku_ingest_native.ingest310")
    realtime = importlib.import_module("hayaku_realtime_native")

    assert core.get_version() == "2.8.2"
    assert hasattr(core, "TA_MA")
    assert hasattr(ingest, "KDataToHdf5Importer")
    assert hasattr(realtime, "start_spot_agent")

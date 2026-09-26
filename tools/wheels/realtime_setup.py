"""Build the optional realtime wheel after `./op.sh build`.

The wheel owns only ``hayaku_realtime_native``. The core wheel retains the
public ``hayaku.extensions.realtime`` entry module without the optional native binary.
"""

import os
import re
import shutil
from pathlib import Path

from setuptools import setup
from setuptools.command.build_py import build_py
from setuptools.dist import Distribution


ROOT = Path(__file__).resolve().parents[2]
os.chdir(ROOT)
NATIVE_PACKAGE = ROOT / "hayaku_realtime_native"
VERSION_MATCH = re.search(
    r'^module\(name = "hayaku", version = "([^"]+)"\)',
    (ROOT / "MODULE.bazel").read_text(encoding="utf-8"),
    re.MULTILINE,
)
if VERSION_MATCH is None:
    raise RuntimeError("Cannot read Hayaku version from MODULE.bazel")
VERSION = VERSION_MATCH.group(1)

NATIVE_BINARIES = list(NATIVE_PACKAGE.glob("realtime*.so")) + list(NATIVE_PACKAGE.glob("realtime*.pyd"))
if not NATIVE_BINARIES:
    raise RuntimeError("Build and stage the native extensions with `./op.sh build` first")


class BinaryDistribution(Distribution):
    """The extension is prebuilt by Bazel and included as package data."""

    def has_ext_modules(self):
        return True


class CleanBuildPy(build_py):
    """Discard native files left by earlier packaging runs."""

    def run(self):
        build_root = (ROOT / "build/package-realtime").resolve()
        build_lib = Path(self.build_lib).resolve()
        if build_root in build_lib.parents and build_lib.is_dir():
            shutil.rmtree(build_lib)
        super().run()


setup(
    name="hayaku-realtime",
    version=VERSION,
    description="Optional native realtime market data for Hayaku",
    packages=["hayaku_realtime_native"],
    options={"build": {"build_base": "build/package-realtime"}},
    cmdclass={"build_py": CleanBuildPy},
    package_data={
        "hayaku_realtime_native": [
            "realtime*.so", "libhayaku_realtime*.so", "libhayaku_realtime*.so.*",
        ],
    },
    include_package_data=False,
    install_requires=[f"hayaku=={VERSION}"],
    python_requires=">=3.10,<3.11",
    zip_safe=False,
    distclass=BinaryDistribution,
)

"""Build the optional native ingestion wheel after `xmake ingest`.

The wheel owns only ``hayaku_ingest_native``. The core wheel owns the public
``hayaku.ingest`` shim, avoiding overlapping files between distributions.
"""

import re
from pathlib import Path

from setuptools import setup
from setuptools.dist import Distribution


ROOT = Path(__file__).resolve().parent
NATIVE_PACKAGE = ROOT / "hayaku_ingest_native"
VERSION_MATCH = re.search(r'set_version\("([^"]+)"', (ROOT / "xmake.lua").read_text(encoding="utf-8"))
if VERSION_MATCH is None:
    raise RuntimeError("Cannot read Hayaku version from xmake.lua")
VERSION = VERSION_MATCH.group(1)

NATIVE_BINARIES = list(NATIVE_PACKAGE.glob("ingest*.so")) + list(NATIVE_PACKAGE.glob("ingest*.pyd"))
if not NATIVE_BINARIES:
    raise RuntimeError("Build the optional native extension with `xmake ingest` first")


class BinaryDistribution(Distribution):
    """The extension is prebuilt by xmake and included as package data."""

    def has_ext_modules(self):
        return True


setup(
    name="hayaku-ingest",
    version=VERSION,
    description="Optional native historical-data ingestion for Hayaku",
    packages=["hayaku_ingest_native"],
    package_data={
        "hayaku_ingest_native": [
            "ingest*.so", "ingest*.pyd", "libhayaku-ingest*.so",
            "libhayaku-ingest*.so.*", "libhayaku-ingest*.dylib", "hayaku-ingest*.dll",
        ],
    },
    include_package_data=False,
    install_requires=[f"hayaku=={VERSION}"],
    python_requires=">=3.10",
    zip_safe=False,
    distclass=BinaryDistribution,
)

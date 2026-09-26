"""Build the optional realtime wheel after `xmake realtime`.

The wheel owns only ``hayaku_realtime_native``. The core wheel retains the
public ``hayaku.extensions.realtime`` entry module without the optional native binary.
"""

import re
from pathlib import Path

from setuptools import setup
from setuptools.dist import Distribution


ROOT = Path(__file__).resolve().parent
NATIVE_PACKAGE = ROOT / "hayaku_realtime_native"
VERSION_MATCH = re.search(r'set_version\("([^"]+)"', (ROOT / "xmake.lua").read_text(encoding="utf-8"))
if VERSION_MATCH is None:
    raise RuntimeError("Cannot read Hayaku version from xmake.lua")
VERSION = VERSION_MATCH.group(1)

NATIVE_BINARIES = list(NATIVE_PACKAGE.glob("realtime*.so")) + list(NATIVE_PACKAGE.glob("realtime*.pyd"))
if not NATIVE_BINARIES:
    raise RuntimeError("Build the optional native extension with `xmake realtime` first")


class BinaryDistribution(Distribution):
    """The extension is prebuilt by xmake and included as package data."""

    def has_ext_modules(self):
        return True


setup(
    name="hayaku-realtime",
    version=VERSION,
    description="Optional native realtime market data for Hayaku",
    packages=["hayaku_realtime_native"],
    options={"build": {"build_base": "build/package-realtime"}},
    package_data={
        "hayaku_realtime_native": [
            "realtime*.so", "realtime*.pyd", "libhayaku-realtime*.so",
            "libhayaku-realtime*.so.*", "libhayaku-realtime*.dylib", "hayaku-realtime*.dll",
        ],
    },
    include_package_data=False,
    install_requires=[f"hayaku=={VERSION}"],
    python_requires=">=3.10",
    zip_safe=False,
    distclass=BinaryDistribution,
)

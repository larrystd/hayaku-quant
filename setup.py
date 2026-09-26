#!/usr/bin/env python
# -*- coding:utf-8 -*-

from pathlib import Path
import re
import shutil
import sys
try:
    from setuptools import find_packages, setup
    from setuptools.command.build_py import build_py
    from setuptools.dist import Distribution
except ImportError:
    from distutils.core import find_packages, setup
    from distutils.command.build_py import build_py
    from distutils.dist import Distribution


class BinaryDistribution(Distribution):
    """Mark wheels as platform-specific: native modules are bundled as package data."""

    def has_ext_modules(self):
        return True


class CleanBuildPy(build_py):
    """Discard stale modules after a package directory is moved or removed."""

    def run(self):
        build_root = Path("build/package-core").resolve()
        build_lib = Path(self.build_lib).resolve()
        if build_root in build_lib.parents and build_lib.is_dir():
            shutil.rmtree(build_lib)
        super().run()


def parse_requirements(filename):
    line_iter = (line.strip() for line in open(filename))
    return [line for line in line_iter if line and not line.startswith('#')]


requirements = parse_requirements('requirements.txt')

current_plat = sys.platform
# Remove the special handling of PyQt5
# if current_plat == 'linux':
#     requirements.remove('PyQt5')

root = Path(__file__).resolve().parent
module_text = (root / 'MODULE.bazel').read_text(encoding='utf-8')
version_match = re.search(r'^module\(name = "hayaku", version = "([^"]+)"\)', module_text, re.MULTILINE)
if version_match is None:
    raise RuntimeError('Cannot read Hayaku version from MODULE.bazel')
hayaku_version = version_match.group(1)

if any(command in sys.argv for command in ('bdist_wheel', 'install')):
    for native_name in ('core310.so', 'libhayaku.so'):
        if not (root / 'hayaku/cpp' / native_name).is_file():
            raise RuntimeError('Run ./op.sh build to stage Bazel native libraries before packaging')

print('current hayaku version:', hayaku_version)

hayaku_name = "hayaku"  # "hayaku-noarrow"
# hayaku_version = "1.0.9"
hayaku_author = "fasiondog"
hayaku_author_email = "fasiondog@sina.com"

hayaku_license = "MIT"
hayaku_keywords = [
    "quant", "trade", "System Trading", "backtester", "量化", "程序化交易", "量化交易",
    "系统交易"
]
hayaku_platforms = "Independant"
hayaku_url = "https://github.com/larrystd/hayaku-quant"

hayaku_description = "Hayaku Quant Framework for System Trading Analysis and backtester"
with open("./readme.md", encoding='utf-8') as f:
    hayaku_long_description = f.read()

hayaku_data_files = []

packages = find_packages(include=['hayaku', 'hayaku.*'])

setup(
    distclass=BinaryDistribution,
    cmdclass={'build_py': CleanBuildPy},
    name=hayaku_name,
    version=hayaku_version,
    description=hayaku_description,
    # long_description_content_type="text/x-rst",
    long_description_content_type='text/markdown',
    long_description=hayaku_long_description,
    author=hayaku_author,
    author_email=hayaku_author_email,
    license=hayaku_license,
    license_files=['LICENSE'],
    keywords=hayaku_keywords,
    platforms=hayaku_platforms,
    url=hayaku_url,
    packages=packages,
    options={'build': {'build_base': 'build/package-core'}},
    zip_safe=False,
    include_package_data=False,
    package_data={
        '': [
            '*.rst', '*.png', '*.md', '*.ipynb', '*.ini', '*.sql', '*.ui', '*.properties', '*.xml',
            'LICENSE.txt', '*.ico', '*.h', '*.mo', '*.qm',
        ],
        'hayaku.cpp': ['core310.so', 'libhayaku.so'],
    },
    data_files=hayaku_data_files,
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 5 - Production/Stable',

        # Indicate who your project is intended for
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Office/Business :: Financial',
        'Topic :: Office/Business :: Financial :: Investment',
        'Topic :: Scientific/Engineering :: Mathematics',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX :: Linux',
        'Operating System :: MacOS :: MacOS X',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.10',
    ],
    python_requires='>=3.10,<3.11',
    entry_points={
        # On win11, using the GUI mode times out immediately, so the download fails
        # 'gui_scripts': [
        #     'HayakuTDX=hayaku.application.gui.HayakuTDX:start',
        # ],
        'console_scripts': [
            'HayakuTDX=hayaku.application.gui.HayakuTDX:start',
            'importdata=hayaku.application.gui.importdata:main',
            'dataserver=hayaku.application.gui.dataserver:main',
            'shmserver=hayaku.application.gui.shmserver:main',
        ]
    },
    install_requires=requirements,
)

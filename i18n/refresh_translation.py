#!/usr/bin/env python3
"""Refresh zh_CN.po from current source without retaining an obsolete POT file."""

from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
PO = ROOT / "i18n/zh_CN.po"
SOURCE_ROOTS = (
    "hayaku_cpp/src",
    "hayaku_pywrap",
    "hayaku_ingest_native",
    "hayaku_realtime_native",
    "hayaku/extensions/visualization",
)


def run(*args):
    subprocess.run(args, cwd=ROOT, check=True)


def dynamic_performance_catalog(path):
    """Performance.report translates keys stored in a map, not htr literals."""
    source = (ROOT / "hayaku_cpp/src/metrics/Performance.cpp").read_text()
    mapping = source.split("legacyKeyMap() {", 1)[1].split("};", 1)[0]
    keys = re.findall(r'\{"([^"\\]+)", "([^"\\]+)"\}', mapping)
    if not keys:
        raise RuntimeError("Performance key map was not found")
    lines = ['msgid ""', 'msgstr ""', '"Content-Type: text/plain; charset=UTF-8\\n"', ""]
    for _, english in keys:
        lines.extend(
            [
                "#. Dynamic Performance.report key; keep this entry.",
                "#: hayaku_cpp/src/metrics/Performance.cpp",
                f'msgid "{english}"',
                'msgstr ""',
                "",
            ]
        )
    path.write_text("\n".join(lines), encoding="utf-8")


def main():
    tracked = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "--", *SOURCE_ROOTS], cwd=ROOT
    ).decode().splitlines()
    cpp = [name for name in tracked if Path(name).suffix in {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"}]
    py = [name for name in tracked if name.startswith("hayaku/extensions/visualization/") and name.endswith(".py")]
    with tempfile.TemporaryDirectory(prefix="hayaku-i18n-") as temp:
        work = Path(temp)
        for name, files in (("cpp", cpp), ("py", py)):
            (work / f"{name}-files").write_text("\n".join(files) + "\n", encoding="utf-8")
        run(
            "xgettext", f"--directory={ROOT}", "--language=C++", "--from-code=UTF-8", "--keyword=",
            "--keyword=htr:1", "--keyword=hctr:1c,2",
            f"--files-from={work / 'cpp-files'}", "-o", str(work / "cpp.pot"),
        )
        run(
            "xgettext", f"--directory={ROOT}", "--language=Python", "--from-code=UTF-8", "--keyword=",
            "--keyword=htr:1", f"--files-from={work / 'py-files'}",
            "-o", str(work / "py.pot"),
        )
        dynamic_performance_catalog(work / "performance.pot")
        run(
            "msgcat", "--use-first", str(work / "cpp.pot"), str(work / "py.pot"),
            str(work / "performance.pot"), "-o", str(work / "current.pot"),
        )
        run(
            "msgmerge", "--no-fuzzy-matching", "--quiet", f"--output-file={work / 'merged.po'}",
            str(PO), str(work / "current.pot"),
        )
        run("msgattrib", "--no-obsolete", str(work / "merged.po"), "-o", str(work / "clean.po"))
        run("msgfmt", "--check", "--check-format", "-o", "/dev/null", str(work / "clean.po"))
        PO.write_bytes((work / "clean.po").read_bytes())
    print(f"Refreshed {PO.relative_to(ROOT)}; run i18n/update_translation.sh to compile it.")


if __name__ == "__main__":
    main()

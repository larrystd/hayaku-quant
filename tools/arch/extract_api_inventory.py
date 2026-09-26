#!/usr/bin/env python3
"""Generate a deterministic inventory of Hayaku's high-risk public interfaces.

This is intentionally a lightweight source scanner, not a C++ parser.  Its job is to make
the current public surface reviewable and diffable.  Classification decisions live here so
that later changes to the public surface show up in code review.
"""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parents[2]

HEADERS = (
    "hayaku_cpp/src/application/HayakuSession.h",
    "hayaku_cpp/src/application/SessionOptions.h",
    "hayaku_cpp/src/data/DataEngine.h",
    "hayaku_cpp/src/data/storage/DataDriverFactory.h",
    "hayaku_cpp/src/execution/AccountConfig.h",
    "hayaku_cpp/src/execution/ExecutionEngine.h",
    "hayaku_cpp/src/execution/OrderRequest.h",
    "hayaku_cpp/src/execution/ExecutionReport.h",
    "hayaku_cpp/src/execution/AccountSnapshot.h",
    "hayaku_cpp/src/strategy/StrategyDefinition.h",
    "hayaku_cpp/src/strategy/StrategyEngine.h",
    "hayaku_cpp/src/strategy/BacktestRequest.h",
    "hayaku_cpp/src/strategy/BacktestResult.h",
    "hayaku_cpp/src/strategy/Strategy.h",
    "hayaku_cpp/src/strategy/decision/EnvironmentBase.h",
    "hayaku_cpp/src/strategy/decision/ConditionBase.h",
    "hayaku_cpp/src/strategy/decision/SignalBase.h",
    "hayaku_cpp/src/strategy/risk/MoneyManagerBase.h",
    "hayaku_cpp/src/strategy/risk/StoplossBase.h",
    "hayaku_cpp/src/strategy/risk/ProfitGoalBase.h",
    "hayaku_cpp/src/execution/pricing/SlippageBase.h",
)

BINDING_GLOBS = (
    "hayaku_pywrap/*.cpp",
    "hayaku_pywrap/app/*.cpp",
    "hayaku_pywrap/data/*.cpp",
    "hayaku_pywrap/data/driver/*.cpp",
    "hayaku_pywrap/strategy/*.cpp",
    "hayaku_pywrap/execution/*.cpp",
    "hayaku_pywrap/analysis/*.cpp",
    "hayaku_pywrap/common/*.cpp",
    "hayaku_pywrap/advanced/*.cpp",
)

PYTHON_EXPORT_FILES = (
    "hayaku/__init__.py",
    "hayaku/core.py",
    "hayaku/extend.py",
    "hayaku/indicator/__init__.py",
    "hayaku/data/__init__.py",
    "hayaku/execution/__init__.py",
    "hayaku/strategy/__init__.py",
    "hayaku/analysis/__init__.py",
    "hayaku/common/__init__.py",
    "hayaku/advanced/__init__.py",
)


@dataclass(frozen=True)
class CppApi:
    owner: str
    name: str
    path: str
    line: int
    classification: str
    decision: str


@dataclass(frozen=True)
class PythonBinding:
    name: str
    kind: str
    path: str
    line: int


def normalize_declaration(value: str) -> str:
    value = re.sub(r"//.*", "", value)
    value = re.sub(r"\s+", " ", value)
    return value.strip()


def classify(owner: str, name: str) -> tuple[str, str]:
    if name.startswith("_testing"):
        return "Internal", "move to test fixture"
    if name.startswith("_"):
        return "SPI", "expose only through the extension namespace"

    internal_by_owner = {
        "StockManager": {
            "getLoadTaskGroup",
            "thread_id",
            "isIpcClientMode",
            "joinPreloadThread",
            "releaseShmServerBaseInfoCache",
            "clearPlugin",
            "hasCancelLoad",
        },
        "System": {
            "sellForceOnOpen",
            "sellForceOnClose",
            "clearDelayBuyRequest",
            "haveDelaySellRequest",
            "haveDelayBuyRequest",
            "pfProcessDelaySellRequest",
            "pfProcessDelayBuyRequest",
            "runMoment",
            "runMomentOnOpen",
            "runMomentOnClose",
            "readyForRun",
            "partChangedNotify",
            "isPythonObject",
        },
        "TradeManagerBase": {"isPythonObject", "addTradeRecord", "addPosition"},
    }
    if name in internal_by_owner.get(owner, set()):
        return "Internal", "remove from the ordinary public surface"

    deprecated_by_owner = {
        "StockManager": {
            "init",
            "reload",
            "reloadWith",
            "quit",
            "cancelLoad",
            "setPluginPath",
            "getPluginPath",
            "getPlugin",
        },
        "TradeManagerBase": {
            "regBroker",
            "clearBroker",
            "getBrokerLastDatetime",
            "setBrokerLastDatetime",
            "tocsv",
            "fetchAssetInfoFromBroker",
        },
        "System": {
            "setTM",
            "setMM",
            "setEV",
            "setCN",
            "setSG",
            "setST",
            "setTP",
            "setPG",
            "setSP",
        },
        "Strategy": {"buy", "sell"},
    }
    if name in deprecated_by_owner.get(owner, set()):
        return "Deprecated", "keep a compatibility forwarder during migration"

    if owner in {
        "OrderBrokerBase",
        "DataDriverFactory",
        "EnvironmentBase",
        "ConditionBase",
        "SignalBase",
        "MoneyManagerBase",
        "StoplossBase",
        "ProfitGoalBase",
        "SlippageBase",
    }:
        return "SPI", "move out of the ordinary top-level API"

    return "Public", "retain until a reviewed replacement exists"


def extract_cpp_api(path: Path) -> list[CppApi]:
    rows: list[CppApi] = []
    owner = path.stem
    access = "private"
    pending = ""
    start_line = 0
    brace_depth = 0
    in_target_class = False

    for line_no, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw_line.strip()
        class_match = re.search(r"\bclass\s+(?:HAYAKU_API\s+)?(\w+)", line)
        if class_match and class_match.group(1) == owner:
            in_target_class = True
            brace_depth = line.count("{") - line.count("}")
            continue

        if not in_target_class:
            continue

        direct_member = brace_depth == 1
        next_brace_depth = brace_depth + line.count("{") - line.count("}")
        if next_brace_depth < 0 or (next_brace_depth == 0 and line.startswith("};")):
            break

        if direct_member and line in {"public:", "protected:", "private:"}:
            access = line[:-1]
            pending = ""
            brace_depth = next_brace_depth
            continue
        if access != "public" or (not direct_member and not pending):
            brace_depth = next_brace_depth
            continue
        if not pending and (not line or line.startswith(("//", "/*", "*", "#"))):
            brace_depth = next_brace_depth
            continue

        if not pending:
            start_line = line_no
        pending = f"{pending} {line}".strip()
        normalized = normalize_declaration(pending)

        if "(" not in normalized:
            if ";" in normalized or "{" in normalized:
                pending = ""
            brace_depth = next_brace_depth
            continue
        if not any(token in normalized for token in (";", "{", "= 0")):
            brace_depth = next_brace_depth
            continue

        before_paren = normalized.split("(", 1)[0]
        name_match = re.search(r"(~?\w+|operator\[\])\s*$", before_paren)
        pending = ""
        if not name_match:
            brace_depth = next_brace_depth
            continue
        name = name_match.group(1)
        if name.startswith("operator") or name == f"~{owner}":
            brace_depth = next_brace_depth
            continue
        classification, decision = classify(owner, name)
        rows.append(
            CppApi(owner, name, str(path.relative_to(ROOT)), start_line, classification, decision)
        )
        brace_depth = next_brace_depth

    unique: dict[tuple[str, str, str], CppApi] = {}
    for row in rows:
        unique[(row.owner, row.name, row.path)] = row
    return sorted(unique.values(), key=lambda row: (row.owner, row.name))


def binding_files() -> Iterable[Path]:
    seen: set[Path] = set()
    for pattern in BINDING_GLOBS:
        for path in ROOT.glob(pattern):
            if path not in seen:
                seen.add(path)
                yield path


def extract_bindings(path: Path) -> list[PythonBinding]:
    rows: list[PythonBinding] = []
    pattern = re.compile(
        r"\.(def_static|def_property_readonly|def_property|def_readwrite|def_readonly|def)\s*\(\s*\"([^\"]+)\""
    )
    for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        for match in pattern.finditer(line):
            rows.append(
                PythonBinding(
                    name=match.group(2),
                    kind=match.group(1),
                    path=str(path.relative_to(ROOT)),
                    line=line_no,
                )
            )
    return rows


def python_star_exports() -> list[tuple[str, int, str]]:
    rows: list[tuple[str, int, str]] = []
    for relative in PYTHON_EXPORT_FILES:
        path = ROOT / relative
        for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if re.search(r"^\s*from\s+.+\s+import\s+\*", line):
                rows.append((relative, line_no, line.strip()))
    return rows


def render(cpp_rows: list[CppApi], bindings: list[PythonBinding]) -> str:
    counts: dict[str, int] = {}
    for row in cpp_rows:
        counts[row.classification] = counts.get(row.classification, 0) + 1

    owner_counts: dict[str, int] = {}
    for row in cpp_rows:
        owner_counts[row.owner] = owner_counts.get(row.owner, 0) + 1

    binding_counts: dict[str, int] = {}
    for row in bindings:
        binding_counts[row.path] = binding_counts.get(row.path, 0) + 1

    lines = [
        "# Hayaku API Inventory",
        "",
        "> Generated by `tools/arch/extract_api_inventory.py`. Do not edit generated tables by hand.",
        "",
        "This inventory covers the high-risk orchestration interfaces selected for the first refactoring phase. It is intentionally narrower than every public symbol in the repository.",
        "",
        "## Summary",
        "",
        "| Metric | Count |",
        "| --- | ---: |",
        f"| Reviewed C++ classes | {len(owner_counts)} |",
        f"| Unique C++ public methods | {len(cpp_rows)} |",
        f"| pybind11 exports in reviewed modules | {len(bindings)} |",
        f"| Python files containing star exports | {len(python_star_exports())} |",
        "",
        "### Classification",
        "",
        "| Classification | Count |",
        "| --- | ---: |",
    ]
    for name in ("Public", "SPI", "Internal", "Deprecated"):
        lines.append(f"| {name} | {counts.get(name, 0)} |")

    lines.extend(
        [
            "",
            "### C++ surface by owner",
            "",
            "| Owner | Public methods found |",
            "| --- | ---: |",
        ]
    )
    for owner, count in sorted(owner_counts.items(), key=lambda item: (-item[1], item[0])):
        lines.append(f"| `{owner}` | {count} |")

    lines.extend(
        [
            "",
            "## Reviewed C++ interfaces",
            "",
            "| Owner | Symbol | Classification | Decision | Source |",
            "| --- | --- | --- | --- | --- |",
        ]
    )
    for row in cpp_rows:
        lines.append(
            f"| `{row.owner}` | `{row.name}` | {row.classification} | {row.decision} | `{row.path}:{row.line}` |"
        )

    lines.extend(
        [
            "",
            "## pybind11 export counts",
            "",
            "This table counts binding declarations, including overloads. A high count is a review signal, not proof that an API should be removed.",
            "",
            "| Binding file | Export declarations |",
            "| --- | ---: |",
        ]
    )
    for path, count in sorted(binding_counts.items(), key=lambda item: (-item[1], item[0])):
        lines.append(f"| `{path}` | {count} |")

    lines.extend(
        [
            "",
            "## Python star exports",
            "",
            "| Source | Statement |",
            "| --- | --- |",
        ]
    )
    for path, line_no, statement in python_star_exports():
        lines.append(f"| `{path}:{line_no}` | `{statement}` |")

    lines.extend(
        [
            "",
            "## First decisions",
            "",
            "- Keep `Stock`, `KData`, `KQuery`, `Indicator`, built-in indicator factories and strategy-component factories stable.",
            "- Treat driver registration, plugin loading, IPC/SHM control, preload-thread control and PF/AF moment processing as non-user APIs.",
            "- Keep Python subclass hooks as SPI, but move them away from the ordinary top-level namespace.",
            "- Freeze `TradeManagerBase`: new account, execution, performance or export features must not add more methods to it.",
            "- Preserve old entry points through compatibility forwarders before removal; this inventory does not authorize immediate deletion.",
            "",
            "## Known limitations",
            "",
            "- The scanner is syntax-based and does not replace Clang AST analysis.",
            "- Template and macro-generated methods may be absent.",
            "- Consumer counts require a separate semantic/reference scan.",
            "- Classification is a first-pass architecture decision and must be reviewed before deletion.",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "docs/arch/api-inventory.md",
        help="Markdown output path",
    )
    args = parser.parse_args()

    cpp_rows: list[CppApi] = []
    for relative in HEADERS:
        cpp_rows.extend(extract_cpp_api(ROOT / relative))

    bindings: list[PythonBinding] = []
    for path in binding_files():
        bindings.extend(extract_bindings(path))

    output = args.output if args.output.is_absolute() else ROOT / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(render(cpp_rows, bindings), encoding="utf-8")
    try:
        display_output = output.relative_to(ROOT)
    except ValueError:
        display_output = output
    print(f"wrote {display_output}")
    print(f"C++ methods: {len(cpp_rows)}, pybind exports: {len(bindings)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

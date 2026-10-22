#!/usr/bin/env python3
"""Compare build/RTW_STEAM/report.json with an earlier copy of it.

    python tools/report_diff.py BEFORE.json [UNIT ...]

Prints each function whose match dropped since BEFORE (save a copy of report.json before a change), and each function
of the named units (by the end of their names, such as battle or ui/event_messages) that now matches at all. A function
that was renamed in symbols.txt shows as dropped under its old name, so check those against its new one.
"""

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def load(path):
    report = json.loads(Path(path).read_text(encoding="utf-8"))
    out = {}
    for unit in report["units"]:
        for function in unit.get("functions", []):
            out[(unit["name"], function["name"])] = function.get("fuzzy_match_percent", 0)
    return out


def main():
    before = load(sys.argv[1])
    after = load(ROOT / "build" / "RTW_STEAM" / "report.json")
    for key, value in sorted(before.items()):
        if value > 0 and after.get(key, 0) < value - 0.001:
            print("DROP", key[0], key[1], value, "->", after.get(key))
    for key, value in sorted(after.items()):
        if any(key[0].endswith(unit) for unit in sys.argv[2:]) and value > 0:
            print(f"{value:8.3f} {key[0]} {key[1]}")


if __name__ == "__main__":
    main()

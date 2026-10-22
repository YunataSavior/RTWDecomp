#!/usr/bin/env python3
"""Pair a compiled unit's functions with the original's, in order, and optionally name the original's after them.

    python tools/unit_funcs.py UNIT [--rename]

UNIT is a splits.txt name without its extension (code/romans_cmd_line); its .text range comes from splits.txt. Each
compiled .text function (in object order) is listed beside the original function at the same position, with both
sizes. --rename gives each original function in symbols.txt its compiled name (the $E initializers, constructors and
so on), skipping pairs whose sizes differ. Compiled functions past the original's (unreferenced inline copies) are
listed and ignored.
"""

import re
import sys
from pathlib import Path

from check import read_coff

ROOT = Path(__file__).resolve().parent.parent
CONFIG = ROOT / "config" / "RTW_STEAM"


def unit_range(unit):
    current = None
    for line in (CONFIG / "splits.txt").read_text(encoding="utf-8").splitlines():
        if line and not line[0].isspace():
            current = line.rstrip(":")
        match = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", line)
        if match and current and current.rsplit(".", 1)[0] == unit:
            return int(match[1], 16), int(match[2], 16)
    sys.exit(f"{unit} is not in splits.txt")


def compiled_functions(unit):
    sections, symbols = read_coff(ROOT / "build" / "RTW_STEAM" / "src" / f"{unit}.obj")
    defined = [(s, v, n) for n, v, s, _, function in symbols.values()
               if function and s > 0 and sections[s - 1][0] == ".text"]
    out = []
    for s, v, n in defined:
        ends = [vv for ss, vv, _ in defined if ss == s and vv > v]
        out.append((s, v, (min(ends) if ends else len(sections[s - 1][1])) - v, n))
    return sorted(out)


def main():
    unit, rename = sys.argv[1], "--rename" in sys.argv
    start, end = unit_range(unit)
    path = CONFIG / "symbols.txt"
    with open(path, encoding="utf-8", newline="") as f:
        lines = f.read().split("\n")
    original = []
    for i, line in enumerate(lines):
        match = re.match(r"(\S+) = \.text:0x([0-9A-F]+); // type:function(?: size:0x([0-9A-F]+))?", line)
        if match and start <= int(match[2], 16) < end:
            original.append((int(match[2], 16), i, int(match[3], 16) if match[3] else None, match[1]))
    original.sort()
    compiled = compiled_functions(unit)
    for k in range(max(len(original), len(compiled))):
        mine = compiled[k] if k < len(compiled) else None
        theirs = original[k] if k < len(original) else None
        left = f"{mine[3]} {mine[2]:#x}" if mine else ""
        right = f"{theirs[0]:#x} {theirs[2]:#x} {theirs[3]}" if theirs and theirs[2] else (
            f"{theirs[0]:#x} ? {theirs[3]}" if theirs else "")
        flag = "*" if mine and theirs and theirs[2] is not None and theirs[2] != mine[2] else " "
        print(f"{flag} {left:60} | {right}")
        if rename and mine and theirs and flag == " " and theirs[3] != mine[3]:
            lines[theirs[1]] = lines[theirs[1]].replace(theirs[3] + " = ", mine[3] + " = ", 1)
    if rename:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write("\n".join(lines))


if __name__ == "__main__":
    main()

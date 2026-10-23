#!/usr/bin/env python3
"""List the findings docs' NYD claims with the state of each code address they name.

    python tools/nyd.py [DOC ...] [--todo]

DOC is part of a findings doc's path (public_order); with none, every doc under findings/ is read. Each
line marked NYD (except the "Not Yet Decompiled" notes) is printed with, for each address in it (0x... or an unk_/fn_
name) that falls in .text, the function that holds it in symbols.txt, its size, its splits.txt unit (* when the unit
has no source yet) and its match in build/RTW_STEAM/report.json (- when no unit compiles it). --todo prints only the functions not yet matching at 100%, once each, smallest first, as a work list.
"""

import bisect
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIG = ROOT / "config" / "RTW_STEAM"


def load_functions():
    starts, entries = [], []
    pattern = re.compile(r"^(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)")
    for line in (CONFIG / "symbols.txt").read_text(encoding="utf-8").splitlines():
        match = pattern.match(line)
        if match:
            entries.append((int(match[2], 16), int(match[3], 16), match[1]))
    entries.sort()
    starts = [entry[0] for entry in entries]
    return starts, entries


def load_units():
    units, current = [], None
    for line in (CONFIG / "splits.txt").read_text(encoding="utf-8").splitlines():
        if line and not line[0].isspace():
            current = line.rstrip(":")
        match = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", line)
        if match and current:
            units.append((int(match[1], 16), int(match[2], 16), current))
    return sorted(units)


def unit_of(address, units):
    for start, end, name in units:
        if start <= address < end:
            source = ROOT / "src" / name
            return name if source.exists() else name + "*"
    return "?"


def load_matches():
    path = ROOT / "build" / "RTW_STEAM" / "report.json"
    if not path.exists():
        return {}
    report = json.loads(path.read_text(encoding="utf-8"))
    out = {}
    for unit in report["units"]:
        for function in unit.get("functions", []):
            name = function["name"]
            out[name] = max(out.get(name, 0), function.get("fuzzy_match_percent", 0))
    return out


def lookup(address, starts, entries):
    i = bisect.bisect_right(starts, address) - 1
    if i >= 0 and address < entries[i][0] + max(entries[i][1], 1):
        return entries[i]
    return None


def main():
    args = [arg for arg in sys.argv[1:] if not arg.startswith("--")]
    todo = "--todo" in sys.argv
    docs = sorted((ROOT / "findings").rglob("*.md"))
    if args:
        docs = [doc for doc in docs if any(arg in doc.relative_to(ROOT).as_posix() for arg in args)]
    starts, entries = load_functions()
    matches = load_matches()
    units = load_units()
    seen = {}
    for doc in docs:
        lines = doc.read_text(encoding="utf-8").splitlines()
        for number, line in enumerate(lines, 1):
            if "NYD" not in line or "Not Yet Decompiled" in line:
                continue
            states = []
            for text in re.findall(r"(?:0x|unk_|fn_)([0-9A-Fa-f]{6,8})\b", line):
                entry = lookup(int(text, 16), starts, entries)
                if not entry:
                    continue
                start, size, name = entry
                match = matches.get(name)
                state = "-" if match is None else f"{match:.0f}%"
                unit = unit_of(start, units)
                states.append(f"0x{start:X} {name} size:0x{size:X} {unit} {state}")
                if match is None or match < 100:
                    seen.setdefault(start, (size, name, f"{unit} {state}", f"{doc.stem}:{number}"))
            if not todo:
                print(f"{doc.relative_to(ROOT).as_posix()}:{number}: {line.strip()[:160]}")
                for state in states:
                    print(f"    {state}")
    if todo:
        for start, (size, name, state, where) in sorted(seen.items(), key=lambda item: item[1][0]):
            print(f"0x{start:X} size:0x{size:X} {name} {state}  ({where})")


if __name__ == "__main__":
    main()

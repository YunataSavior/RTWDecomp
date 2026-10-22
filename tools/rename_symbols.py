#!/usr/bin/env python3
"""Name the original's functions after a partly decompiled unit's.

    python tools/rename_symbols.py UNIT
    python tools/rename_symbols.py UNIT ADDR=SUBSTRING ...

UNIT is a splits.txt name without its extension (code/battle). The first form lists the compiled unit's .text
functions that symbols.txt doesn't name, with their sizes. The second gives the original function at each ADDR (hex)
the name of the one unnamed compiled function whose name contains SUBSTRING (set_speed@Battle), and prints both
sizes; a size that differs is marked with a *. (tools/unit_funcs.py --rename does this by position for a whole unit.)
"""

import re
import sys
from pathlib import Path

from unit_funcs import compiled_functions

ROOT = Path(__file__).resolve().parent.parent
PATH = ROOT / "config" / "RTW_STEAM" / "symbols.txt"


def main():
    unit, pairs = sys.argv[1], sys.argv[2:]
    with open(PATH, encoding="utf-8", newline="") as f:
        lines = f.read().split("\n")
    named = {m[1] for line in lines if (m := re.match(r"(\S+) = ", line))}
    unnamed = [(n, size) for _, _, size, n in compiled_functions(unit) if n not in named]
    if not pairs:
        for name, size in unnamed:
            print(f"{size:#7x} {name}")
        return
    for pair in pairs:
        address, part = pair.split("=", 1)
        found = [(n, size) for n, size in unnamed if part in n]
        if len(found) != 1:
            sys.exit(f"{part}: {len(found)} unnamed compiled functions match")
        name, size = found[0]
        pattern = re.compile(rf"(\S+) = \.text:0x{int(address, 16):08X};(.*?size:0x([0-9A-F]+))?")
        for i, line in enumerate(lines):
            if (match := pattern.match(line)):
                theirs = int(match[3], 16) if match[3] else None
                lines[i] = line.replace(match[1] + " = ", name + " = ", 1)
                flag = "*" if theirs is not None and theirs != size else " "
                print(f"{flag} {address} {match[1]} -> {name} ({size:#x}, original {theirs and hex(theirs)})")
                break
        else:
            sys.exit(f"0x{address}: no .text symbol there")
    with open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write("\n".join(lines))


if __name__ == "__main__":
    main()

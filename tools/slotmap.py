#!/usr/bin/env python3
"""Map a function's stack slots in the original to the compiled unit's.

    python tools/slotmap.py UNIT SYMBOL

Zips the split original (build/VERSION/obj/UNIT.obj) and the compiled unit (build/VERSION/src/UNIT.obj) instruction by
instruction, with relocated fields ignored, and prints each original [ebp-X] slot that the compiled code puts
elsewhere. It reports instructions whose shape differs, and stops at the first whose mnemonic or size does. With
tools/vc6_names.py, it shows which locals to rename.
"""

import re
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from check import Object

ROOT = Path(__file__).resolve().parent.parent
SLOT = r"ebp (?:\+ \w+ )?- (0x[0-9a-f]+|[0-9]+)\]"


def main():
    unit, symbol = sys.argv[1:3]
    build = ROOT / "build" / "RTW_STEAM"
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    want = list(md.disasm(Object(build / "obj" / f"{unit}.obj").functions[symbol][0], 0))
    got = list(md.disasm(Object(build / "src" / f"{unit}.obj").functions[symbol][0], 0))
    shape = lambda i: re.sub(SLOT, "S]", re.sub(r"0x[0-9a-f]{3,}", "N", f"{i.mnemonic} {i.op_str}"))
    pairs = {}
    for a, b in zip(want, got):
        if shape(a) != shape(b):
            print(f"shape differs at +{a.address:#x}: {a.mnemonic} {a.op_str} | {b.mnemonic} {b.op_str}")
            if a.mnemonic != b.mnemonic or a.size != b.size:
                break
        for x, y in zip(re.findall(SLOT, a.op_str), re.findall(SLOT, b.op_str)):
            pairs.setdefault(int(x, 0), set()).add(int(y, 0))
    moved = 0
    for x in sorted(pairs):
        if pairs[x] != {x}:
            moved += 1
            print(f"  -{x:#x} -> " + ", ".join(f"-{y:#x}" for y in sorted(pairs[x])))
    print(f"{moved} of {len(pairs)} slots differ")


if __name__ == "__main__":
    main()

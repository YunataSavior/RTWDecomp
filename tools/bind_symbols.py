#!/usr/bin/env python3
"""Name the original's unnamed relocation targets after the compiled unit's.

    python tools/bind_symbols.py UNIT [--dry]

Gives the original's _lbl_/_fn_ relocation targets in symbols.txt the compiled unit's names, pairing the relocations
of functions whose size and relocation offsets agree. Each name goes at the lowest address it pairs with (the split
gives the others as offsets); data takes its size from a same-named symbol when there is one, and is scope:local
when the compiled symbol is static. --dry prints the changes without writing them. Check what it prints: a name it
gives an address next to the real one (an `array - 2` addend) is wrong and should be removed.
"""

import re
import sys
from pathlib import Path

from check import Object, target_matcher

ROOT = Path(__file__).resolve().parent.parent
CONFIG = ROOT / "config" / "RTW_STEAM"


def main():
    unit = sys.argv[1]
    want = Object(ROOT / "build" / "RTW_STEAM" / "obj" / f"{unit}.obj")
    got = Object(ROOT / "build" / "RTW_STEAM" / "src" / f"{unit}.obj")
    same = target_matcher(CONFIG, got)
    storage = {n: (st, s, v, f) for n, v, s, st, f in got.symbols.values()}

    def size_of(name):
        st, s, v, f = storage[name]
        if s <= 0:
            return 4
        following = [vv for nn, vv, ss, _, _ in got.symbols.values() if ss == s and vv > v]
        return min(following, default=len(got.sections[s - 1][1])) - v

    binds = {}
    for name, (code, relocs) in want.functions.items():
        if name not in got.functions:
            continue
        other, other_relocs = got.functions[name]
        a = {(o, k): t for o, t, k in relocs}
        b = {(o, k): t for o, t, k in other_relocs}
        if len(other) != len(code) or a.keys() != b.keys():
            continue
        for k in a:
            wanted, compiled = a[k], b[k]
            if same(wanted, compiled):
                continue
            match = re.fullmatch(r"_(lbl|fn)_([0-9A-F]{8})", wanted)
            if not match:
                print("named mismatch", name, hex(k[0]), wanted, compiled)
                continue
            address = int(match[2], 16)
            if compiled not in binds or address < binds[compiled][0]:
                binds[compiled] = (address, match[1])

    path = CONFIG / "symbols.txt"
    raw = path.read_bytes().decode()
    newline = "\r\n" if "\r\n" in raw else "\n"
    lines = raw.split(newline)
    if lines[-1] == "":
        lines.pop()
    entry_pattern = re.compile(r"(\S+) = ([^:\s]+):0x([0-9A-Fa-f]+);(.*)")
    known_size = {}
    for line in lines:
        match = re.match(r"(\S+) = [^:\s]+:0x[0-9A-Fa-f]+;.*size:0x([0-9A-Fa-f]+)", line)
        if match:
            known_size.setdefault(match[1], int(match[2], 16))

    def index():
        return {int(entry_pattern.match(l)[3], 16): i for i, l in enumerate(lines) if entry_pattern.match(l)}

    for name, (address, kind) in sorted(binds.items(), key=lambda x: x[1][0]):
        st, s, v, f = storage[name]
        at = index()
        if kind == "fn":
            section, extra = ".text", ""
        else:
            section_name = got.sections[s - 1][0] if s > 0 else ".bss"
            section = next((x for x in (".bss", ".rdata", ".data") if section_name.startswith(x)), ".bss")
            size = 1 if name.startswith("_$S") else known_size.get(name, size_of(name))
            extra = f" // type:object size:0x{size:X}" + (" scope:local" if st == 3 else "")
        if address in at:
            old = lines[at[address]]
            match = entry_pattern.match(old)
            lines[at[address]] = name + old[len(match[1]):]
            print("~", match[1], "->", name)
        else:
            entry = f"{name} = {section}:0x{address:08X};{extra}"
            matches = [(i, entry_pattern.match(l)) for i, l in enumerate(lines)]
            matches = [(i, m) for i, m in matches if m and m[2] == section]
            later = [i for i, m in matches if int(m[3], 16) > address]
            lines.insert(later[0] if later else matches[-1][0] + 1, entry)
            print("+", entry)
    if "--dry" not in sys.argv:
        path.write_bytes((newline.join(lines) + newline).encode())


if __name__ == "__main__":
    main()

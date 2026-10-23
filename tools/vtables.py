#!/usr/bin/env python3
"""List the original's vtables that hold functions from an address range, with each vtable's entries.

    python tools/vtables.py START END [--full]

START and END are hex (END exclusive), such as a unit's .text range from splits.txt. A vtable is taken to be a run of
dwords in a data section that all point at symbols.txt functions, split where code refers to one of them (where a
vtable starts); each one holding a function from START to END is printed with its address, the functions whose code
refers to it (the constructors and destructors that set it), and its entries in the range by offset and name. --full
prints all its entries, with * marking those in the range.
"""

import bisect
import re
import struct
import sys
from pathlib import Path

from pe import PEFile

ROOT = Path(__file__).resolve().parent.parent
VERSION = "RTW_STEAM"


def load_functions():
    entries = []
    pattern = re.compile(r"^(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)")
    for line in (ROOT / "config" / VERSION / "symbols.txt").read_text(encoding="utf-8").splitlines():
        match = pattern.match(line)
        if match:
            entries.append((int(match[2], 16), int(match[3], 16), match[1]))
    entries.sort()
    return entries


def main():
    args = [arg for arg in sys.argv[1:] if arg != "--full"]
    full = "--full" in sys.argv
    if len(args) != 2:
        sys.exit(__doc__)
    start, end = int(args[0], 16), int(args[1], 16)
    pe = PEFile(ROOT / "orig" / VERSION / "RomeTW.exe")
    functions = load_functions()
    starts = [entry[0] for entry in functions]
    names = {entry[0]: entry[2] for entry in functions}
    text = next(section for section in pe.sections if section.name == ".text")
    text_end = text.virtual_address + text.virtual_size

    def owner(address):
        i = bisect.bisect_right(starts, address) - 1
        return functions[i][2] if i >= 0 and address < functions[i][0] + functions[i][1] else hex(address)

    vtables = []
    for section in pe.sections:
        if section.characteristics & 0x20000000 or section.file_size == 0:     # executable
            continue
        data = pe.data[section.file_offset:section.file_offset + section.file_size]
        run = []
        for offset in range(0, len(data) - 3, 4):
            value = struct.unpack_from("<I", data, offset)[0]
            if text.virtual_address <= value < text_end and value in names:
                run.append((section.virtual_address + offset, value))
                continue
            if any(start <= target < end for _, target in run):
                vtables.append(run)
            run = []
    code = pe.data[text.file_offset:text.file_offset + text.file_size]

    def users_of(address):
        needle = re.escape(struct.pack("<I", address))
        return sorted({owner(text.virtual_address + match.start()) for match in re.finditer(needle, code)})

    for run in vtables:
        users = [users_of(address) for address, _ in run]
        cuts = [i for i, found in enumerate(users) if found and i > 0]
        for first, last in zip([0] + cuts, cuts + [len(run)]):
            table = run[first:last]
            if not any(start <= target < end for _, target in table):
                continue
            print(f"vtable 0x{table[0][0]:X} ({len(table)} entries), referred to by "
                  f"{', '.join(users[first]) or 'nothing'}")
            for slot, (_, target) in enumerate(table):
                inside = start <= target < end
                if full or inside:
                    print(f"  +0x{slot * 4:02X} {'*' if inside and full else ''}0x{target:X} {names.get(target, '?')}")


if __name__ == "__main__":
    main()

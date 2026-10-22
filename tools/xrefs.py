#!/usr/bin/env python3
"""List the instructions in the original executable that refer to an address, or call or jump to it.

    python tools/xrefs.py ADDR [ADDR ...] [--range N]

ADDR is hex. Each hit shows the function it's in (symbols.txt's name and its unit from splits.txt) and the
instruction. --range N (hex) also counts references to ADDR..ADDR+N-1, for a struct or an array.
"""

import argparse
import bisect
import re
import struct
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from pe import PEFile

ROOT = Path(__file__).resolve().parent.parent
VERSION = "RTW_STEAM"


def functions():
    found = []
    for line in (ROOT / "config" / VERSION / "symbols.txt").read_text(encoding="utf-8").splitlines():
        match = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)", line)
        if match and int(match[3], 16) > 0:
            found.append((int(match[2], 16), int(match[3], 16), match[1]))
    found.sort()
    return found


def units():
    found = []
    unit = None
    for line in (ROOT / "config" / VERSION / "splits.txt").read_text(encoding="utf-8").splitlines():
        if line and not line[0].isspace() and line.rstrip().endswith(":"):
            unit = line.rstrip()[:-1]
        match = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", line)
        if match and unit:
            found.append((int(match[1], 16), int(match[2], 16), unit))
    found.sort()
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("addresses", nargs="+")
    parser.add_argument("--range", default="1")
    args = parser.parse_args()
    span = int(args.range, 16)
    pe = PEFile(ROOT / "orig" / VERSION / "RomeTW.exe")
    funcs = functions()
    starts = [f[0] for f in funcs]
    unit_table = units()
    unit_starts = [u[0] for u in unit_table]
    cs = Cs(CS_ARCH_X86, CS_MODE_32)
    text = [s for s in pe.sections if s.characteristics & 0x20000000]

    for target in (int(a, 16) for a in args.addresses):
        hits = set()
        for section in text:
            data = pe.data[section.file_offset:section.file_offset + section.file_size]
            for value in range(target, target + span):
                needle = struct.pack("<I", value)
                start = 0
                while (offset := data.find(needle, start)) >= 0:
                    hits.add(section.virtual_address + offset)
                    start = offset + 1
            # Calls and jumps: E8/E9 and a rel32 to the target.
            base = section.virtual_address
            for opcode in (0xE8, 0xE9):
                start = 0
                while (offset := data.find(bytes([opcode]), start)) >= 0:
                    start = offset + 1
                    if offset + 5 <= len(data):
                        rel = struct.unpack_from("<i", data, offset + 1)[0]
                        if target <= base + offset + 5 + rel < target + span:
                            hits.add(base + offset + 1)
        print(f"0x{target:08X}: {len(hits)} hits")
        seen = set()
        for hit in sorted(hits):
            i = bisect.bisect_right(starts, hit) - 1
            name, insn = "?", ""
            if i >= 0 and hit < funcs[i][0] + funcs[i][1]:
                start, size, name = funcs[i]
                for ins in cs.disasm(pe.read(start, size), start):
                    if ins.address <= hit < ins.address + ins.size:
                        insn = f"{ins.address:08X} {ins.mnemonic} {ins.op_str}"
                        break
                if not insn:
                    continue
            if insn in seen:
                continue
            seen.add(insn)
            j = bisect.bisect_right(unit_starts, hit) - 1
            unit = unit_table[j][2] if j >= 0 and hit < unit_table[j][1] else "?"
            print(f"  {name:28} {unit:40} {insn}")


if __name__ == "__main__":
    main()

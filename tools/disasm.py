#!/usr/bin/env python3
"""Disassemble the original executable at an address.

    python tools/disasm.py ADDR LEN [--annotate]

ADDR and LEN are hex. --annotate adds, after each instruction, the symbols.txt name of a call target or address
operand, or the string at an address operand.
"""

import argparse
import re
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from pe import PEFile

ROOT = Path(__file__).resolve().parent.parent
VERSION = "RTW_STEAM"


def symbol_names():
    names = {}
    for line in (ROOT / "config" / VERSION / "symbols.txt").read_text(encoding="utf-8").splitlines():
        match = re.match(r"(\S+) = [^:\s]+:0x([0-9A-Fa-f]+);", line)
        if match:
            names.setdefault(int(match[2], 16), match[1])
    return names


def string_at(pe, address):
    try:
        data = pe.read(address, 64)
    except ValueError:
        return None
    text = data.split(b"\0")[0]
    if text and all(32 <= c < 127 or c in (9, 10) for c in text):
        return repr(text.decode())
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("address", type=lambda s: int(s, 16))
    parser.add_argument("length", type=lambda s: int(s, 16))
    parser.add_argument("--annotate", action="store_true")
    args = parser.parse_args()
    pe = PEFile(ROOT / "orig" / VERSION / "RomeTW.exe")
    names = symbol_names() if args.annotate else {}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for ins in md.disasm(pe.read(args.address, args.length), args.address):
        line = f"{ins.address:#x} {ins.mnemonic} {ins.op_str}"
        if args.annotate:
            notes = []
            for value in re.findall(r"0x[0-9a-f]{6,}", ins.op_str):
                value = int(value, 16)
                if value in names and not names[value].startswith(("_lbl_", "_fn_")):
                    notes.append(names[value])
                elif ins.mnemonic != "call":
                    text = string_at(pe, value)
                    if text:
                        notes.append(text)
            if notes:
                line += "   ; " + ", ".join(notes)
        print(line)


if __name__ == "__main__":
    main()

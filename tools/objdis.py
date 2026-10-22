#!/usr/bin/env python3
"""Disassemble a function of a compiled object, with its relocations' targets named.

    python tools/objdis.py OBJ SYMBOL
    python tools/objdis.py OBJ --list

OBJ is a COFF object (build/RTW_STEAM/src/code/romans.obj). SYMBOL is the function's name as the object gives it, or
a part of it that only one function's name contains. --list prints the object's functions and their sizes. Useful for
a function the original doesn't have yet, which tools/odiff.py can't show.
"""

import argparse
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from check import Object


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("obj", type=Path)
    parser.add_argument("symbol", nargs="?")
    parser.add_argument("--list", action="store_true", help="list the functions")
    args = parser.parse_args()
    obj = Object(args.obj)
    if args.list or not args.symbol:
        for name, (code, _) in obj.functions.items():
            print(f"{len(code):6X}  {name}")
        return
    names = [n for n in obj.functions if n == args.symbol] or [n for n in obj.functions if args.symbol in n]
    if len(names) != 1:
        sys.exit(f"{args.symbol} names {len(names)} functions: {' '.join(names[:10])}")
    code, relocs = obj.functions[names[0]]
    targets = {offset: target for offset, target, _ in relocs}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    print(names[0])
    for insn in md.disasm(code, 0):
        named = [targets[o] for o in range(insn.address, insn.address + insn.size) if o in targets]
        note = f"   ; {named[0]}" if named else ""
        print(f"{insn.address:5X}  {insn.mnemonic} {insn.op_str}{note}")


if __name__ == "__main__":
    main()

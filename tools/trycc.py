#!/usr/bin/env python3
"""Test-compile a scratch source with the game's flags, and compare its functions with the original's.

    python tools/trycc.py SOURCE [NAME[=ADDR] ...] [--flags "/MT /O2 ..."] [--all]

SOURCE is compiled with Visual C++ 6.0 (build.ninja's vc6) and config.json's cflags (or --flags) to build/trycc/.
Each NAME (a part of one compiled function's name) is printed beside the original's function at ADDR (hex; its
symbols.txt size, or the compiled size), with the rows that differ marked: relocated operands and addresses compare as
equal, and branch targets compare by their offset in the function. A NAME without ADDR is only disassembled. With no
NAME, the object's functions are listed. --all prints every row rather than only the differing ones and their
neighbours. The command line is passed to CL.EXE as a list, so a Unix shell can't rewrite its /flags as paths.
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from check import Object
from pe import PEFile

ROOT = Path(__file__).resolve().parent.parent
VERSION = "RTW_STEAM"
ADDRESS = re.compile(r"0x[0-9a-f]{5,}")
BRANCH = re.compile(r"^(j\w+|call|loop\w*)$")


def vc6_dir():
    for line in (ROOT / "build.ninja").read_text(encoding="utf-8").splitlines():
        if line.startswith("vc6 = "):
            return line[6:].strip()
    sys.exit("build.ninja has no vc6; run configure.py first")


def sizes():
    found = {}
    pattern = re.compile(r"^\S+ = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)")
    for line in (ROOT / "config" / VERSION / "symbols.txt").read_text(encoding="utf-8").splitlines():
        match = pattern.match(line)
        if match:
            found[int(match[1], 16)] = int(match[2], 16)
    return found


def rows(md, code, base, relocated=()):
    """(text, key) per instruction: KEY ignores what relocation or linking changes."""
    out = []
    for insn in md.disasm(code, base):
        text = f"{insn.mnemonic} {insn.op_str}".strip()
        if BRANCH.match(insn.mnemonic) and re.fullmatch(r"0x[0-9a-f]+", insn.op_str):
            target = int(insn.op_str, 16)
            key = f"{insn.mnemonic} +{target - base:X}" if insn.mnemonic != "call" else "call <target>"
            if insn.mnemonic != "call":
                text = f"{insn.mnemonic} +{target - base:X}"
        elif any(insn.address <= offset < insn.address + insn.size for offset in relocated):
            # The relocated value is the number that isn't a register's displacement.
            key = re.sub(r"(?<![+-] )\b(0x[0-9a-f]+|\d+)\b(?!\*)", "<addr>", text)
            key = re.sub(r"\[(\w+(?:\*\d)?)\]", r"[\1 + <addr>]", key) if "<addr>" not in key else key
        else:
            key = ADDRESS.sub("<addr>", text)
        out.append((text, key))
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("source", type=Path)
    parser.add_argument("names", nargs="*")
    parser.add_argument("--flags")
    parser.add_argument("--all", action="store_true")
    args = parser.parse_args()
    config = json.loads((ROOT / "config" / VERSION / "config.json").read_text(encoding="utf-8"))
    flags = args.flags.split() if args.flags else config["cflags"]
    obj_path = ROOT / "build" / "trycc" / args.source.with_suffix(".obj").name
    obj_path.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([sys.executable, str(ROOT / "tools" / "cl.py"), vc6_dir(), str(obj_path),
                             str(args.source), *flags], capture_output=True, text=True)
    messages = [line for line in (result.stdout + result.stderr).splitlines()
                if line.strip() and line.strip() != args.source.name]
    if messages:
        print("\n".join(messages))
    if result.returncode:
        sys.exit(result.returncode)
    obj = Object(obj_path)
    if not args.names:
        for name, (code, _) in obj.functions.items():
            print(f"{len(code):6X}  {name}")
        return
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    pe = PEFile(ROOT / "orig" / VERSION / "RomeTW.exe")
    known = sizes()
    for spec in args.names:
        name, _, address = spec.partition("=")
        matches = [n for n in obj.functions if n == name] or [n for n in obj.functions if name in n]
        if len(matches) != 1:
            print(f"{name} names {len(matches)} functions: {' '.join(matches[:10])}")
            continue
        code, relocs = obj.functions[matches[0]]
        targets = {offset: target for offset, target, _ in relocs}
        mine = rows(md, code, 0, targets)
        if not address:
            print(f"; {matches[0]} (0x{len(code):X} bytes)")
            for text, _ in mine:
                print(f"  {text}")
            continue
        start = int(address, 16)
        theirs = rows(md, pe.read(start, known.get(start, len(code))), start)
        same = sum(a[1] == b[1] for a, b in zip(mine, theirs))
        total = max(len(mine), len(theirs))
        print(f"; {matches[0]} (0x{len(code):X} bytes) vs 0x{start:X} (0x{known.get(start, len(code)):X} bytes): "
              f"{same}/{total} rows the same")
        if same == total and len(code) == known.get(start, len(code)):
            continue
        shown = set()
        for i in range(total):
            a = mine[i] if i < len(mine) else ("", "")
            b = theirs[i] if i < len(theirs) else ("", "")
            if args.all or a[1] != b[1]:
                shown.update(range(i - 1, i + 2))
        for i in range(total):
            if i not in shown:
                continue
            a = mine[i] if i < len(mine) else ("", "")
            b = theirs[i] if i < len(theirs) else ("", "")
            print(f"{'  ' if a[1] == b[1] else '! '}{i:3} {a[0]:<40} {b[0]}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""python tools/check.py CONFIG TARGET BASE STAMP: check that BASE (a compiled unit) matches TARGET (its split from
the original), then touch STAMP. CONFIG is the version's config directory (config/RTW_STEAM).

Each function of TARGET must be in BASE by the same name, with the same bytes (relocated fields aside) and the same
relocations: offset, type and target symbol. A relocation's addend is the field's value, so it is compared with the
bytes. Two targets with different names still match when:

- the compiled one is a string literal or another static data symbol of BASE whose bytes are the original's at the
  target's address (literals are named $SG<n> by the compiler, and file statics need no name of their own);
- CONFIG/folded.txt lists the compiled one as folded into the original one: the linker kept one copy of functions
  whose code is the same (a template's functions for different types, most often);
- both name an address (an _lbl_ name, or one symbols.txt gives) and, with their addends, the same one: the split
  names the symbol an address falls in, so `array - 2` can be the symbol before `array`.

A string literal of zeros ("") is in .bss, so it matches an original address that has no file-backed data.
"""
import json
import re
import struct
import sys
from pathlib import Path

from pe import PEFile


def read_coff(path):
    """([(name, data, [(offset, symbol index, type)])], {index: (name, value, section, storage, function)}) of an
    i386 COFF object."""
    data = Path(path).read_bytes()
    _, count, _, symbol_offset, symbol_count, optional, _ = struct.unpack_from("<HHIIIHH", data)
    strings = symbol_offset + 18 * symbol_count
    symbols, i = {}, 0
    while i < symbol_count:
        offset = symbol_offset + 18 * i
        field = data[offset:offset + 8]
        if field[:4] == b"\0\0\0\0":
            start = strings + struct.unpack_from("<I", field, 4)[0]
            name = data[start:data.index(b"\0", start)].decode()
        else:
            name = field.rstrip(b"\0").decode()
        value, section, kind, storage, aux = struct.unpack_from("<IhHBB", data, offset + 8)
        symbols[i] = (name, value, section, storage, kind == 0x20)
        i += 1 + aux
    sections = []
    for s in range(count):
        name, _, _, size, raw, relocations, _, relocation_count, _, _ = struct.unpack_from(
            "<8sIIIIIIHHI", data, 20 + optional + 40 * s)
        relocs = [struct.unpack_from("<IIH", data, relocations + 10 * r) for r in range(relocation_count)]
        sections.append((name.rstrip(b"\0").decode(), data[raw:raw + size] if raw else bytes(size), relocs))
    return sections, symbols


class Object:
    """An object's functions ({name: (code, [(offset, target, type)])}) and static data ({name: bytes})."""

    def __init__(self, path):
        self.sections, self.symbols = read_coff(path)
        defined = [s for s in self.symbols.values() if s[3] in (2, 3) and s[2] > 0]
        self.functions = {}
        for name, value, section, _, function in defined:
            if not function:
                continue
            _, data, relocs = self.sections[section - 1]
            following = [v for _, v, s, _, f in defined if f and s == section and v > value]
            end = min(following, default=len(data))
            self.functions[name] = (data[value:end], [(o - value, self.symbols[i][0], kind)
                                                      for o, i, kind in relocs if value <= o < end])
        self.data = {}
        for name, value, section, storage, function in defined:
            section_name, data, _ = self.sections[section - 1]
            if storage == 3 and not function and section_name.startswith((".data", ".bss")) and name != section_name:
                following = [v for _, v, s, _, _ in defined if s == section and v > value]
                self.data[name] = data[value:min(following, default=len(data))]


def target_matcher(config, got):
    """A function telling whether a relocation of the compiled object `got` to `compiled` matches one of the original
    to `wanted` (see above)."""
    config = Path(config)
    settings = json.loads((config / "config.json").read_text())
    addresses = {}
    for line in (config / "symbols.txt").read_text().splitlines():
        match = re.match(r"(\S+) = [^:\s]+:0x([0-9A-Fa-f]+);", line)
        if match:
            addresses[match[1]] = int(match[2], 16)
    folded = {}
    if (config / "folded.txt").exists():
        for line in (config / "folded.txt").read_text().splitlines():
            names = line.split("#")[0].split()
            for name in names[1:]:
                folded[name] = names[0]
    original = PEFile(settings["original"])

    def same_target(wanted, compiled):
        if wanted == compiled or folded.get(compiled) == wanted:
            return True
        if compiled not in got.data:
            return False
        match = re.fullmatch(r"_lbl_([0-9A-F]{8})", wanted)
        address = int(match[1], 16) if match else addresses.get(wanted)
        if address is None:
            return False
        content = got.data[compiled].rstrip(b"\0") + b"\0"
        try:
            return original.read(address, len(content)) == content
        except ValueError:
            if content.strip(b"\0"):
                return False
            try:
                original.section_for_address(address)
            except ValueError:
                return False
            return True

    def address_of(name):
        match = re.fullmatch(r"_lbl_([0-9A-F]{8})", name)
        return int(match[1], 16) if match else addresses.get(name)

    same_target.address_of = address_of
    return same_target


def main():
    config, target, base, stamp = map(Path, sys.argv[1:])
    want, got = Object(target), Object(base)
    same_target = target_matcher(config, got)

    problems = []
    for name, (code, relocs) in want.functions.items():
        if name not in got.functions:
            problems.append(f"{name}: not in the compiled object")
            continue
        other, other_relocs = got.functions[name]
        mine = {(o, k): t for o, t, k in relocs}
        theirs = {(o, k): t for o, t, k in other_relocs}
        if len(other) != len(code):
            problems.append(f"{name}: 0x{len(other):X} bytes, not 0x{len(code):X}")
            continue
        masked = []

        def same(k):
            if same_target(mine[k], theirs[k]):
                return True
            wanted, compiled = (same_target.address_of(n) for n in (mine[k], theirs[k]))
            offset, kind = k
            if kind != 6 or wanted is None or compiled is None:  # IMAGE_REL_I386_DIR32
                return False
            if wanted + struct.unpack_from("<i", code, offset)[0] != compiled + struct.unpack_from("<i", other, offset)[0]:
                return False
            masked.append(offset)
            return True

        if mine.keys() != theirs.keys() or not all(same(k) for k in mine):
            differ = min(k for k in mine.keys() | theirs.keys() if k not in mine or k not in theirs or not same(k))
            problems.append(f"{name}: relocations differ (first at +0x{differ[0]:X})")
            continue
        for offset in masked:
            code = code[:offset] + other[offset:offset + 4] + code[offset + 4:]
        if other != code:
            at = next(i for i in range(len(code)) if code[i] != other[i])
            problems.append(f"{name}: bytes differ at +0x{at:X}")
    if problems:
        print(f"{base} does not match {target}:", *problems, sep="\n  ")
        sys.exit(1)
    stamp.parent.mkdir(parents=True, exist_ok=True)
    stamp.touch()


if __name__ == "__main__":
    main()

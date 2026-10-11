#!/usr/bin/env python3
"""python tools/locate_units.py VERSION [--write]: locate the translation units the original names, from the source
paths its asserts and logging push (`push offset "E:\\Perforce\\...\\code\\army.cpp"`).

A function is a `push ebp; mov ebp, esp` prologue up to the next one (the game keeps frame pointers). A function that
pushes the path of a .cpp or .c belongs to that unit, and a unit's .text is the range from its first such function to
the end of its last (VC6 links each object's code contiguously; functions before the first or after the last assert
are left to be added by hand). Units whose ranges overlap are reported and not written.

Prints the units found and those named but not located. With --write, adds the located units that overlap no unit
already in config/VERSION/splits.txt to it, and a symbol for each of their functions not in symbols.txt.
"""
import argparse
import re
import struct
import sys
from bisect import bisect_right
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from split import original, read_splits, read_symbols

PROLOGUE = b"\x55\x8b\xec"
PATH = re.compile(rb"[\x20-\x7e]{6,260}?\.(?:cpp|c|h|hpp|inl)\x00", re.I)


def unit_name(path):
    """The unit a source path is (the project's path under src/, lowercase), or None for a path outside the
    workspace."""
    path = path.replace("/", "\\").lower()
    if (i := path.find("\\barbi_daily\\code\\")) >= 0:
        return "code/" + path[i + len("\\barbi_daily\\code\\"):].replace("\\", "/")
    if (i := path.find("\\libs_daily\\")) >= 0:
        return "libs_daily/" + path[i + len("\\libs_daily\\"):].replace("\\", "/")
    return None


def locate(version):
    _, pe = original(version)
    text = next(s for s in pe.sections if s.name == ".text")
    code = pe.data[text.file_offset:text.file_offset + text.virtual_size]
    starts = [text.virtual_address + m.start() for m in re.finditer(re.escape(PROLOGUE), code)]
    end_of_text = text.virtual_address + text.virtual_size

    paths = {}
    for section in pe.sections:
        if section.name in (".rdata", ".data"):
            data = pe.data[section.file_offset:section.file_offset + section.file_size]
            for match in PATH.finditer(data):
                path = match.group()[:-1].decode()
                if "\\" in path or "/" in path:
                    paths[section.virtual_address + match.start()] = path

    named, functions = set(), defaultdict(set)
    for address, path in paths.items():
        unit = unit_name(path)
        if unit is None or not unit.endswith((".cpp", ".c")):
            continue
        named.add(unit)
        needle = b"\x68" + struct.pack("<I", address)
        at = 0
        while (i := code.find(needle, at)) >= 0:
            at = i + 1
            j = bisect_right(starts, text.virtual_address + i) - 1
            if j >= 0:
                functions[unit].add(j)

    units = {}
    for unit, indices in functions.items():
        first, last = min(indices), max(indices)
        end = starts[last + 1] if last + 1 < len(starts) else end_of_text
        units[unit] = (starts[first], end, [(starts[k], (starts[k + 1] if k + 1 < len(starts) else end_of_text))
                                             for k in range(first, last + 1)])
    ordered = sorted(units.items(), key=lambda item: item[1][0])
    overlapping = set()
    for (a, (_, a_end, _)), (b, (b_start, _, _)) in zip(ordered, ordered[1:]):
        if b_start < a_end:
            overlapping.update((a, b))
    return named, units, overlapping


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("version")
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    named, units, overlapping = locate(args.version)
    located = {u: r for u, r in units.items() if u not in overlapping}
    print(f"{len(named)} units named, {len(units)} with code found, {len(located)} located without overlap, "
          f"{len(overlapping)} overlapping")
    for unit in sorted(overlapping, key=lambda u: units[u][0]):
        print(f"  overlap: {unit} 0x{units[unit][0]:08X}-0x{units[unit][1]:08X}")
    if not args.write:
        return

    splits_path = Path(f"config/{args.version}/splits.txt")
    symbols_path = Path(f"config/{args.version}/symbols.txt")
    _, existing = read_splits(splits_path)
    taken = [(start, end) for _, ranges in existing for section, start, end, _ in ranges if section == ".text"]
    have = {unit for unit, _ in existing}
    symbols = read_symbols(symbols_path)
    known = {address for _, _, address, _ in symbols}
    added_units, added_symbols = [], []
    for unit, (start, end, functions) in sorted(located.items(), key=lambda item: item[1][0]):
        if unit in have or any(start < e and s < end for s, e in taken):
            continue
        added_units.append(f"{unit}:\n\t.text       start:0x{start:08X} end:0x{end:08X}\n")
        for f_start, f_end in functions:
            if f_start not in known:
                added_symbols.append((f"_fn_{f_start:08X}", ".text", f_start, {"type": "function",
                                                                                   "size": f"0x{f_end - f_start:X}"}))
    with splits_path.open("a") as out:
        out.write("".join("\n" + entry for entry in added_units))
    lines = []
    for name, section, address, options in sorted(symbols + added_symbols, key=lambda s: s[2]):
        rest = " ".join(f"{k}:{v}" if v is not True else k for k, v in options.items())
        lines.append(f"{name} = {section}:0x{address:08X};" + (f" // {rest}" if rest else ""))
    symbols_path.write_text("\n".join(lines) + "\n")
    print(f"added {len(added_units)} units to {splits_path} and {len(added_symbols)} symbols to {symbols_path}")


if __name__ == "__main__":
    main()

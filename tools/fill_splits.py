#!/usr/bin/env python3
"""python tools/fill_splits.py VERSION [--write] [--dump FILE]: split the rest of the original's .text, the code no
unit in config/VERSION/splits.txt has yet, into units.

Where a unit starts: CA's units all include a header that defines Unicode character constants (0, 0xFEFF, ' ', '_',
...) as statics with dynamic initializers, so each unit's .text opens with their initializers, in the C++
initializer table (__xc_a to __xc_z) in link order: the one that clears a word followed by the one that stores 0xFEFF
is a unit's first function. The game's own code is not padded and the libraries linked after it are aligned with
padding, so the first padding after a ret ends the game's code too.

Which unit it is: the linker lays out each object's .text and .data in the same order, and nearly every source file
has its path in .data (`static char THIS_FILE[] = __FILE__;`, and its asserts' __FILE__), so the paths in .data are
the link order of the units they name. Between two units of splits.txt, the segments (from one unit start to the next)
are matched in order to the units named between theirs in .data, by an alignment (dynamic programming) that favours
pairing them off one to one and the units whose .data each segment's functions refer to (each unit's .data runs from
its first path to the next unit's). A segment no name is matched to is an `auto/<category>/<address>.cpp` unit.

A unit of splits.txt grows over the rest of its segment (the code from its start to its first assert, and after its
last), except a Matching unit, whose source would no longer match: that code becomes an auto unit, reported. Ranges
of splits.txt are never shrunk.

Functions are the starts of symbols.txt's functions, `push ebp; mov ebp, esp` prologues (the game keeps frame
pointers), the C++ initializers, call targets, image pointers in .rdata and .data (vtables) and code after alignment
padding. With --write, rewrites splits.txt (in address order) and adds a sized `_fn_` symbol to symbols.txt for each
function start in the code it adds to a unit. --dump FILE writes each segment with its unit and votes.

The version's config.json gives the initializer table (`initializers`: [__xc_a, __xc_z], from _cinit) and where the C
runtime starts (`runtime`), which only sets the category of the auto units after it.
"""
import argparse
import json
import re
import struct
import sys
from bisect import bisect_left, bisect_right
from collections import Counter
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

sys.path.insert(0, str(Path(__file__).parent))
from locate_units import unit_name
from split import original, read_splits, read_symbols

PATH = re.compile(rb"[\x20-\x7e]{6,260}?\.(?:cpp|c)\x00", re.I)
HEX = re.compile(r"0x([0-9a-f]{6,8})")
PADDING = (0xCC, 0x90)
MATCH = 1.0             # the score of pairing a segment with a name, besides the votes for it


class Image:
    def __init__(self, version):
        self.config, self.pe = original(version)
        sections = {s.name: s for s in self.pe.sections}
        self.text = sections[".text"]
        self.text_start = self.text.virtual_address
        self.text_end = self.text_start + self.text.virtual_size
        self.code = self.pe.data[self.text.file_offset:self.text.file_offset + self.text.virtual_size]
        data = sections[".data"]
        self.data_start = data.virtual_address
        self.data = self.pe.data[data.file_offset:data.file_offset + data.file_size]
        self.data_end = self.data_start + len(self.data)
        self.rdata = sections[".rdata"]
        self.disassembler = Cs(CS_ARCH_X86, CS_MODE_32)

    def byte(self, address):
        return self.code[address - self.text_start]

    def in_text(self, value):
        return self.text_start <= value < self.text_end

    def instructions(self, start, end):
        return self.disassembler.disasm_lite(self.code[start - self.text_start:end - self.text_start], start)


def path_units(image):
    """[(address, unit)] of each unit's first source path in .data, in address order."""
    first = {}
    for match in PATH.finditer(image.data):
        unit = unit_name(match.group()[:-1].decode())
        if unit:
            first.setdefault(unit, image.data_start + match.start())
    return sorted((address, unit) for unit, address in first.items())


def initializers(image):
    """The C++ initializer table's functions, in table (link) order."""
    start, end = (int(v, 0) for v in image.config["initializers"])
    values = struct.unpack(f"<{(end - start) // 4}I", image.pe.read(start, end - start))
    return [v for v in values if image.in_text(v)]


def unit_starts(image):
    """The first function of each unit that includes CA's Unicode constants header."""
    def body(address):
        """The first instruction of the function ADDRESS, following a stub's call to its initializer."""
        for _ in range(2):
            found = None
            for _, _, mnemonic, operands in image.instructions(address, min(address + 32, image.text_end)):
                if mnemonic == "push" and operands == "ebp" or mnemonic == "mov" and operands == "ebp, esp":
                    continue
                found = (mnemonic, operands)
                break
            if found and found[0] == "call" and found[1].startswith("0x"):
                address = int(found[1], 16)
                continue
            return found
        return found

    table = initializers(image)
    bodies = [body(a) for a in table]
    starts = []
    for i in range(len(table) - 1):
        first, second = bodies[i], bodies[i + 1]
        if (first and first[0] in ("and", "mov") and re.fullmatch(r"word ptr \[0x[0-9a-f]+\], 0", first[1])
                and second and second[1].endswith(", 0xfeff")):
            starts.append(table[i])
    return sorted(starts)


def ends_function(image, address):
    """Whether the code just before ADDRESS can end a function (a ret, a jmp, or padding after one)."""
    if address <= image.text_start + 5:
        return False
    if image.byte(address - 1) in (0xC3, 0xCC, 0x90) or image.byte(address - 3) == 0xC2:
        return True
    return image.byte(address - 5) == 0xE9 or image.byte(address - 2) == 0xEB


def game_code_end(image, after):
    """The first function from AFTER (the end of the game's last unit in splits.txt) that follows alignment padding
    (a ret, then int3s or nops up to a 16-byte boundary)."""
    for m in re.compile(rb"(?:\xc3|\xc2..)[\xcc\x90]+", re.S).finditer(image.code, after - image.text_start):
        address = image.text_start + m.end()
        if address % 16 == 0:
            return address
    return image.text_end


def function_starts(image, symbols):
    starts = {address for name, section, address, options in symbols
              if section == ".text" and options.get("type", "function") == "function"}
    starts |= {image.text_start + m.start() for m in re.finditer(re.escape(b"\x55\x8b\xec"), image.code)}
    starts |= set(initializers(image))
    for m in re.finditer(rb"(?:\xc3|\xc2..|\xe9....|\xeb.)[\xcc\x90]+", image.code, re.S):
        address = image.text_start + m.end()
        if address % 16 == 0 and image.in_text(address) and image.byte(address) not in PADDING:
            starts.add(address)
    # (vtables and other function tables)
    rdata = image.pe.data[image.rdata.file_offset:image.rdata.file_offset + image.rdata.file_size]
    for section_data in (rdata, image.data):
        for value in struct.unpack(f"<{len(section_data) // 4}I", section_data[:len(section_data) // 4 * 4]):
            if image.in_text(value) and (value in starts or ends_function(image, value)):
                starts.add(value)
    starts = sorted(s for s in starts if image.in_text(s))
    # (call targets, once)
    refs, called = scan(image, starts)
    new = {t for t in called if t not in refs and ends_function(image, t)}
    if new:
        starts = sorted(set(starts) | new)
        refs, _ = scan(image, starts)
    return starts, refs


def scan(image, starts):
    """({function: [the .data addresses it refers to]}, {call targets}) of each function in STARTS (sorted),
    disassembling each up to the next start."""
    refs, called = {}, set()
    for i, start in enumerate(starts):
        end = starts[i + 1] if i + 1 < len(starts) else image.text_end
        found = []
        for _, _, mnemonic, operands in image.instructions(start, end):
            if mnemonic == "call" and operands.startswith("0x"):
                target = int(operands, 16)
                if image.in_text(target):
                    called.add(target)
            elif not mnemonic.startswith("j"):
                for value in HEX.findall(operands):
                    value = int(value, 16)
                    if image.data_start <= value < image.data_end:
                        found.append(value)
        refs[start] = found
    return refs, called


def align(segments, names, votes, started):
    """{segment index: name} pairing SEGMENTS with NAMES in order, maximizing MATCH for each pair plus the votes of the
    segment's functions for the name (VOTES[i]: {name: count}). Only a segment that STARTED a unit (began at a unit
    start) is paired: code without CA's header is a library's (whose .data follows a named unit's, so it votes for it)."""
    m, n = len(segments), len(names)
    score = [[0.0] * (n + 1) for _ in range(m + 1)]
    move = [[None] * (n + 1) for _ in range(m + 1)]
    for i in range(m + 1):
        for j in range(n + 1):
            if i == 0 and j == 0:
                continue
            best, how = float("-inf"), None
            if i and score[i - 1][j] > best:
                best, how = score[i - 1][j], "skip segment"
            if j and score[i][j - 1] > best:
                best, how = score[i][j - 1], "skip name"
            if i and j:
                v = votes[i - 1].get(names[j - 1], 0)
                if started[i - 1]:
                    pair = score[i - 1][j - 1] + v + MATCH
                    if pair > best:
                        best, how = pair, "pair"
            score[i][j], move[i][j] = best, how
    out, i, j = {}, m, n
    while i or j:
        how = move[i][j]
        if how == "pair":
            out[i - 1] = names[j - 1]
            i, j = i - 1, j - 1
        elif how == "skip segment":
            i -= 1
        else:
            j -= 1
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("version")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--dump")
    args = parser.parse_args()
    splits_path = Path(f"config/{args.version}/splits.txt")
    symbols_path = Path(f"config/{args.version}/symbols.txt")
    sys.path.insert(0, ".")
    from configure import LIBS, Matching

    image = Image(args.version)
    runtime = int(image.config["runtime"], 0)
    sections, units = read_splits(splits_path)
    symbols = read_symbols(symbols_path)
    paths = path_units(image)
    path_addresses = [address for address, _ in paths]
    order = {unit: i for i, (_, unit) in enumerate(paths)}
    matching = {obj["name"] for lib in LIBS for obj in lib["objects"] if obj["status"] == Matching}

    starts, refs = function_starts(image, symbols)
    print(f"{len(starts)} functions")
    unit_start = unit_starts(image)
    unit_start_set = set(unit_start)
    notes = []
    placed = sorted((start, end, unit) for unit, ranges in units for section, start, end, _ in ranges
                    if section == ".text")
    game_end = game_code_end(image, max(e for _, e, u in placed if u.startswith("code/")))
    print(f"{len(unit_start)} unit starts; the game's code ends at 0x{game_end:08X}")

    # (the last unit's .data ends where the C runtime's begins: the lowest .data after it that the runtime refers to)
    data_limit = min((v for f in starts[bisect_left(starts, runtime):] for v in refs[f] if v > path_addresses[-1]),
                     default=image.data_end)

    def votes_in(start, end):
        """{link order index: count} of the references to .data from the functions in [START, END)."""
        counter = Counter()
        for f in starts[bisect_left(starts, start):bisect_left(starts, end)]:
            for value in refs[f]:
                if (i := bisect_right(path_addresses, value) - 1) >= 0 and value < data_limit:
                    counter[i] += 1
        return counter

    # the pieces of .text between the cuts: unit starts, the game's end, and the ends of the units of splits.txt
    for (_, a_end, a), (b_start, _, b) in zip(placed, placed[1:]):
        if b_start < a_end:
            raise SystemExit(f"splits.txt: {a} and {b} overlap")
    cuts = sorted({image.text_start, image.text_end, game_end, runtime} | unit_start_set
                  | {s for s, _, _ in placed} | {e for _, e, _ in placed})
    inside = lambda address: (i := bisect_right(placed, (address, float("inf"), "")) - 1) >= 0 and \
        placed[i][0] <= address < placed[i][1]
    pieces = [(a, b) for a, b in zip(cuts, cuts[1:]) if not inside(a)]

    # each piece: the head of the unit it ends at (if it begins at a unit start), the tail of the unit before it (if
    # that unit's segment began at a unit start), else a segment of its own
    owner_of_start = {s: u for s, _, u in placed}
    owner_of_end = {e: u for _, e, u in placed}
    segment_started = {}            # placed unit -> whether its code begins at a unit start (with its head)
    for s, _, u in placed:
        segment_started[u] = s in unit_start_set
    # (a unit of splits.txt that runs past a unit start has the next unit's start: its tail is that unit's, and a
    # segment that starts one)
    overrun = {u for s, e, u in placed if any(s < x < e for x in unit_start[bisect_right(unit_start, s):
                                                                            bisect_left(unit_start, e)])}
    heads, tails, free, continued = {}, {}, [], set()
    for a, b in pieces:
        # (the runtime's units are not CA's: they have no unit starts of their own to grow from)
        if (b in owner_of_start and b not in unit_start_set and a in unit_start_set and a != game_end
                and not owner_of_start[b].startswith("crt/")):
            heads[owner_of_start[b]] = (a, b)
            segment_started[owner_of_start[b]] = True
        else:
            free.append((a, b))
    still_free = []
    for a, b in free:
        u = owner_of_end.get(a)
        if u is not None and u in overrun:
            notes.append(f"{u} runs past the unit start before 0x{a:08X}: its end is another unit's")
            continued.add(a)
            still_free.append((a, b))
        elif (u is not None and segment_started[u] and a not in unit_start_set and not (a < game_end <= b)
              and a != game_end and a != runtime and not u.startswith("crt/")):
            tails[u] = (a, b)
        else:
            still_free.append((a, b))
    free = still_free

    # name the free segments between each pair of placed units
    result = {u: [s, e] for s, e, u in placed}
    autos, dump = [], []
    for u, (a, b) in list(heads.items()) + list(tails.items()):
        if u in matching:
            autos.append((a, b))
            notes.append(f"0x{a:08X}-0x{b:08X} is {u}'s, which is Matching: left as an auto unit")
        else:
            result[u][0] = min(result[u][0], a)
            result[u][1] = max(result[u][1], b)
    # (units of splits.txt with no path in .data, such as main.cpp, keep their place inside a gap)
    anchors = [(s, e, u) for s, e, u in placed if u in order]
    for k in range(len(anchors) + 1):
        lo_address = anchors[k - 1][1] if k else image.text_start
        hi_address = anchors[k][0] if k < len(anchors) else image.text_end
        segments = [(a, b) for a, b in free if lo_address <= a and b <= hi_address]
        if not segments:
            continue
        lo = next((order[u] for _, _, u in reversed(anchors[:k]) if u in order), -1)
        hi = next((order[u] for _, _, u in anchors[k:] if u in order), len(paths))
        names = [i for i in range(lo + 1, hi) if paths[i][1] not in result]
        votes = [votes_in(a, b) for a, b in segments]
        started = [a in unit_start_set or a in continued for a, b in segments]
        paired = align(segments, names, votes, started)
        dump.append(f"{anchors[k - 1][2] if k else 'the start'} .. {anchors[k][2] if k < len(anchors) else 'the end'}:"
                    f" {' '.join(paths[i][1] for i in names)}")
        for i, (a, b) in enumerate(segments):
            if i in paired:
                result[paths[paired[i]][1]] = [a, b]
            else:
                autos.append((a, b))
            dump.append(f"  {a:08X}-{b:08X}{' *' if started[i] else '  '} {paths[paired[i]][1] if i in paired else '-'}"
                        "  " + " ".join(f"{paths[n][1]}:{c}" for n, c in votes[i].most_common(4)))
        unpaired = [paths[i][1] for i in names if i not in paired.values()]
        if unpaired:
            notes.append(f"between {anchors[k - 1][2] if k else 'the start'} and "
                         f"{anchors[k][2] if k < len(anchors) else 'the end'}: no segment for {', '.join(unpaired)}")

    # merge adjacent auto pieces of the same category into units
    def category_of(a):
        if a < game_end:
            return "game"
        if a in unit_start_set or a in continued:
            return "libs"
        return "crt" if a >= runtime else "sdk"

    out_units = {}
    previous = None
    for a, b in sorted(autos):
        cat = category_of(a)
        if previous and previous[1] == a and previous[2] == cat and a not in unit_start_set:
            previous[1] = b
            continue
        previous = [a, b, cat]
        out_units[id(previous)] = previous
    auto_units = {f"auto/{cat}/{a:08X}.cpp": (a, b) for a, b, cat in out_units.values()}

    other_sections = {unit: [r for r in ranges if r[0] != ".text"] for unit, ranges in units}
    final = {u: [(".text", s, e, {})] + other_sections.get(u, []) for u, (s, e) in result.items()}
    for u, (a, b) in auto_units.items():
        final[u] = [(".text", a, b, {})]
    spans = sorted((r[0][1], r[0][2], u) for u, r in final.items())
    for (_, a_end, a), (b_start, _, b) in zip(spans, spans[1:]):
        if b_start < a_end:
            raise SystemExit(f"internal error: {a} and {b} overlap")
    covered = sum(e - s for s, e, _ in spans)
    before = sum(e - s for s, e, _ in placed)
    original_ranges = {u: (s, e) for s, e, u in placed}
    grown = [u for u in original_ranges if tuple(result[u]) != original_ranges[u]]
    added = [u for u in result if u not in original_ranges]
    print(f".text covered: {before} -> {covered} of {image.text_end - image.text_start} bytes")
    print(f"{len(added)} named units added, {len(grown)} grown, {len(auto_units)} auto units; "
          f"{sum(1 for u in order if u not in result)} named units still not located")
    for note in notes:
        print("  " + note)
    if args.dump:
        with open(args.dump, "w", encoding="utf-8", newline="") as f:
            f.write("\n".join(dump) + "\n")
    if not args.write:
        return

    lines = ["Sections:"]
    for name, options in sections.items():
        lines.append(f"\t{name:<11} " + " ".join(f"{k}:{v}" for k, v in options.items()))
    for unit, ranges in sorted(final.items(), key=lambda item: item[1][0][1]):
        lines += ["", f"{unit}:"]
        for section, start, end, options in ranges:
            extra = "".join(f" {k}:{v}" for k, v in options.items())
            lines.append(f"\t{section:<11} start:0x{start:08X} end:0x{end:08X}{extra}")
    splits_path.write_text("\n".join(lines) + "\n")

    # a symbol for each function in the code added to a unit, sized up to the next function or the unit's end
    known = {address for _, section, address, _ in symbols if section == ".text"}
    new_code = []
    for u, ranges in final.items():
        s, e = ranges[0][1], ranges[0][2]
        if u in original_ranges:
            os_, oe = original_ranges[u]
            new_code += [(s, os_), (oe, e)]
        else:
            new_code.append((s, e))
    new_symbols = []
    for s, e in new_code:
        if s >= e:
            continue
        here = [s] + [f for f in starts[bisect_left(starts, s):bisect_left(starts, e)] if f != s]
        for f, g in zip(here, here[1:] + [e]):
            if f not in known:
                new_symbols.append((f"_fn_{f:08X}", ".text", f, {"type": "function", "size": f"0x{g - f:X}"}))
    rows = []
    for name, section, address, options in sorted(symbols + new_symbols, key=lambda s: s[2]):
        rest = " ".join(f"{k}:{v}" if v is not True else k for k, v in options.items())
        rows.append(f"{name} = {section}:0x{address:08X};" + (f" // {rest}" if rest else ""))
    symbols_path.write_text("\n".join(rows) + "\n")
    print(f"wrote {splits_path} ({len(final)} units) and {len(new_symbols)} symbols to {symbols_path}")


if __name__ == "__main__":
    main()

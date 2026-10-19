"""python tools/match_libs.py VERSION LIB... [--start ADDRESS] [--end ADDRESS] [--dump FILE] [--write]: find the
objects of static libraries (such as VC6's LIBCMT.LIB) in the original's .text, and (with --write) make each a unit
in splits.txt, named for its source file, and name its functions in symbols.txt.

The libraries are built with function-level linking (each function its own COMDAT section), and the linker drops the
functions nothing calls, so an object is found as a chain of its functions, in their order in the object, one after
another on their alignment.  Each function's bytes are compared with the original's, the bytes its relocations fix up
left out; then the relocations are checked: what each one points to in the original must agree with where the objects
already placed put that symbol.  That tells apart the objects whose code is the same but for what it calls (fprintf
calls _output, fwprintf _woutput).  Chains are taken best first: the most evidence from relocations, then the most code.
A function the library's version does not match is placed by its symbol, where the placed code points at it, or by
its likeness beside the rest of its object; the code still not placed is left in auto/ units.
"""
import argparse
import re
from bisect import bisect_right
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from split import imports, original  # noqa: E402

CODE = 0x20
REMOVE = 0x800
COMDAT = 0x1000
DIR32, REL32 = 0x06, 0x14
# The size of the field each i386 relocation type fixes up.
RELOCATION_SIZES = {0x06: 4, 0x07: 4, 0x0A: 2, 0x0B: 4, 0x14: 4, 0x02: 2}
STRONG = 0x10                               # a chain shorter than this needs a relocation to vouch for it
TINY = 0x20                                 # a function this short is looked for by a shorter run of its bytes


def archive_members(path):
    """(name, data) for each object in the archive at PATH."""
    data = Path(path).read_bytes()
    if data[:8] != b"!<arch>\n":
        raise SystemExit(f"{path} is not an archive")
    offset, long_names = 8, b""
    while offset + 60 <= len(data):
        header = data[offset:offset + 60]
        name = header[:16].decode("latin-1").rstrip()
        size = int(header[48:58])
        body = data[offset + 60:offset + 60 + size]
        offset += 60 + size + (size & 1)
        if name == "//":
            long_names = body
        elif name in ("/", "/<ECSYMBOLS>/"):
            continue
        else:
            if name.startswith("/") and name[1:].isdigit():
                start = int(name[1:])
                name = long_names[start:long_names.index(b"\0", start)].decode("latin-1")
            yield name.rstrip("/"), body


class Function:
    """A code section of an object."""

    def __init__(self, number, name, alignment, code, mask):
        self.number, self.name, self.alignment, self.code, self.mask = number, name, alignment, code, mask
        self.relocations = []               # (offset, type, symbol index)
        self.symbols = []                   # (name, offset) of the external symbols defined in it
        self.locals = []                    # (name, offset) of its static functions
        self.anchor = longest_run(code, mask)


class Object:
    def __init__(self, lib, name, data):
        self.lib, self.name = lib, name
        self.functions, self.symbols, self.file = read_coff(data)
        self.order = 0
        self.stem = re.split(r"[\\/]", name)[-1].rsplit(".", 1)[0].lower()


def read_coff(obj):
    """The code sections of the COFF object OBJ (but its .text$x: the linker puts those after all of .text), and its
    symbols ({index: (name, section number, value, external)}) and the source file its .file symbol names."""
    machine, count, _, table, symbol_count, optional_size, _ = struct.unpack_from("<HHIIIHH", obj, 0)
    if machine != 0x14C:
        return [], {}, None
    strings = table + symbol_count * 18

    def string(raw):
        if raw[:4] == b"\0\0\0\0":
            start = strings + struct.unpack_from("<I", raw, 4)[0]
            return obj[start:obj.index(b"\0", start)].decode("latin-1")
        return raw.split(b"\0", 1)[0].decode("latin-1")

    symbols, statics, file, i = {}, [], None, 0
    while i < symbol_count:
        entry = obj[table + i * 18:table + i * 18 + 18]
        value, section, kind, storage, aux = struct.unpack_from("<IhHBB", entry, 8)
        symbols[i] = (string(entry[:8]), section, value, storage == 2)
        if storage == 103 and file is None:
            file = obj[table + (i + 1) * 18:table + (i + 1 + aux) * 18].split(b"\0", 1)[0].decode("latin-1")
        elif storage == 3 and kind == 0x20:
            statics.append(i)
        i += 1 + aux

    functions = {}
    for number in range(1, count + 1):
        offset = 20 + optional_size + (number - 1) * 40
        name = obj[offset:offset + 8].split(b"\0", 1)[0].decode("latin-1")
        size, data, relocations, _, relocation_count, _, flags = struct.unpack_from("<IIIIHHI", obj, offset + 16)
        if not flags & CODE or flags & REMOVE or size == 0 or name.startswith(".text$"):
            continue
        code = obj[data:data + size]
        mask = bytearray(b"\1" * size)
        fixups = []
        for r in range(relocation_count):
            address, symbol, kind = struct.unpack_from("<IIH", obj, relocations + r * 10)
            mask[address:address + RELOCATION_SIZES.get(kind, 4)] = bytes(RELOCATION_SIZES.get(kind, 4))
            fixups.append((address, kind, symbol))
        alignment = 1 << (((flags >> 20) & 0xF) - 1) if (flags >> 20) & 0xF else 16
        functions[number] = Function(number, name, alignment, code, bytes(mask))
        functions[number].relocations = fixups
    for name, section, value, external in symbols.values():
        if external and section in functions:
            functions[section].symbols.append((name, value))
    for index in statics:
        name, section, value, _ = symbols[index]
        if section in functions:
            functions[section].locals.append((name, value))
    return list(functions.values()), symbols, file


def longest_run(code, mask):
    """The longest run of CODE no relocation touches (its offset and bytes)."""
    best, start = (0, b""), None
    for i, m in enumerate(mask + b"\0"):
        if m and start is None:
            start = i
        elif not m and start is not None:
            if i - start > len(best[1]):
                best = (start, code[start:i])
            start = None
    return best


class Image:
    def __init__(self, version, low, high):
        _, pe = original(version)
        section = next(s for s in pe.sections if s.name == ".text")
        self.base = section.virtual_address
        self.text = pe.data[section.file_offset:section.file_offset + section.file_size]
        self.imports = {name: slot for slot, name in imports(pe).items()}
        self.low = low or self.base
        self.high = high or self.base + section.virtual_size

    def matches(self, address, function):
        at = address - self.base
        if address < self.low or address + len(function.code) > self.high:
            return False
        window = self.text[at:at + len(function.code)]
        return all(m == 0 or a == b for a, b, m in zip(window, function.code, function.mask))

    def find(self, function):
        offset, run = function.anchor
        found = []
        if len(run) < (1 if len(function.code) <= TINY else 4):
            return found
        at = self.text.find(run, self.low - self.base + offset, self.high - self.base)
        while at >= 0:
            if self.matches(self.base + at - offset, function):
                found.append(self.base + at - offset)
            at = self.text.find(run, at + 1, self.high - self.base)
        return found

    def dword(self, address):
        return struct.unpack_from("<I", self.text, address - self.base)[0]


def import_name(name):
    """NAME, but an import address table slot's without its __stdcall @N (the original's import table has no N)."""
    return re.sub(r"@\d+$", "", name) if name.startswith("__imp_") else name


def align(address, alignment):
    return (address + alignment - 1) & ~(alignment - 1)


def chains(image, obj):
    """The chains of OBJ's functions (tuples of (function index, address)) its functions' matches start."""
    found = set()
    functions = obj.functions
    for i, seed in enumerate(functions):
        for address in image.find(seed):
            chain = [(i, address)]
            end = address + len(seed.code)
            for j in range(i + 1, len(functions)):
                for candidate in sorted({end, align(end, functions[j].alignment)}):
                    if image.matches(candidate, functions[j]):
                        chain.append((j, candidate))
                        end = candidate + len(functions[j].code)
                        break
            start = address
            for j in range(i - 1, -1, -1):
                size = len(functions[j].code)
                for candidate in range(start - size, start - size - functions[chain[0][0]].alignment, -1):
                    if align(candidate + size, functions[chain[0][0]].alignment) == start or candidate + size == start:
                        if image.matches(candidate, functions[j]):
                            chain.insert(0, (j, candidate))
                            start = candidate
                            break
                    else:
                        break
            found.add(tuple(chain))
    return found


class Chain:
    def __init__(self, image, obj, links):
        self.obj, self.links = obj, links
        self.start = links[0][1]
        self.end = links[-1][1] + len(obj.functions[links[-1][0]].code)
        self.size = sum(len(obj.functions[i].code) for i, _ in links)
        where = {obj.functions[i].number: address for i, address in links}
        self.defines = {}
        for i, address in links:
            for name, value in obj.functions[i].symbols:
                self.defines[name] = address + value
        # (what each relocation points to in the original, by symbol name; and the relocations within the chain that
        # disagree with it)
        self.refers, self.broken = {}, 0
        for i, address in links:
            function = obj.functions[i]
            for offset, kind, index in function.relocations:
                if kind not in (DIR32, REL32) or index not in obj.symbols:
                    continue
                name, section, value, external = obj.symbols[index]
                name = import_name(name)
                addend = struct.unpack_from("<I", function.code, offset)[0]
                target = (image.dword(address + offset) - addend) & 0xFFFFFFFF
                if kind == REL32:
                    target = (target + address + offset + 4) & 0xFFFFFFFF
                if section in where:
                    if target != where[section] + (0 if external else value):
                        self.broken += 1
                elif external and section == 0:
                    self.refers[name] = target

    def evidence(self, known):
        """How many of the chain's symbols agree with KNOWN ({name: address}), or None when one disagrees."""
        agree = 0
        for table in (self.defines, self.refers):
            for name, address in table.items():
                if name in known:
                    if known[name] != address:
                        return None
                    agree += 1
        return agree


def similarity(image, address, function):
    """The share of FUNCTION's bytes (but those its relocations fix up) the original's at ADDRESS agree with."""
    at = address - image.base
    window = image.text[at:at + len(function.code)]
    same = sum(1 for a, b, m in zip(window, function.code, function.mask) if m and a == b)
    return same / max(1, sum(function.mask))


def likely_starts(image, low, high, function, step=3, width=6):
    """The addresses in [LOW, HIGH) most of FUNCTION's runs of WIDTH bytes would put its start at, best first."""
    votes = {}
    window = image.text[low - image.base:high - image.base]
    for offset in range(0, len(function.code) - width + 1, step):
        if not all(function.mask[offset:offset + width]):
            continue
        run = function.code[offset:offset + width]
        at = window.find(run)
        while at >= 0:
            start = low + at - offset
            if low <= start and start + min(len(function.code), 16) <= high:
                votes[start] = votes.get(start, 0) + 1
            at = window.find(run, at + 1)
    return sorted(votes, key=lambda s: -votes[s])[:4]


class Fuzzy:
    """A function placed by its likeness to the library's (its code changed since: another version of the library)."""

    def __init__(self, obj, index, address, score, why):
        self.obj, self.index, self.start, self.score, self.why = obj, index, address, score, why
        self.end = address + len(obj.functions[index].code)


def fill_gaps(image, objects, taken, known, threshold=0.6):
    """Functions placed in the gaps between TAKEN's chains by their likeness: first those the symbols KNOWN put
    there, then the rest of their objects and of the objects beside each gap.  Each object's functions stay together
    and in order."""
    spans = {c.obj: [c.start, c.end, c.links[0][0], c.links[-1][0]] for c in taken}
    placed = []
    defined = {}
    for obj in objects:
        for index, function in enumerate(obj.functions):
            for name, value in function.symbols:
                defined.setdefault(name, []).append((obj, index, value))

    def free(start, end, obj):
        for o, (s, e, _, _) in spans.items():
            if o is not obj and s < end and start < e:
                return False
        return image.low <= start and end <= image.high

    def add(obj, index, start, end, score, why):
        span = spans.get(obj)
        if span:
            first, last = span[2], span[3]
            if not (index < first and end <= span[0] or index > last and start >= span[1]):
                return False
            new = [min(span[0], start), max(span[1], end), min(first, index), max(last, index)]
        else:
            new = [start, end, index, index]
        if not free(new[0], new[1], obj):
            return False
        spans[obj] = new
        placed.append(Fuzzy(obj, index, start, score, why))
        return True

    # (a symbol the placed code points at is where its function starts, however much that function changed)
    for name, address in sorted(known.items(), key=lambda kv: kv[1]):
        if not image.low <= address < image.high or any(s <= address < e for s, e, _, _ in spans.values()):
            continue
        for obj, index, value in defined.get(name, []):
            start = address - value
            end = min(start + len(obj.functions[index].code), image.high)
            if add(obj, index, start, end, similarity(image, start, obj.functions[index]), name):
                break

    # (then each placed object's missing functions, beside it)
    changed = True
    while changed:
        changed = False
        bounds = sorted((s, e) for s, e, _, _ in spans.values())
        for obj, (start, end, first, last) in sorted(spans.items(), key=lambda kv: kv[1][0]):
            after = min((s for s, _ in bounds if s >= end), default=image.high)
            before = max((e for _, e in bounds if e <= start), default=image.low)
            tries = [(i, end, after) for i in range(last + 1, len(obj.functions))]
            tries += [(i, before, start) for i in range(first - 1, -1, -1)]
            for index, low, high in tries:
                function = obj.functions[index]
                if len(function.code) < 8 or high - low < 8:
                    continue
                best = max(((similarity(image, s, function), s) for s in likely_starts(image, low, high, function)),
                           default=(0, 0))
                if best[0] >= threshold and add(obj, index, best[1], min(best[1] + len(function.code), high),
                                                best[0], "likeness"):
                    changed = True
                    break
            if changed:
                break
    return placed, spans


def resolve(candidates, known):
    """The chains taken, best first, each object's at most once and none overlapping (KNOWN: the addresses of the
    symbols known already)."""
    known, taken, placed = dict(known), [], set()
    while True:
        open_ = []
        for chain in candidates:
            if chain.obj in placed or chain.broken or any(t.start < chain.end and chain.start < t.end for t in taken):
                continue
            evidence = chain.evidence(known)
            if evidence is not None and (chain.size >= STRONG or evidence):
                open_.append((evidence, chain.size, chain))
        open_.sort(key=lambda c: (-c[0], -c[1], c[2].start))
        progress = False
        for evidence, size, chain in open_:
            if chain.obj in placed or any(t.start < chain.end and chain.start < t.end for t in taken):
                continue
            if chain.evidence(known) != evidence:
                continue
            # (a chain must beat every other open chain for its object and every chain it overlaps)
            # (but of the objects whose code here is the same, the first is taken)
            rivals = [(e, s) for e, s, c in open_ if c is not chain and c.obj not in placed
                      and (c.obj is chain.obj or (c.start < chain.end and chain.start < c.end))
                      and not (c.obj.order > chain.obj.order and (c.start, c.end) == (chain.start, chain.end))]
            if any((e, s) >= (evidence, size) for e, s in rivals):
                continue
            taken.append(chain)
            placed.add(chain.obj)
            known.update(chain.defines)
            for name, address in chain.refers.items():
                known.setdefault(name, address)
            progress = True
        if not progress:
            return sorted(taken, key=lambda c: c.start), known


def is_padding(image, start, end):
    return all(b in (0xCC, 0x90, 0x00) for b in image.text[start - image.base:end - image.base])


def build_units(image, spans, fuzzy, threshold=0.6):
    """[(start, end, object or None)] covering [image.low, image.high): each placed object's code, with the gaps
    beside it that hold its functions not found.  A gap goes to the object before it when the last function placed
    there was placed only by its symbol (how long it is now is not known) or when only that object has functions
    missing on that side, to the object after it when only that one does; padding goes to the object before it; the
    other gaps are units of their own."""
    unsure = {(f.obj, f.index) for f in fuzzy if f.score < threshold}
    ordered = sorted(spans.items(), key=lambda kv: kv[1][0])
    units, end, previous = [], image.low, None
    for obj, (start, stop, first, last) in ordered + [(None, (image.high, image.high, 0, 0))]:
        if start > end:
            after = previous is not None and ((previous, spans[previous][3]) in unsure
                                              or spans[previous][3] < len(previous.functions) - 1)
            before = obj is not None and first > 0
            if previous is not None and (is_padding(image, end, start) or (previous, spans[previous][3]) in unsure
                                         or after and not before):
                units[-1][1] = start
            elif before and not after:
                if obj is not None:
                    start = end
            else:
                units.append([end, start, None])
        if obj is not None:
            units.append([start, stop, obj])
            end, previous = stop, obj
    return [tuple(u) for u in units]


def unit_names(units, category):
    """{start: the unit's name}: crt/ and the source file its object's .file symbol names, else auto/CATEGORY/."""
    names, taken = {}, {}
    for start, _, obj in units:
        if obj is None:
            names[start] = f"auto/{category}/{start:08X}.cpp"
            continue
        file = (obj.file or obj.stem + ".c").replace("\\", "/").lower()
        name = f"crt/{file}"
        stem = name.rsplit(".", 1)[0]
        if stem in taken:
            name = f"crt/{Path(obj.lib).stem.lower()}/{file}"
            stem = name.rsplit(".", 1)[0]
        if stem in taken:
            raise SystemExit(f"match_libs.py: {obj.name} and {taken[stem]} would both be {name}")
        taken[stem] = obj.name
        names[start] = name
    return names


def function_names(taken, fuzzy):
    """[(address, name, size or None, scope)] of the placed functions' symbols (the size only of those found exactly:
    to the next symbol in its section, as an assembler's section can have several entry points).  An assembler's
    local symbols are its labels, not functions, and are left out."""
    found = []
    placings = [(chain.obj, chain.obj.functions[i], address, True) for chain in taken for i, address in chain.links]
    placings += [(f.obj, f.obj.functions[f.index], f.start, False) for f in fuzzy]
    for obj, function, address, exact in placings:
        statics = [] if (obj.file or "").lower().endswith(".asm") else function.locals
        symbols = sorted([(value, name, "global") for name, value in function.symbols]
                         + [(value, name, "local") for name, value in statics])
        for k, (value, name, scope) in enumerate(symbols):
            following = [v for v, _, _ in symbols[k + 1:] if v > value]
            size = (following[0] if following else len(function.code)) - value
            found.append((address + value, name, size if exact else None, scope))
    return sorted(found)


def write_splits(path, low, high, units, names):
    """Replace splits.txt's units in [LOW, HIGH) (crt/ and auto/ units only) with UNITS; what of a unit replaced lies
    past HIGH stays a unit of its own."""
    text = Path(path).read_text(encoding="utf-8")
    blocks = re.split(r"\n(?=\S)", text.rstrip("\n"))
    kept, extra = [], []
    for block in blocks:
        header = block.split("\n", 1)[0]
        match = re.search(r"\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", block)
        if header == "Sections:" or not match:
            kept.append(block)
            continue
        start, end = int(match[1], 16), int(match[2], 16)
        if end <= low or start >= high:
            kept.append(block)
            continue
        unit = header[:-1]
        if not (unit.startswith("crt/") or unit.startswith("auto/")) or block.count("start:") > 1:
            raise SystemExit(f"match_libs.py: {unit} ({start:#x}-{end:#x}) is not a .text-only crt/ or auto/ unit")
        if start < low:
            raise SystemExit(f"match_libs.py: {unit} starts before {low:#x}")
        if end > high:
            category = unit.split("/")[1] if unit.startswith("auto/") else "crt"
            extra.append((high, end, f"auto/{category}/{high:08X}.cpp"))
    for start, end, _ in units:
        extra.append((start, end, names[start]))
    for start, end, name in extra:
        kept.append(f"{name}:\n\t.text       start:0x{start:08X} end:0x{end:08X}\n")
    head, rest = kept[0], kept[1:]

    def key(block):
        match = re.search(r"start:0x([0-9A-Fa-f]+)", block)
        return int(match[1], 16)
    rest.sort(key=key)
    Path(path).write_text("\n".join(b.rstrip("\n") + "\n" for b in [head] + rest), encoding="utf-8", newline="")


def write_symbols(path, functions):
    """Name the placed functions in symbols.txt: a generated _fn_ name at its address is renamed, and the size of one
    found exactly set from the library's; a symbol with another name there is kept, and reported.  The generated
    symbols inside a function found exactly are removed, and those a new symbol lands in cut short."""
    lines = Path(path).read_text(encoding="utf-8").splitlines()
    by_address, names = {}, {}
    for number, line in enumerate(lines):
        match = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+);", line)
        if match:
            by_address.setdefault(int(match[2], 16), []).append(number)
        if line.strip():
            names[line.split(" ", 1)[0]] = number

    def sized(line, size, scope=None):
        body, _, comment = line.partition(" // ")
        options = [o for o in (comment or "type:function").split() if not o.startswith("size:")]
        options.insert(1 if options and options[0].startswith("type:") else 0, f"size:{size:#x}")
        if scope:
            options = [o for o in options if not o.startswith("scope:")] + [f"scope:{scope}"]
        return f"{body} // {' '.join(options)}"

    added, renamed, kept, inside, exact, done = [], 0, [], [], [], set()
    for address, name, size, scope in sorted(functions, key=lambda f: (f[0], f[3] != "global", f[1])):
        if address in done:
            continue                        # (another name for the same code: an assembler's label)
        numbers = by_address.get(address, [])
        # (statics of the same name in different objects are different functions)
        if name in names and (scope == "global" or names[name] in numbers):
            number = names[name]
            if number in numbers and size is not None:
                lines[number] = sized(lines[number], size)
                exact.append((address, size))
            done.add(address)
            continue
        generated = [n for n in numbers if lines[n].startswith("_fn_")]
        if generated:
            number = generated[0]
            line = f"{name} = .text:0x{address:08X};" + (" // " + lines[number].partition(" // ")[2]
                                                       if " // " in lines[number] else "")
            lines[number] = sized(line, size, scope) if size is not None else line
            if size is None:
                lines[number] = re.sub(r"scope:\S+", f"scope:{scope}", lines[number]) if "scope:" in line \
                    else lines[number] + f" scope:{scope}"
            renamed += 1
        elif numbers:
            kept.append((address, name, lines[numbers[0]].split(" ", 1)[0]))
            done.add(address)
            continue
        else:
            size_option = f" size:{size:#x}" if size is not None else ""
            added.append(f"{name} = .text:0x{address:08X}; // type:function{size_option} scope:{scope}")
            inside.append(address)
        if size is not None:
            exact.append((address, size))
        names[name] = -1
        done.add(address)
    # (a function found exactly has no other function inside it)
    gone = set()
    for start, size in exact:
        for address in range(start + 1, start + size):
            gone.update(n for n in by_address.get(address, []) if lines[n].startswith("_fn_"))
    # (a generated symbol a new one lands in ends where the new one starts)
    starts = sorted(by_address)
    for address in inside:
        k = bisect_right(starts, address) - 1
        if k < 0:
            continue
        for number in by_address[starts[k]]:
            match = re.search(r"size:0x([0-9A-Fa-f]+)", lines[number])
            if lines[number].startswith("_fn_") and match and starts[k] + int(match[1], 16) > address:
                lines[number] = lines[number].replace(match[0], f"size:{address - starts[k]:#x}")
    lines = [line for number, line in enumerate(lines) if number not in gone] + added

    def key(line):
        match = re.match(r"\S+ = ([^:]+):0x([0-9A-Fa-f]+);", line)
        return (match[1], int(match[2], 16)) if match else ("", 0)
    order = {".text": 0, ".rdata": 1, ".data": 2, ".bss": 3, ".data1": 4}
    lines.sort(key=lambda l: (order.get(key(l)[0], 9), key(l)[1]))
    Path(path).write_text("\n".join(lines) + "\n", encoding="utf-8", newline="")
    return renamed, len(added), len(gone), kept


def known_symbols(version, low, high):
    """{name: address} of symbols.txt's named functions in [LOW, HIGH)."""
    known = {}
    for line in Path(f"config/{version}/symbols.txt").read_text(encoding="utf-8").splitlines():
        match = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+);", line)
        if match and not match[1].startswith(("_fn_", "lbl_", "_lbl_")) and low <= int(match[2], 16) < high:
            known[match[1]] = int(match[2], 16)
    return known


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("version")
    parser.add_argument("libs", nargs="+")
    parser.add_argument("--start", type=lambda s: int(s, 0))
    parser.add_argument("--end", type=lambda s: int(s, 0))
    parser.add_argument("--category", default="crt", help="the category of the units no object is found for")
    parser.add_argument("--dump", help="write the placings to this file")
    parser.add_argument("--write", action="store_true", help="rewrite splits.txt's units and name symbols.txt's "
                                                             "functions")
    args = parser.parse_args()

    image = Image(args.version, args.start, args.end)
    objects = [Object(Path(lib).name, name, data) for lib in args.libs for name, data in archive_members(lib)]
    objects = [o for o in objects if o.functions]
    for order, obj in enumerate(objects):
        obj.order = order
    candidates = [Chain(image, obj, links) for obj in objects for links in chains(image, obj)]
    taken, known = resolve(candidates, {**image.imports, **known_symbols(args.version, image.low, image.high)})
    fuzzy, spans = fill_gaps(image, objects, taken, known)
    units = build_units(image, spans, fuzzy)
    names = unit_names(units, args.category)
    exact = {c.obj: c for c in taken}
    extra = {}
    for f in fuzzy:
        extra.setdefault(f.obj, []).append(f)

    lines = []
    for start, end, obj in units:
        if obj is None:
            lines.append(f"{start:08X}-{end:08X} {names[start]} (no object found)")
            continue
        chain = exact.get(obj)
        notes = [f"{len(chain.links)} exact" if chain else "0 exact"]
        notes += [f"{(obj.functions[f.index].symbols or [(str(f.index), 0)])[0][0]}@{f.start:X}~{f.score:.2f}"
                  for f in sorted(extra.get(obj, []), key=lambda f: f.start)]
        lines.append(f"{start:08X}-{end:08X} {names[start]} ({obj.lib} {obj.name}: {len(obj.functions)} functions, "
                     f"{', '.join(notes)})")
    report = "\n".join(lines)
    if args.dump:
        Path(args.dump).write_text(report + "\n", encoding="utf-8", newline="")
    elif not args.write:
        print(report)
    lost = sum(e - s for s, e, o in units if o is None)
    print(f"{len(spans)} of {len(objects)} objects placed ({len(fuzzy)} functions by likeness); "
          f"{lost:#x} of the {image.high - image.low:#x} bytes in units no object is found for", file=sys.stderr)
    if args.write:
        write_splits(f"config/{args.version}/splits.txt", image.low, image.high, units, names)
        renamed, added, removed, kept = write_symbols(f"config/{args.version}/symbols.txt",
                                                      function_names(taken, fuzzy))
        print(f"splits.txt: {len(units)} units; symbols.txt: {renamed} functions renamed, {added} added, "
              f"{removed} removed (inside the functions found exactly)",
              file=sys.stderr)
        for address, name, other in kept:
            print(f"  {address:08X}: {name} is already {other}", file=sys.stderr)


if __name__ == "__main__":
    main()

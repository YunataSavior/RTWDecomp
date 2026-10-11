#!/usr/bin/env python3
"""python tools/split.py VERSION: the original executable as one COFF object per unit (build/VERSION/obj), like
decomp-toolkit's `dol split`, from config/VERSION/symbols.txt and config/VERSION/splits.txt (its formats).

RomeTW.exe has no base relocation table, so references are recovered by disassembling each function: rel32 calls and
jumps that leave the function, and 32-bit displacements and immediates that land in the image. A function's jump and
index tables (which VC6 places after its code) and data are scanned for aligned pointers into the image. A reference's
target is the symbol containing it, else `__imp_` for an import address table slot, else a generated `_fn_`/`_lbl_` name.
The .bss of splits.txt is the uninitialized tail of the original's .data (the linker merged them).
"""
import hashlib
import json
import re
import struct
import sys
from bisect import bisect_right
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_BRANCH_RELATIVE, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

sys.path.insert(0, str(Path(__file__).parent))
from pe import PEFile

DIR32, REL32 = 0x0006, 0x0014
SECTION_FLAGS = {"code": 0x60000020, "data": 0xC0000040, "rodata": 0x40000040, "bss": 0xC0000080}
ALIGN_FLAGS = {1: 0x00100000, 2: 0x00200000, 4: 0x00300000, 8: 0x00400000, 16: 0x00500000, 32: 0x00600000}


def read_symbols(path):
    """[(name, section, address, {option: value})] of a symbols.txt."""
    symbols = []
    for number, line in enumerate(Path(path).read_text().splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        match = re.fullmatch(r"(\S+) = ([^:\s]+):0x([0-9A-Fa-f]+);(?: // (.*))?", line)
        if not match:
            raise SystemExit(f"{path}:{number}: cannot parse: {line}")
        options = dict(option.split(":", 1) if ":" in option else (option, True)
                       for option in (match[4] or "").split())
        symbols.append((match[1], match[2], int(match[3], 16), options))
    return symbols


def read_splits(path):
    """({section: {option: value}}, [(unit, [(section, start, end, {option: value})])]) of a splits.txt."""
    sections, units, current = {}, [], None
    for number, line in enumerate(Path(path).read_text().splitlines(), 1):
        if not line.strip() or line.lstrip().startswith("//"):
            continue
        if not line[0].isspace():
            name = line.rstrip()
            if not name.endswith(":"):
                raise SystemExit(f"{path}:{number}: expected a unit or Sections:")
            current = None if name == "Sections:" else []
            if current is not None:
                units.append((name[:-1], current))
            continue
        fields = line.split()
        options = dict(field.split(":", 1) for field in fields[1:])
        if current is None:
            sections[fields[0]] = options
        else:
            current.append((fields[0], int(options.pop("start"), 16), int(options.pop("end"), 16), options))
    return sections, units


def original(version):
    """The configured original executable, checked against its SHA-1."""
    config = json.loads(Path(f"config/{version}/config.json").read_text())
    path = Path(config["original"])
    if not path.exists():
        raise SystemExit(f"{path}: copy the original executable here (see README.md)")
    if hashlib.sha1(path.read_bytes()).hexdigest() != config["sha1"]:
        raise SystemExit(f"{path}: SHA-1 is not {config['sha1']}")
    return config, PEFile(path)


def imports(pe):
    """{import address table slot: __imp_ symbol} of the original."""
    coff = struct.unpack_from("<I", pe.data, 0x3C)[0] + 4
    rva = struct.unpack_from("<I", pe.data, coff + 20 + 104)[0]
    cstr = lambda address: pe.data[(o := pe.address_to_offset(address)):pe.data.index(b"\0", o)].decode()
    slots, descriptor = {}, pe.image_base + rva
    while True:
        lookup, _, _, name, thunks = struct.unpack("<IIIII", pe.read(descriptor, 20))
        if not name:
            return slots
        entry, slot = pe.image_base + (lookup or thunks), pe.image_base + thunks
        while value := struct.unpack("<I", pe.read(entry, 4))[0]:
            # (__declspec(dllimport) names carry the C underscore; a __stdcall @N suffix is not known from the import
            # name, so symbols.txt names the slot where it matters)
            slots[slot] = (f"__imp__{cstr(pe.image_base + value + 2)}" if not value & 0x80000000
                           else f"__imp_{cstr(pe.image_base + name)}_{value & 0xFFFF}")
            entry, slot = entry + 4, slot + 4
        descriptor += 20


class Symbols:
    """symbols.txt by address. A symbol without a size extends to the next one in its section within a unit, but only
    a reference to its own address resolves to it (a size is known where the symbol's extent is)."""

    def __init__(self, symbols, pe):
        self.rows = sorted((address, name, section, options) for name, section, address, options in symbols)
        self.addresses = [row[0] for row in self.rows]
        self.sizes, self.known = [], []
        for i, (address, name, section, options) in enumerate(self.rows):
            if "size" in options:
                self.sizes.append(int(options["size"], 0))
                self.known.append(self.sizes[-1])
                continue
            following = next((row[0] for row in self.rows[i + 1:] if row[2] == section), None)
            pe_section = pe.section_for_address(address)
            end = pe_section.virtual_address + max(pe_section.virtual_size, pe_section.file_size)
            self.sizes.append((following or end) - address)
            self.known.append(1)

    def containing(self, address):
        """(name, start) of the symbol ADDRESS is in, or None."""
        i = bisect_right(self.addresses, address) - 1
        if i >= 0 and address < self.rows[i][0] + max(self.known[i], 1):
            return self.rows[i][1], self.rows[i][0]
        return None

    def within(self, start, end):
        """[(address, size, name, options)] of the symbols in [START, END)."""
        lo, hi = bisect_right(self.addresses, start - 1), bisect_right(self.addresses, end - 1)
        return [(self.rows[i][0], self.sizes[i], self.rows[i][1], self.rows[i][3]) for i in range(lo, hi)]


class Splitter:
    def __init__(self, version):
        self.config, self.pe = original(version)
        self.sections, self.units = read_splits(f"config/{version}/splits.txt")
        self.symbols = Symbols(read_symbols(f"config/{version}/symbols.txt"), self.pe)
        self.imports = imports(self.pe)
        first, last = self.pe.sections[0], self.pe.sections[-1]
        self.image = (first.virtual_address, last.virtual_address + max(last.virtual_size, last.file_size))
        self.disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
        self.disassembler.detail = True

    def in_image(self, value):
        return self.image[0] <= value < self.image[1]

    def target(self, address):
        """(symbol, addend) a reference to ADDRESS is written against."""
        found = self.symbols.containing(address)
        if found:
            return found[0], address - found[1]
        if address in self.imports:
            return self.imports[address], 0
        # (with the C underscore, so a source can declare an unnamed function or variable extern "C" by this name)
        code = self.pe.section_for_address(address).characteristics & 0x20000000
        return f"_{'fn' if code else 'lbl'}_{address:08X}", 0

    def pointers(self, start, end, relocs):
        """Aligned 32-bit values in [START, END) that point into the image."""
        for address in range((start + 3) & ~3, end - 3, 4):
            value = struct.unpack("<I", self.pe.read(address, 4))[0]
            if self.in_image(value):
                relocs.append((address, DIR32, value))

    def code(self, start, end, relocs):
        """References out of each function in [START, END)."""
        functions = [(a, s) for a, s, _, options in self.symbols.within(start, end)
                     if options.get("type", "function") == "function"] or [(start, end - start)]
        for function, size in functions:
            function_end = min(function + size, end)
            tables = function_end
            for ins in self.disassembler.disasm(self.pe.read(function, function_end - function), function):
                if ins.address >= tables:
                    break
                for op in ins.operands:
                    if op.type == X86_OP_MEM and ins.disp_size == 4:
                        value = op.mem.disp & 0xFFFFFFFF
                        if self.in_image(value):
                            relocs.append((ins.address + ins.disp_offset, DIR32, value))
                            if function <= value < function_end:
                                tables = min(tables, value)
                    elif op.type == X86_OP_IMM and ins.imm_size == 4:
                        value = op.imm & 0xFFFFFFFF
                        if ins.group(CS_GRP_BRANCH_RELATIVE):
                            if not function <= value < function_end:
                                relocs.append((ins.address + ins.imm_offset, REL32, value))
                        elif self.in_image(value):
                            relocs.append((ins.address + ins.imm_offset, DIR32, value))
            self.pointers(tables, function_end, relocs)

    def split(self, unit, ranges, path):
        coff_sections, defined = [], []
        for section, start, end, _ in ranges:
            settings = self.sections.get(section)
            if settings is None:
                raise SystemExit(f"splits.txt: {unit}: {section} is not in Sections:")
            kind = settings["type"]
            bss = kind == "bss"
            data = None if bss else bytearray(self.pe.read(start, end - start))
            if not bss and len(data) != end - start:
                raise SystemExit(f"splits.txt: {unit}: {section} 0x{start:08X}-0x{end:08X} is past the file's data")
            relocs = []
            if kind == "code":
                self.code(start, end, relocs)
            elif not bss:
                self.pointers(start, end, relocs)
            written = []
            for address, kind_, value in sorted(set(relocs)):
                symbol, addend = self.target(value)
                # (the field holds the addend, as the compiler writes it: DIR32 adds the symbol's address and REL32
                # the symbol's minus the next instruction's)
                struct.pack_into("<i", data, address - start, addend)
                written.append((address - start, symbol, kind_))
            index = len(coff_sections) + 1
            for address, _, name, options in self.symbols.within(start, end):
                function = kind == "code" and options.get("type", "function") == "function"
                defined.append((name, index, address - start, function, options.get("scope") != "local"))
            flags = SECTION_FLAGS[kind] | ALIGN_FLAGS.get(int(settings.get("align", "4"), 0), 0)
            coff_sections.append((section, flags, data, end - start, written))
        write_coff(path, coff_sections, defined)

    def run(self, version):
        out = Path(f"build/{version}/obj")
        for unit, ranges in self.units:
            self.split(unit, ranges, out / Path(unit).with_suffix(".obj"))
        (out / "ok").touch()


def write_coff(path, sections, defined):
    """An i386 COFF object: SECTIONS [(name, flags, data or None for bss, size, [(offset, symbol, type)])] and
    DEFINED symbols [(name, section index, value, function, external)]; referenced names not defined are external."""
    strings = bytearray(4)
    index = {}
    entries = []

    def symbol(name, value, section, function, external):
        encoded = name.encode()
        if len(encoded) <= 8:
            field = encoded.ljust(8, b"\0")
        else:
            field = struct.pack("<II", 0, len(strings))
            strings.extend(encoded + b"\0")
        index.setdefault(name, len(entries))
        entries.append(field + struct.pack("<IhHBB", value, section, 0x20 if function else 0, 2 if external else 3, 0))

    for name, section, value, function, external in defined:
        symbol(name, value, section, function, external)
    for *_, relocs in sections:
        for _, name, _ in relocs:
            if name not in index:
                symbol(name, 0, 0, False, True)
    struct.pack_into("<I", strings, 0, len(strings))

    header_size = 20 + 40 * len(sections)
    body, headers, cursor = bytearray(), bytearray(), header_size
    for name, flags, data, size, relocs in sections:
        raw = 0 if data is None else cursor
        if data is not None:
            body += data
            cursor += len(data)
        reloc_offset = cursor if relocs else 0
        for offset, target, kind in relocs:
            body += struct.pack("<IIH", offset, index[target], kind)
            cursor += 10
        headers += struct.pack("<8sIIIIIIHHI", name.encode()[:8], 0, 0, size, raw, reloc_offset, 0, len(relocs), 0,
                               flags)
    header = struct.pack("<HHIIIHH", 0x14C, len(sections), 0, cursor, len(entries), 0, 0)
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + headers + body + b"".join(entries) + strings)


if __name__ == "__main__":
    Splitter(sys.argv[1]).run(sys.argv[1])

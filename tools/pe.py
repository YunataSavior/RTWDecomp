"""A PE32 image's sections, read by virtual address (from rayanht/mwcc, CC0)."""
import struct
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Section:
    name: str
    virtual_address: int
    virtual_size: int
    file_offset: int
    file_size: int
    characteristics: int


class PEFile:
    def __init__(self, path: Path):
        self.data = Path(path).read_bytes()
        coff = struct.unpack_from("<I", self.data, 0x3C)[0] + 4
        if self.data[coff - 4:coff] != b"PE\0\0":
            raise ValueError(f"{path} is not a PE executable")
        _, count = struct.unpack_from("<HH", self.data, coff)
        optional_size = struct.unpack_from("<H", self.data, coff + 16)[0]
        self.image_base = struct.unpack_from("<I", self.data, coff + 20 + 28)[0]
        table = coff + 20 + optional_size
        sections = []
        for i in range(count):
            offset = table + i * 40
            virtual_size, rva, file_size, file_offset = struct.unpack_from("<IIII", self.data, offset + 8)
            sections.append(Section(self.data[offset:offset + 8].split(b"\0", 1)[0].decode("ascii"), self.image_base + rva,
                                    virtual_size, file_offset, file_size,
                                    struct.unpack_from("<I", self.data, offset + 36)[0]))
        self.sections = tuple(sections)
        self.found = {}

    def section_for_address(self, address):
        for section in self.sections:
            if section.virtual_address <= address < section.virtual_address + max(section.virtual_size, section.file_size):
                return section
        raise ValueError(f"address 0x{address:x} is not in a section")

    def address_to_offset(self, address):
        section = self.section_for_address(address)
        if address - section.virtual_address >= section.file_size:
            raise ValueError(f"address 0x{address:x} has no file-backed data")
        return section.file_offset + address - section.virtual_address

    def offset_to_address(self, offset):
        for section in self.sections:
            if section.file_offset <= offset < section.file_offset + section.file_size:
                return section.virtual_address + offset - section.file_offset
        raise ValueError(f"file offset 0x{offset:x} is not in a section")

    def read(self, address, size):
        offset = self.address_to_offset(address)
        return self.data[offset:offset + size]

    def find(self, needle, limit=None):
        """The addresses NEEDLE occurs at in the data sections (the first LIMIT)."""
        key = (needle, limit)
        if key not in self.found:
            addresses = []
            for section in self.sections:
                if section.characteristics & 0x20000000:
                    continue
                data = self.data[section.file_offset:section.file_offset + section.file_size]
                start = 0
                while (limit is None or len(addresses) < limit) and (offset := data.find(needle, start)) >= 0:
                    addresses.append(section.virtual_address + offset)
                    start = offset + 1
            self.found[key] = addresses
        return self.found[key]

    def contains(self, address, needle):
        """Whether NEEDLE is at ADDRESS."""
        try:
            offset = self.address_to_offset(address)
        except ValueError:
            return False
        return self.data[offset:offset + len(needle)] == needle

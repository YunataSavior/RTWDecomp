#!/usr/bin/env python3
"""Read values from the original executable by address.

    python tools/peek.py str ADDR
    python tools/peek.py dwords ADDR COUNT
    python tools/peek.py float ADDR
    python tools/peek.py double ADDR

Addresses are hex.
"""

import struct
import sys
from pathlib import Path

from pe import PEFile

ROOT = Path(__file__).resolve().parent.parent


def main():
    mode, address = sys.argv[1], int(sys.argv[2], 16)
    pe = PEFile(ROOT / "orig" / "RTW_STEAM" / "RomeTW.exe")
    if mode == "dwords":
        for i in range(int(sys.argv[3])):
            print(f"{address + 4 * i:#x} {struct.unpack('<I', pe.read(address + 4 * i, 4))[0]:#x}")
    elif mode == "float":
        print(struct.unpack("<f", pe.read(address, 4))[0])
    elif mode == "double":
        print(struct.unpack("<d", pe.read(address, 8))[0])
    elif mode == "str":
        data = b""
        while not data.endswith(b"\0"):
            data += pe.read(address + len(data), 1)
        print(repr(data[:-1].decode("latin1")))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

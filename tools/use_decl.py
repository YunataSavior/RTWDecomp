#!/usr/bin/env python3
"""Point the source's uses of a placeholder function at its decompiled one.

    python tools/use_decl.py OLD NEW HEADER [OLD NEW ...] [--dry]

For each OLD NEW pair (such as fn_00D93A0C strat_text), every .cpp and .h under src/ (other than HEADER and the .cpp
beside it) that names OLD loses its own declaration lines of OLD (a line ending in ";" that declares it, such as
"void* fn_00D93A0C(int text); // a guess"), has OLD replaced by NEW, and gains an #include of HEADER (a path from
src/code, such as localisation/string_table_manager.h) after its last #include "...". Each changed line is printed;
--dry prints them without writing. Casts or parameter types that no longer fit are left to fix by hand.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CODE = ROOT / "src" / "code"


def main():
    dry = "--dry" in sys.argv
    args = [arg for arg in sys.argv[1:] if arg != "--dry"]
    if len(args) < 3 or len(args) % 2 == 0:
        sys.exit(__doc__)
    header = args[-1]
    pairs = list(zip(args[0:-1:2], args[1:-1:2]))
    own = {(CODE / header).resolve(), (CODE / header).with_suffix(".cpp").resolve()}
    include = f'#include "{header}"'
    for path in sorted((ROOT / "src").rglob("*")):
        if path.suffix not in (".cpp", ".h") or path.resolve() in own:
            continue
        text = path.read_text(encoding="utf-8")
        if not any(re.search(rf"\b{old}\b", text) for old, _ in pairs):
            continue
        lines = text.split("\n")
        out = []
        for line in lines:
            new_line = line
            for old, new in pairs:
                if re.match(rf"^\s*[\w\s\*&:<>,]*\b{old}\s*\([^;{{}}=]*\)\s*;", line) and "return" not in line:
                    new_line = None
                    break
                new_line = re.sub(rf"\b{old}\b", new, new_line)
            if new_line != line:
                print(f"{path.relative_to(ROOT).as_posix()}: - {line.strip()}")
                if new_line is not None:
                    print(f"{path.relative_to(ROOT).as_posix()}: + {new_line.strip()}")
            if new_line is not None:
                out.append(new_line)
        if include not in out:
            last = max((i for i, line in enumerate(out) if line.startswith('#include "')), default=None)
            if last is None:
                print(f"{path.relative_to(ROOT).as_posix()}: no #include to add {header} after")
            else:
                out.insert(last + 1, include)
                print(f"{path.relative_to(ROOT).as_posix()}: + {include}")
        if not dry:
            path.write_text("\n".join(out), encoding="utf-8", newline="")


if __name__ == "__main__":
    main()

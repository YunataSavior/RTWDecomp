#!/usr/bin/env python3
"""python tools/cl.py VC6 OUTPUT SOURCE [FLAG...]: compile SOURCE to OUTPUT with Visual C++ 6.0's CL.EXE.

VC6 is the install's root (holding VC98 and Common) or its VC98 directory. CL.EXE runs with VC6's headers after the
project's include/, and needs MSPDB60.DLL from Common/MSDev98/Bin on PATH. VC6 has no dependency output, so OUTPUT.d
lists the project headers SOURCE includes, found by following its quoted and angled #include lines.
"""
import os
import re
import subprocess
import sys
from pathlib import Path

INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)


def vc98(root):
    for candidate in (root / "VC98", root):
        if (candidate / "Bin" / "CL.EXE").exists():
            return candidate
    raise SystemExit(f"{root}: no Visual C++ 6.0 here (VC98/Bin/CL.EXE); pass --vc6 to configure.py")


def headers(source, include_dirs):
    """The project headers SOURCE includes, directly or not."""
    found, pending = set(), [Path(source)]
    while pending:
        path = pending.pop()
        for name in INCLUDE.findall(path.read_text(errors="replace")):
            for directory in (path.parent, *include_dirs):
                candidate = directory / name
                if candidate.is_file() and candidate not in found:
                    found.add(candidate)
                    pending.append(candidate)
                    break
    return sorted(found)


def main():
    root, output, source, *flags = sys.argv[1:]
    if not root:
        raise SystemExit("Visual C++ 6.0 is not configured: python configure.py --vc6 <install root>")
    vc = vc98(Path(root))
    common = vc.parent / "Common" / "MSDev98" / "Bin"
    include_dirs = [Path("include")]
    env = dict(os.environ,
               PATH=os.pathsep.join([str(vc / "Bin"), str(common), os.environ.get("PATH", "")]),
               INCLUDE=os.pathsep.join([*map(str, include_dirs), str(vc / "Include"), str(vc / "MFC" / "Include")]),
               LIB=str(vc / "Lib"))
    Path(output).parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(vc / "Bin" / "CL.EXE"), "/nologo", "/c", *flags, f"/Fo{output}", source], env=env)
    if result.returncode:
        sys.exit(result.returncode)
    deps = " ".join(str(p).replace("\\", "/").replace(" ", "\\ ") for p in headers(source, include_dirs))
    Path(output + ".d").write_text(f"{output}: {deps}\n")


if __name__ == "__main__":
    main()

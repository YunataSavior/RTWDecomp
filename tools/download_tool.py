#!/usr/bin/env python3
"""python tools/download_tool.py TOOL OUTPUT: fetch a build tool.

  objdiff-cli   objdiff's command line (OUTPUT: the executable)
"""
import platform
import sys
import urllib.request
from pathlib import Path

OBJDIFF_TAG = "v3.8.1"


def objdiff_url():
    system = {"Darwin": "macos", "Linux": "linux", "Windows": "windows"}[platform.system()]
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "arm64": "arm64" if system == "macos" else "aarch64"}.get(machine, machine)
    suffix = ".exe" if system == "windows" else ""
    return f"https://github.com/encounter/objdiff/releases/download/{OBJDIFF_TAG}/objdiff-cli-{system}-{machine}{suffix}"


def main():
    tool, output = sys.argv[1], Path(sys.argv[2])
    output.parent.mkdir(parents=True, exist_ok=True)
    url = {"objdiff-cli": objdiff_url}[tool]()
    with urllib.request.urlopen(url, timeout=300) as response:
        output.write_bytes(response.read())
    output.chmod(0o755)


if __name__ == "__main__":
    main()

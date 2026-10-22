#!/usr/bin/env python3
"""Show objdiff's diff of one function, with only the differing rows.

    python tools/odiff.py UNIT SYMBOL [CONTEXT]

UNIT is as objdiff.json names it (code/romans_cmd_line). The original is on the left and the compiled unit on the
right; CONTEXT (2) rows around each difference are shown too.
"""

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    unit, symbol = sys.argv[1], sys.argv[2]
    context = int(sys.argv[3]) if len(sys.argv) > 3 else 2
    out = subprocess.run([str(ROOT / "build" / "tools" / "objdiff-cli.exe"), "diff", "-p", str(ROOT), "-u", unit,
                          "-o", "-", "--format", "json", symbol], capture_output=True, text=True).stdout
    diff = json.loads(out)

    def rows(side):
        for s in diff[side]["symbols"]:
            if s["name"] == symbol:
                return s.get("instructions", []), s.get("match_percent")
        return [], None

    left, left_percent = rows("left")
    right, right_percent = rows("right")
    print(f"target {left_percent}  ours {right_percent}")
    count = max(len(left), len(right))
    row = lambda side, i: side[i] if i < len(side) else None
    text = lambda r: r["instruction"]["formatted"] if r and "instruction" in r else ""
    kind = lambda r: r.get("diff_kind", "") if r else ""
    bad = [i for i in range(count) if kind(row(left, i)) or kind(row(right, i))]
    show = set()
    for i in bad:
        show.update(range(max(0, i - context), min(count, i + context + 1)))
    last = -1
    for i in sorted(show):
        if i != last + 1:
            print("   ...")
        mark = "!" if i in bad else " "
        print(f"{mark} {text(row(left, i))[:58]:58} | {text(row(right, i))[:58]}")
        last = i


if __name__ == "__main__":
    main()

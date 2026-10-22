#!/usr/bin/env python3
"""Find names for one scope's locals that VC6 gives the original's slot order.

    python tools/vc6_names.py --decl a,b,c --slots c,a,b role=name1,name2 ... [--count N]

--decl gives the scope's locals (roles) in declaration order, --slots the order the original gives them from the
highest slot down (tools/slotmap.py shows it). Each role=... lists candidate names, best first; a role without one
keeps its own name. Prints the N (5) assignments with the best-placed candidates. See tools/vc6_locals.py for the
rule: slots go bucket by bucket, and within a bucket the last declared first.
"""

import argparse

from vc6_locals import bucket, layout


def solve(decl, slots, candidates, count):
    position = {role: i for i, role in enumerate(decl)}
    found = []

    def search(k, chosen, cost):
        if k == len(slots):
            found.append((cost, dict(chosen)))
            return
        role = slots[k]
        for i, name in enumerate(candidates[role]):
            if name in chosen.values():
                continue
            if k:
                before = slots[k - 1]
                b0, b1 = bucket(chosen[before]), bucket(name)
                if b1 < b0 or (b1 == b0 and position[before] < position[role]):
                    continue
            chosen[role] = name
            search(k + 1, chosen, cost + i)
            del chosen[role]

    search(0, {}, 0)
    found.sort(key=lambda x: x[0])
    results = []
    for cost, chosen in found[:count]:
        names = [chosen[r] for r in decl]
        assert [decl[names.index(n)] for n in layout(names)] == slots
        results.append(chosen)
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--decl", required=True)
    parser.add_argument("--slots", required=True)
    parser.add_argument("--count", type=int, default=5)
    parser.add_argument("candidates", nargs="*")
    args = parser.parse_args()
    decl, slots = args.decl.split(","), args.slots.split(",")
    candidates = {role: [role] for role in decl}
    for item in args.candidates:
        role, names = item.split("=", 1)
        candidates[role] = names.split(",")
    results = solve(decl, slots, candidates, args.count)
    if not results:
        print("none: add candidates")
    for chosen in results:
        print("  ".join(f"{role}={chosen[role]}({bucket(chosen[role])})" for role in decl))


if __name__ == "__main__":
    main()

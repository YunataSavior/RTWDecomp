#!/usr/bin/env python3
"""Predict where VC6 puts a function's locals on the stack, from their names.

    python tools/vc6_locals.py NAME...

prints each name's bucket and the order VC6 gives the names (declared in the order given, in one scope), from
[ebp-4] down.

VC6 keeps a scope's locals in a 16-bucket hash table and gives out stack slots bucket by bucket, from bucket 0, and
within a bucket the last declared first. Outer scopes get their slots before inner ones. Compiler temporaries ($T...)
follow all the named locals, in the order they are created, not by hash.
"""

import argparse


def bucket(name: str) -> int:
    """The bucket of a local's name (found by experiment: it predicts every one of 600 random names)."""
    h = 0
    for c in name.encode():
        h = ((h << 2) + (h >> 4) + c) & 0xFFFFFFFF
    return (h ^ (h >> 16)) & 15


def layout(names: list[str]) -> list[str]:
    """Names declared in this order, in one scope, in slot order (the first gets the highest address)."""
    return sorted(names, key=lambda n: (bucket(n), -names.index(n)))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("names", nargs="+", help="locals of one scope, in declaration order")
    args = parser.parse_args()
    for name in args.names:
        print(f"{bucket(name):2}  {name}")
    print("slot order:", " ".join(layout(args.names)))


if __name__ == "__main__":
    main()

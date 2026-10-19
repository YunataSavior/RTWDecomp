RTWDecomp
=========

A work-in-progress decompilation of `RomeTW.exe`, the Windows/x86 executable of Rome: Total War Gold (Steam).

Supported versions:

- `RTW_STEAM`: Rome: Total War Gold, Steam (`RomeTW.exe`, SHA-1 `bf5fb6e7b1cfe6ac94ebaef5c08ff1dfb2bf8812`)

The game was built with **Microsoft Visual C++ 6.0 Service Pack 6**: its Rich header records compiler build 9782, the
build of SP6's `C1XX.DLL` and `C2.DLL` (for 1064 of its C++ objects). It links the C runtime statically, and its
runtime is SP6's `LIBCMT.LIB` (every function found matches it exactly). Its game code is compiled with `/MT /Os
/Ob1 /Op /G6`, without `/GX`: `/Op` shows in float conversions going through memory, and `/G6` in zero-extensions
written `xor eax, eax; mov al, cl` rather than `movzx`. It has no relocation table, debug symbols or RTTI names, but its
asserts name about 1,100 of its source files, so the source tree is known.

The project follows [decomp-toolkit](https://github.com/encounter/decomp-toolkit)'s conventions (`symbols.txt` and
`splits.txt`, one split object per unit, an object list in `configure.py` as in
[dtk-template](https://github.com/encounter/dtk-template), [objdiff](https://github.com/encounter/objdiff) for diffing
and progress), so it should feel familiar from GameCube and Wii projects. decomp-toolkit itself reads only PowerPC
executables, so `tools/split.py` does its splitting for the PE, reusing `tools/pe.py` from
[mwcc](https://github.com/rayanht/mwcc).

Dependencies
============

- Windows (Visual C++ 6.0 runs natively)
- Python 3.11+
- [ninja](https://github.com/ninja-build/ninja)
- Visual C++ 6.0 with Service Pack 6, to compile sources (not needed to split or diff). It is not downloaded; see
  below.

Setting up Visual C++ 6.0 SP6
-----------------------------

The compiler only needs to be copied, not installed:

1. From a Visual C++ 6.0 disc, copy `VC98\` and `COMMON\MSDEV98\BIN\` (which holds `MSPDB60.DLL`) into a directory
   of your choice, keeping those paths.
2. From the Visual Studio 6.0 Service Pack 6 disc (MSDN disc 0773.2), copy `VC98\BIN`, `VC98\INCLUDE`, `VC98\LIB`,
   `VC98\CRT`, `VC98\MFC` and `VC98\ATL` over them.
3. SP6's code generator is on that disc as `OS\SYSTEM\MSVCEP.DLL`: copy it to `VC98\BIN\C2.DLL`. (Without it, SP6's
   compiler stops with error C1900, an IL version mismatch. `MSSE.DLL` is the Standard Edition's, which does not
   optimize.)

`CL.EXE` should then report version 12.00.8804, and the objects it writes carry `@comp.id` build 9782. Visual C++
6.0's first release (12.00.8168) will not do: its code differs (it keeps a temporary SP6 drops, in
`CharacterRecord::set_trait`), and its libraries lack the functions SP5 added.

Building
========

1. Copy `RomeTW.exe` from your copy of the game to `orig/RTW_STEAM/`. Never commit it (`.gitignore` ignores it).
2. Set up Python and configure:

   ```sh
   python -m venv .venv
   .venv\Scripts\pip install -r requirements.txt
   .venv\Scripts\python configure.py --vc6 E:\path\to\VC6_SP6
   ninja
   ```

   `--vc6` is the directory holding `VC98` (or set `VC6_DIR`). Without it, `ninja` still splits the original, and
   only units with a source fail to compile.
3. `ninja` ends by printing the progress, from objdiff's report (`build/RTW_STEAM/report.json`, which `ninja report`
   writes alone). Data is not split per unit yet, so only code is counted.

`build.ninja` reruns `configure.py` when it or the config files change. Rerun it yourself after adding a source file.

Working on a unit
=================

`configure.py` lists every translation unit in `LIBS`, with its status:

- `Matching`: its source matches. `ninja` checks each of its functions against the original (`tools/check.py`: the
  same bytes and the same relocations, by symbol name) and fails when one differs. A relocation to a string literal
  or another file static matches by its bytes, as the compiler names literals `$SG<n>`, and a call to a function the
  linker folded into another (`config/RTW_STEAM/folded.txt`) matches the one it was folded into.
- `NonMatching`: its source, if it has one, is compiled for objdiff but not checked.
- `Equivalent`: as `NonMatching`, for a source that behaves as the original but does not match it.

A unit is built once it is located in the original (in `config/RTW_STEAM/splits.txt`). To work on one:

1. Create `src/<unit>` (for example `src/code/army.cpp`) and rerun `configure.py`.
2. Open the project in objdiff and match its functions. Name each function in `config/RTW_STEAM/symbols.txt` by the
   name the compiler gives it (`_name` for C, the mangled name for C++, `_name@N` for `__stdcall`), so that objdiff
   and `tools/check.py` pair it with the compiled one.
3. When it matches, change its status in `configure.py` to `Matching`.

Declarations whose real header is not known yet go in `include/unsorted.h`. A function or variable with no name yet
can be declared `extern "C"` by the name the splitter gives it (`fn_00EFA483`, `lbl_012CA6B0`).

Matching tips
-------------

[`MATCHING.md`](MATCHING.md) collects what has been found about matching VC6's output: the patterns its code shows,
which source gives each, and how it lays out a function's locals on the stack.

Project structure
=================

- `configure.py`: the units, their status and flags; writes `build.ninja` and `objdiff.json`.
- `config/RTW_STEAM/config.json`: the original executable, its SHA-1 and the default compiler flags.
- `config/RTW_STEAM/symbols.txt`: the original's symbols, in decomp-toolkit's format.
- `config/RTW_STEAM/splits.txt`: which address ranges belong to which unit, in decomp-toolkit's format. `.bss` is
  the uninitialized tail of the original's `.data`.
- `config/RTW_STEAM/folded.txt`: functions the linker folded together (`/OPT:ICF` kept one copy of identical code,
  mostly a template's functions for different types), for `tools/check.py`.
- `src/code/`: the game, as `...\Dev\barbi_daily\code\` was.
- `src/libs_daily/`: CA's libraries (`ca_libs`, `cas_loader`, `cas_render`, `gs_online2`), as `libs_daily\` was.
- `include/`: headers that are not CA's, and stand-ins for CA's whose names are not known (`unsorted.h`,
  `common_constants.h`).
- `tools/split.py`: splits the original into one COFF object per unit (see below).
- `tools/locate_units.py`: finds units' code from the source paths their asserts push, and adds them to `splits.txt`
  and their functions to `symbols.txt` (see below).
- `tools/fill_splits.py`: splits the rest of `.text` into units, from where each unit's code starts and the link order
  of the source paths in `.data` (see below).
- `tools/match_libs.py`: finds the objects of static libraries (VC6's `LIBCMT.LIB`, `LIBCPMT.LIB`, `LIBCIMT.LIB`) in
  `.text`, and names their units and functions (see below).
- `tools/check.py`: checks a compiled unit against its split.
- `tools/cl.py`: runs VC6's `CL.EXE` with its environment, and writes the header dependencies VC6 cannot.
- `tools/vc6_locals.py`: predicts the stack order VC6 gives locals from their names (see
  [`MATCHING.md`](MATCHING.md#locals-and-the-stack)).
- `tools/ghidra/ExportSymbols.py`: a Ghidra script writing the program's functions and named data in `symbols.txt`
  format.

How units are located
---------------------

The game's asserts and logging push their source file's path (`E:\Perforce\Rome\Rome,BI-1.5,1.6\Dev\barbi_daily\code\
army.cpp`). `tools/locate_units.py` takes each function (a `push ebp; mov ebp, esp` prologue up to the next) that
pushes a `.cpp` or `.c` path as part of that unit, and the unit's `.text` as the range from the first such function to
the end of the last, since VC6 links each object's code contiguously. 405 units are located this way, with no
overlaps. A unit's functions before its first assert or after its last are not included yet: extend its range in
`splits.txt` when you find them.

The other units have no asserts in their code, but most still hold their path in `.data` (unreferenced, like MFC's
`THIS_FILE`), which marks where their data sits in link order. `tools/fill_splits.py` places them, and the code
before and after each located unit's asserts:

- Where a unit starts: CA's units all include a header that defines Unicode character constants (`0`, `0xFEFF`,
  `' '`, `'_'`, ...) as statics with dynamic initializers, so each unit's code opens with their initializers. In the
  C++ initializer table (`__xc_a` to `__xc_z`, in link order), the one that clears a word followed by the one that
  stores `0xFEFF` is a unit's first function: 1116 of them.
- Which unit it is: the linker lays out each object's `.text` and `.data` in the same order, so the source paths in
  `.data` are the link order of the units they name (the 409 units located by their asserts are in the same order in
  both). Between two located units, the code from one unit start to the next is matched in order to the units named
  between theirs in `.data`, favouring a one-to-one pairing (most gaps have exactly as many starts as names) and the
  units whose `.data` the code refers to.
- The game's own code is not padded and the libraries linked after it are aligned to 16 bytes, so the first padding
  after the game's last unit ends the game's code.

A located unit grows over the rest of its code, except a Matching unit, whose source would no longer match. Code no
name is matched to (a unit without a path, or a library that does not include CA's header: libpng, zlib, NvTriStrip,
D3DX) becomes an `auto/<category>/<address>.cpp` unit, which `configure.py` adds by itself. It prints what it could
not place.

The C runtime (`0x00FC4FA0` to `0x00FDE2C0`) is VC6's `LIBCMT.LIB`, with `LIBCPMT.LIB` (`std::string`, the locks)
and `LIBCIMT.LIB` (the old `<iostream.h>` streams). `tools/match_libs.py` finds each of their objects there:

```
python tools/match_libs.py RTW_STEAM LIBCMT.LIB LIBCPMT.LIB LIBCIMT.LIB --start 0x00FC4FA0 --end 0x00FDE2C0 --write
```

- The libraries are built with function-level linking, and the linker drops the functions nothing calls, so an
  object is found as a chain of its functions in their order, each on its alignment. Their bytes are compared with
  the bytes the relocations fix up left out.
- Each relocation is then checked: what it points to in the original must agree with where the objects already
  placed put that symbol, and with the import table. That tells apart objects whose code is the same but for what it
  calls (`fprintf` calls `_output`, `fwprintf` `_woutput`; `_chmod` calls `GetFileAttributesA`, `_wchmod`
  `GetFileAttributesW`).
- A function that has changed since the libraries' version is placed by its symbol where the placed code points at
  it (`_free`, `__output`), or by its likeness beside the rest of its object.
- Each unit is named for the source file its object's `.file` symbol records (`crt/output.c`,
  `crt/i386/ftol.asm`, `crt/frame.cpp`), and `configure.py` adds the `crt/` units by itself. The functions found are
  named in `symbols.txt`, with their sizes when found exactly.

Run against Service Pack 6's libraries (VC6's first release's lack SP5's SSE floating-point emulation, and their
heap, `_output` and `_free` differ), every function found matches exactly: 259 objects, and 0x2B bytes no object is
found for, left in an `auto/crt/` unit.

How splitting works
-------------------

With no relocation table, `tools/split.py` finds references by disassembling each function with
[capstone](https://www.capstone-engine.org/): `call`/`jmp`/`jcc` rel32 targets outside the function, and 32-bit
displacements and immediates that land in the image. Jump and index tables (which VC6 places after a function's code)
and data are scanned for aligned pointers into the image. Each reference becomes a relocation to the symbol containing
its target, or to `__imp__Name` for an import slot, or to a generated `_fn_XXXXXXXX`/`_lbl_XXXXXXXX`. Unnamed targets
show in objdiff as those names until `symbols.txt` names them.

Known limits:

- An immediate that only looks like an address (a constant between `0x00401000` and `0x02B56000`) becomes a false
  relocation.
- Import names carry no `__stdcall` suffix (`__imp__ExitProcess`, not `__imp__ExitProcess@4`), because the import
  table does not record it. Name the slot in `symbols.txt` to match a compiled reference.
- Only `.text` is split per unit so far; units' data is not.
- `fill_splits.py`'s ranges are only as good as its evidence: a unit with neither a path nor CA's header is merged
  into the unit before it, and a function that only looks like a function start (a `push ebp; mov ebp, esp` in a
  jump table, say) splits a function in two.

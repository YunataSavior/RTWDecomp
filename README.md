RTWDecomp
=========

A work-in-progress decompilation of `RomeTW.exe`, the Windows/x86 executable of Rome: Total War Gold (Steam).

Supported versions:

- `RTW_STEAM`: Rome: Total War Gold, Steam (`RomeTW.exe`, SHA-1 `bf5fb6e7b1cfe6ac94ebaef5c08ff1dfb2bf8812`)

The game was built with **Microsoft Visual C++ 6.0 Service Pack 6**: its Rich header records compiler build 9782, the
build of SP6's `C1XX.DLL` and `C2.DLL`. It links the C runtime statically. Its game code is compiled with `/MT /Os
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

`CL.EXE` should then report version 12.00.8804, and the objects it writes carry `@comp.id` build 9782.

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

- Small inline functions (accessors, `max`) are expanded but leave their temporaries in place, one level of nesting at
  a time: an accessor that stores its result in a temporary before use is usually an inline calling another inline
  (`num_regions()` returning `m_regions.size()`), and `max` taking `const int&` keeps both arguments in slots.
- Rounding to int is the inline assembly `push eax; fld x; frndint; fistp [esp]; pop eax` (`float_to_int` in
  `include/unsorted.h`), not a cast.
- Float literals are COMDAT constants named by their 80-bit value (`__real@4@3fff8ccccd0000000000` is 1.1f); name the
  original's in `symbols.txt` the same way.
- The game redefines `for` as `switch (0) case 0: default: for`, to give loop variables standard scope
  (`include/for_scope.h`). That is the `and [ebp-X], 0` into a slot never read that starts nearly every loop.
- `cmp x, y; sbb eax, eax; neg eax` is an unsigned `x < y` turned into a bool (`m_18 < 0xffffffff`), not `!=`.
- A value copied through a temporary right after a call (`call; mov [ebp-X], eax; mov eax, [ebp-X]`, then a field
  read) is an inline called on the call's result.
- `__FILE__` must be the original's path and `__LINE__` its line, since the game's allocator and asserts push both.
  A source starts with `#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\road.cpp"` (keeping
  its own line numbers), and a `#line N` before a statement gives it the original's line.
- VC6 expands template functions defined outside their class as it does inline ones. It does not expand a function
  holding a `switch` (so any loop written with the `for` macro), and stops expanding one that calls an inline with a
  local or a by-value parameter of its own after it has given it stack slots. Those slots stay unused, so unused
  slots in a frame mean a call to such a function (`Array::~Array`, `Array::add_unique`).
- `b = x != -1;` zero-extends (`xor eax, eax; cmp; setne al`), `b = x != -1 ? true : false;` does not.
- An inline `bool` function written `if (a && b) return true; return false;` keeps its result in a byte; `return a
  && b;` keeps it in an int.
- Assigning straight to a bitfield (`tile->m_road_level = level`) copies a `char` value through a byte temporary; an
  inline setter taking the value does not.
- `if (x) return;` before a last call compiles to `je; jmp`, unlike `if (!x) f();`.
- Every game file starts with the same 36 thunk/initializer pairs: they initialize the file's own copies of the
  constants in `include/common_constants.h`, which `unsorted.h` includes. Each initializer uses three numbers of the
  counter that names `$E<n>` initializers and `$S<n>` guards, so the unit's own are numbered after them: take the
  numbers from the compiled object. The unit's copies of the constants (in its `.bss`, and the `const float`s in its
  `.rdata`) are named in `symbols.txt` with `scope:local`.

Locals and the stack
--------------------

VC6 does not lay out a function's locals in the order they are declared, so two sources that compile to the same code
can differ in their stack offsets, and a function's locals can need names that put them where the original has them.
The rules, found by experiment:

- Each scope's named locals are kept in a 16-bucket hash table. Slots are given out bucket by bucket from bucket 0
  (from `[ebp-4]` down), and within a bucket the last declared first. The bucket is
  `h = (h << 2) + (h >> 4) + c` over the name's characters, then `(h ^ (h >> 16)) & 15`.
- Outer scopes get their slots before inner ones; a `case` block or loop body is a scope of its own.
- Compiler temporaries come after all the named locals, in the order the compiler creates them: first those the front
  end makes while parsing (an argument bound to a `const T&`, a class returned by value), then those made when inlines
  are expanded, then code generation's (a `switch` value, conversions, `?:` and `&&` results).

`tools/vc6_locals.py` prints names' buckets and the order they would get. To fix a function's layout, map its slots
to its locals (by masking stack offsets and diffing), then choose names per scope that the tool puts in the
original's order.

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
- `tools/check.py`: checks a compiled unit against its split.
- `tools/cl.py`: runs VC6's `CL.EXE` with its environment, and writes the header dependencies VC6 cannot.
- `tools/vc6_locals.py`: predicts the stack order VC6 gives locals from their names (see above).
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
`THIS_FILE`), which marks where their data sits in link order. Placing them from that is still to do.

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
- Only units in `splits.txt` count toward progress, so the totals cover the located part of the executable.

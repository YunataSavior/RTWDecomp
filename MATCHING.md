Matching notes
==============

What has been found about matching Visual C++ 6.0's output, by experiment. See the [README](README.md) for how the
project is built and how a unit is worked on.

Tips
----

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
- How many slots a partly expanded function leaves depends on how deep the call is: `ObserverList::notify` leaves
  `LinkedList::pop_back`'s first `CA_DELETE` slots but not its second only when it calls `pop_back` through another
  inline (`remove_last`).
- A field address kept in a slot before it is read (`add eax, 0x14; mov [ebp-X], eax; mov eax, [ebp-X]`) can be a
  named reference local (`const String& name = description->name();`), placed by its name's bucket.
- `b = x != -1;` zero-extends (`xor eax, eax; cmp; setne al`), `b = x != -1 ? true : false;` does not.
- An inline `bool` function written `if (a && b) return true; return false;` keeps its result in a byte; `return a
  && b;` keeps it in an int.
- Assigning straight to a bitfield (`tile->m_road_level = level`) copies a `char` value through a byte temporary; an
  inline setter taking the value does not.
- `if (x) return;` before a last call compiles to `je; jmp`, unlike `if (!x) f();`.
- Every game file starts with the same thunk/initializer pairs: they initialize the file's own copies of the
  constants in `include/common_constants.h`, which `unsorted.h` includes. Each initializer uses three numbers of the
  counter that names `$E<n>` initializers and `$S<n>` guards, so the unit's own are numbered after them: take the
  numbers from the compiled object. The unit's copies of the constants (in its `.bss`, and the `const float`s in its
  `.rdata`) are named in `symbols.txt` with `scope:local`.
- Most files have 36 of those pairs. The last three come from other headers:
  - Files with all 36 include `include/map_constants.h` after `unsorted.h`.
  - Files with the first 35, like `landmark_db.cpp`, include `include/angle_constants.h` instead.
  - Files with only the first 33, like `character_traits_db.cpp`, include neither.
- The destructor that `atexit` runs for a static class member checks and sets a `$S` guard bit first (`movzx eax,
  byte ptr [$S]; and eax, 1; ...`); one for a file-scope static or global does not. So a guarded one means a static
  member.
- VC6 expands an array's constructor and destructor loops (count, size and pointer in slots) into the code that builds
  it. Separate `thiscall` functions holding those loops (the constructor returning `this`) are the implicit
  constructor and destructor of a class whose member is the array (`LandmarkDescriptions`).
- An inline operator taking `const T&` stores each constant operand in a slot first (`mov [ebp-X], 0x800`); with
  by-value enum operands, VC6 folds the expression.
- A `this` kept in a slot before a call whose result is stored through it (`mov [ebp-X], obj; call f; mov ecx,
  [ebp-X]; mov [ecx+8], eax`) is an inline member making that call (`load_model`), not an assignment of its result.
- A copy constructor that first calls another class's constructor on `this` and then assigns each item is a derived
  class's own copy constructor; the base holds the array, and its implicit constructor is the call
  (`FixedArray`/`FixedArrayBase` in `character_traits_db.h`). An implicit copy of a class whose items have no copy
  constructor of their own is a `rep movsd` instead.
- An uninitialized slot far down the frame whose address is passed (`lea eax, [ebp-0xcc]; push eax`) is a local of an
  expanded inline function, not a named local of the function itself.
- VC6 builds a class returned from an inline (`return read_religion(reader, ok);`) straight into the caller's
  temporary only when the class has a destructor; otherwise it copies it. In `a.x = f()` it also evaluates `a.x`
  after the call when `f` is an inline, and before it otherwise.
- VC6 does not try to expand a constructor of more than four initializers once a member has a destructor, so a member
  type with a destructor can cost a containing class's constructor its unused slots elsewhere (`ReligionValue`).
- Out-of-class template members with locals (`BitSet::set`) leave their locals' slots where VC6 tries and fails to
  expand them.
- VC6 expands at most 8 nested inline calls by default (`#pragma inline_depth`). When a deeply nested destructor
  leaves fewer unused slots than the original and no other source fits, raise the depth
  (`character_traits_db.h`).
- Unused slots in a caller can also come from a header inline that VC6 failed to expand, whose out-of-line copy is
  elsewhere in the game (`UniString::operator==(const char*)` at 0x00422DE4). Compile candidate bodies on their own
  until the copy's bytes match the original's; the right body then leaves the right number of slots in its callers.
- A small function the original calls where VC6 would expand an inline (a fill loop with no slots left behind) is
  an ordinary member defined elsewhere (`ReligionTotals::fill`), not an inline.
- A call's result kept in a slot only to be pushed (`call; mov [ebp-X], eax; push [ebp-X]`) is the argument of an
  inline taking `const T&`: a variable passed there is used directly, a call's result is not (`UniString::operator+=`
  wrapping `append`).
- A value read into a slot and loaded only when stored (`call read; mov eax, [i]; mov ecx, this; mov edx, [ebp-X]`)
  is an inline returning a reference to its own local (`SaveStream::read_int_ref`); one returning by value loads it
  first.
- A virtual call whose adjusted `this` gets a slot after every other temporary (`add eax, 0xc; mov [ebp-X], eax`) is
  made through a base class at that offset (`Army`'s `ArmyOwner`, `Character`'s `CharacterKind`), not a member.
- `cmp; jb X; jmp Y` (or `je X; jmp Y`) where one jump is expected is an `if` with an empty branch:
  `if (version >= 8) {} else ...`, `if (a && b) {}`.
- `bool b = x == y ? true : false;` compiles to `cmp; sete al`; without the `?:` it is `sub; neg; sbb; inc`.
- `NEW T` pushes `sizeof(T)` and an array index is scaled by the item size, which give classes' sizes (`Army`, `Tile`).
- VC6 expands only so many inline calls per function, by the size of the caller's code. A function calling one inline
  many times expands the first few and calls the rest (`HashMap::insert`'s `TextEqual` in `triggers.cpp` expands 4
  of 12), so a source with the right shape but less code expands too many.
- An inline that VC6 tries and fails to expand still gets slots for its parameters and locals. So a frame's unused
  slots can show that a function is an inline (`TextHash`, `HashMap::end`), even when the original calls its
  out-of-line copy.
- An empty inline destructor (`~iterator() {}`) is enough for VC6 to build a class returned by value straight into
  the variable it initializes; without one, it builds a temporary and copies it with `rep movsd`.
- A byte read with `movzx` (`movzx eax, byte ptr [ecx]`) is an `unsigned char`; VC6 reads a `char` with `movsx`.
- A float constant in the unit's `.rdata` that isn't a `__real@` COMDAT is a `static const float`
  (`k_trait_chance_scale` in `triggers.cpp`). Declare it at file scope: a local static's mangled name holds a scope
  number that changes when the function is restructured. Name it in `symbols.txt` with `scope:local`.
- For an assembly listing of a unit, add `/FA /Fa<file>.cod` to its `tools/cl.py` command line. `/FAs` (with the
  source) fails on the `#line` paths.
- After `NEW T`, a constructor call followed by a reload of the allocation's temporary (`call ??0X; mov eax, [ebp-X]`)
  is an inline constructor of `T` that calls a member's or base's out-of-line one (`SavedPointers` and its
  `SavedPointerTable` in `romans_file.cpp`); when `T`'s own constructor is the call, its result is used directly.
- `xor eax, eax; and eax, 1; test eax, eax; je; push p; call ??3` in `debug_delete`'s loop is the scalar deleting
  destructor, expanded with flags 0, of a class with an empty inline destructor (`~SavedPointers() {}`). With no
  destructor at all, VC6 expands `debug_delete` into its caller.
- A search loop that tests its match in the loop's condition (`while (e->m_key && (e->m_key != key || e->m_type !=
  type))`) compiles to one jump per test; testing it in the body and `break`ing adds a `jne; jmp` pair.
- An `Array` loop whose start iterator takes one temporary (`mov eax, [this+X]; mov [ebp-A], eax; mov eax, [ebp-A];
  mov [ebp-B], eax`) is built from `data()` (`iterator it = m_neighbours.data()`); from `begin()` it takes two.
- A function whose callers read only `al` (`call; movzx eax, al`) but whose body returns another function's `int`
  unchanged returns `unsigned char`: the narrowing costs no code (`StrategyMap::unk_00471495` in `world_map.cpp`).
- An argument passed by value from a slot (`push [ebp-X]; push [ebp-Y]`) rather than through the constructor's
  returned pointer (`push [eax+4]; push [eax]`) is a named local (`RoadPoint position(index);`), not a temporary.
- A file's run of header initializers (the 36 thunk/initializer pairs above) marks where it starts, even when none of
  its code pushes its `__FILE__`: `world_map.cpp` starts with its run at 0x0046D364, well before its first path.
- `cmp; ...; mov byte ptr [t], 1` then `movzx eax, byte ptr [t]; neg eax; sbb eax, eax; inc eax` is `a != b` through an
  inline `operator!=` built on `operator==`; `!(a == b)` tests the byte directly.
- `movzx eax, byte ptr [x]; neg eax; sbb eax, eax; neg eax` before a test is a byte converted to `bool`
  (`(bool)occupant->m_84`).
- Many `return 0`s that jump to one shared `xor eax, eax` before `leave` are nested `if`s whose failures fall through
  to a final `return 0;`; a `return 0;` written in place compiles to its own `xor eax, eax; jmp`.

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
  - So a pointer stored to a slot below the temporaries, then loaded into `ecx` for a call, is an expanded inline's
    `this`, not a named local (`Character::start_turn()` in `faction.cpp`).
- Sibling loops don't share slots: each `for (unsigned int i = 0; ...)` gets its own `i`. Loops that all use one slot
  have their variable declared once, before them.

`tools/vc6_locals.py` prints names' buckets and the order they would get. To fix a function's layout, map its slots
to its locals (by masking stack offsets and diffing), then choose names per scope that the tool puts in the
original's order.

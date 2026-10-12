# Character traits: how they're read, and why attributes drift after trait removal

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short version:** when a trigger's negative `Affects` lowers a trait's level, the trait drops (or disappears), but **the old level's effects stay in the character's stored attributes**, permanently and in the save (confirmed in game). Age plays no part (NYD). Removing a trait through its `AntiTraits` (confirmed in game), or with the `give_trait` console command (NYD), keeps the attributes correct.

Sources:

- The decompiled trait database, [`src/code/character_traits_db.cpp`](../../src/code/character_traits_db.cpp) and [`character_traits_db.h`](../../src/code/character_traits_db.h), matching the original.
  - It reads `export_descr_character_traits.txt` into the definitions below.
- The decompiled character record, [`src/code/character_record.cpp`](../../src/code/character_record.cpp) and [`character_record.h`](../../src/code/character_record.h), matching the original (`0x58E173`–`0x594493`).
  - It holds the code that changes a character's traits, ancillaries and attributes.
- The decompiled triggers, [`src/code/triggers.cpp`](../../src/code/triggers.cpp) and [`triggers.h`](../../src/code/triggers.h).
  - They read the `Trigger` blocks and fire them when their event happens.
  - All but two of its functions match the original; the two left (a hash table's insert and `free_triggers`) play no part in these findings.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the other code that calls it, which isn't decompiled yet (NYD).
  - That includes the console commands and the character scroll.
- In-game tests (see [Confirmed in-game](#confirmed-in-game)).

Addresses are virtual addresses in that exe (image base `0x400000`). Barbarian Invasion runs `RomeTW-BI.exe`, which wasn't examined.

## How `export_descr_character_traits.txt` is read

`load_traits()` (`0x8E3F49`) opens the file, reads every `Trait` block (`read_traits`, `0x8E404A`), then hands the same reader to the trigger parser (`0x8AA428`). So the triggers must follow the traits in the same file. Texts come from `export_VnVs.txt`, through its lookup file `export_Descr_VnVs_enums.txt`.

A trait's index is its position in the file. Per `Trait` block, in this order:

| Line | Required | What the parser does |
|---|---|---|
| `Characters` | Yes | Reads **one** character type, or `all` |
| `Hidden` | No | Sets the hidden flag |
| `ExcludeCultures` | No | Reads any number of cultures; an unknown one is a script error |
| `NoGoingBackLevel` | No | Reads an integer |
| `AntiTraits` | No | Reads any number of trait names, resolved after the whole file is read |
| `Level` | No | Starts a level, up to 9 of them; the lines below belong to it |

Each `Level` reads, in order:

- `Description` and `EffectsDescription`, which are required.
- `GainMessage`, `LoseMessage` and `Epithet`, which are optional.
- `Threshold`, which is required.
- Then any number of `Effect`, `Religious_Belief` and `Religious_Order` lines.
  - An unknown attribute name in `Effect` is a script error.

Things the parser does that the file format doesn't make obvious:

- **`Characters` takes only the first type.** For `Characters spy, assassin` the reader stops at the comma, so the trait is defined for spies only.
  - In vanilla RTW that's `GoodConspirator` and `BadConspirator`. This comes from the code; it wasn't checked in game.
- **Every trait has a hidden level 0** with threshold 0, ahead of the `Level` blocks (which become levels 1 and up).
- **At most 9 `Level` blocks.** The definition holds 10 levels counting level 0, and the error message ("Max is 10") counts that one.
  - The check comes after a level is stored, so a 10th `Level` block writes past the table before the error is reported.
- **`AntiTraits` names aren't checked.** Each name is looked up by `find_trait(String)` once all traits are read, and a name that matches no trait still takes a slot, as a null entry.
  - The lookup is case-sensitive.
  - There's room for 20 names and no bounds check.
- The error for a missing `EffectsDescription` says "when expecting Description", copied from the line before.

### Looking traits up by name

There are two lookups, with different case rules:

- `find_trait(const char*)` (`0x8E4C92`) ignores case (`_stricmp`). The triggers use it, and so does other game code (NYD).
- `find_trait(String)` (`0x8E4CF9`) is case-sensitive. The `give_trait` and `give_trait_points` console commands (`0xDE405D`, `0xDE4BEC`; NYD) and the `AntiTraits` resolution use it.

## Data layout

A trait definition (`TraitDefinition`, 0x2d4 bytes; the parser's `TraitDescription` adds the anti-trait names after it):

| Offset | Member | Meaning |
|---|---|---|
| `+0x000` | `m_index` | Its position in the file |
| `+0x004` | `m_name` | Its name |
| `+0x00c` | `m_levels[10]` | Its levels, 0x3c bytes each (below) |
| `+0x264` | `m_num_levels` | The level count, counting level 0 |
| `+0x268` | `m_anti_traits[20]` | Its anti-traits, as pointers to their definitions |
| `+0x2b8` | `m_num_anti_traits` | |
| `+0x2bc` | `m_characters` | The character types that can have it, a bit each |
| `+0x2c4` | `m_no_going_back_level` | |
| `+0x2c8` | `m_exclude_cultures` | A bit per culture |
| `+0x2d0` | `m_hidden` | |

A level (`TraitLevel`, at `+0x00c + level * 0x3c`):

| Offset | Member |
|---|---|
| `+0x00` | `m_level`: the level number |
| `+0x04`–`+0x14` | Its texts: name, description, epithet, gain message, lose message |
| `+0x18` | `m_threshold`: the points it takes |
| `+0x1c` | `m_effects`: an array of {attribute, value} pairs |
| `+0x28` | `m_religious_belief` |
| `+0x30` | `m_religious_order` |
| `+0x38` | `m_effects_description` |

A character's trait entry (`CharacterTrait`) holds the definition at `+0`, a pointer to its current level at `+4`, and its points at `+8`.

## How attributes are stored

- A character keeps a 55-slot attribute array at `char+0x2c + id*4`.
  - The attribute IDs come from the name-to-ID function `0xE41B13`, which the parser also uses for `Effect` lines: `Command` is 0, `Influence` 1, `Management` 2, `Subterfuge` 3, and so on (NYD).
  - 55 means an unknown name.
- The array is **saved in the savegame and loaded back as-is** (`CharacterRecord::load`, `0x5933E8`). It's never rebuilt from the current traits and ancillaries.
  - It changes only through `set_attribute(id, value)` (`0x590B15`), which the trait and ancillary code calls with `current ± level effect`.
- The character scroll's getters:
  - Management (`0x49C5B6`) and Influence (`0x4D496D`) return the stored value, clamped to 0–10 (NYD).
  - Command (`0x590EC6`) returns the stored value plus battle-situation modifiers, clamped to 0–10.
  - None of them read age (NYD for Management and Influence).
- `descr_strat.txt`'s `character_record` entries accept base `command`, `influence`, `management`, `subterfuge` and `age` values (parsed at `0x449A1A`–`0x449D08`; NYD).
  - They're range-checked and then **discarded**: the create call at `0x449EF4` never receives them.

## Working out a level

`CharacterTrait::update_level(increase_mode)` (`0x58E7F5`) works out the level from the points, and returns whether the level changed:

- With `increase_mode = 1`, it reports a change **only if the level went up**, and `NoGoingBackLevel` isn't enforced.
- With `increase_mode = 0`, it reports a change **only if the level went down**, and `NoGoingBackLevel` is enforced.
  - The level is held at the no-going-back level, and the points are set to that level's threshold.

## The two update paths

### Trigger `Affects` (the buggy path)

Every trigger `Affects`, positive or negative, calls add-points `0x58F9D7(def, amount, 1)` (`CharacterRecord::add_trait_points`, from `fire_triggers` at `0x8AB29B`). Add-points does nothing for a female family member, a character of type 4, or an amount of 0. For a trait the character already has:

1. `0x58E771` does `points += amount`, even when the amount is negative, then calls `0x58E7F5(1)`, the increase mode.
2. The level pointer is updated to the new, possibly lower, level.
3. The attributes are updated (the old level's effects removed, the new level's added) only when step 1 reports a change, which happens only on a level **up** (`0x58FBD7`–`0x58FCD5`).
4. A trait whose points reach 0 or less is **not** removed. It stays as an invisible level-0 entry with 0 or negative points.

The result of a negative `Affects` that lowers a level:

- The scroll shows the lower level, or no trait at all.
- The stored attributes still include the old level's effects.
- `NoGoingBackLevel` is ignored, so traits can be dropped below it.
- **It ratchets.** If the trait is gained again, going up the levels adds the effects again, on top of the leftover ones; remove it again, and those stay too.

Also, a trigger `Affects` on a trait the character *doesn't* have creates it with `|amount|` points (`0x58FDB6`). So a negative `Affects` without a `Trait X > 0` guard *gives* the trait.

### AntiTraits (the correct path)

Before add-points gives the new points, it checks each trait the character already holds (`0x58FA46`–`0x58FB4C`). If that trait's `AntiTraits` list names the trait being added, it calls subtract-points `0x58FF84`:

1. `0x58E797` does `points -= amount`, then calls `0x58E7F5(0)`, the decrease mode.
2. If the level dropped, the old level's effects are subtracted and the new level's added (`0x59001E`–`0x5900E8`), so **the attributes stay correct**.
3. If the points reach 0 or less, the trait entry is removed (`0x5901D9`).
4. `NoGoingBackLevel` is respected.

Only the points not used up there go to the new trait.

The check reads the **existing** trait's `AntiTraits` list, so the trait being removed must name the trait being added. Ancillary gain and loss (`0x59067D` and `0x59082A`) also adjust the stored attributes symmetrically.

## What that means for trait-removing triggers

| Trigger | Effect | Drift |
|---|---|---|
| `Affects <bad trait> -N` | Hides the bad trait but keeps its penalty; the penalty stacks each time the trait comes back and is removed again | Downward |
| `Affects <good trait> -N` (capping a trait's level) | Caps the visible level but keeps the higher level's bonus; bonuses stack as the trait climbs again | Upward |
| `Affects <trait> N` | Correct: level-ups update the attributes | None |

To remove a trait cleanly from a trigger:

1. Give points to a trait that the trait being removed lists in its `AntiTraits`, instead of a negative `Affects`.
   - For example, `Affects GoodCommander N` takes points from `BadCommander` through the correct path, as long as `BadCommander`'s own `AntiTraits` names `GoodCommander`.
2. Leave alone the traits that have no usable anti-trait, or are already at or above their `NoGoingBackLevel`.
   - Removing those with a negative `Affects` only hides them and keeps the penalty, and each re-gain stacks it again; left in place, the penalty at least stays constant.
3. Don't use negative `Affects` to cap a trait's level, or accept that it leaves stale bonuses behind.

Characters that already carry leftover effects won't recover by themselves, since the stored attributes are never recalculated. The console recipe below can offset them one character at a time.

## How triggers are read

`read_triggers` (`0x8AA428`) reads each `Trigger` block's lines in this order:

1. `WhenToTest` must come straight after the name, or it's a script error.
2. One `Condition` line, with its `and` lines, is optional.
3. At least one `Affects`, `AcquireAncillary` or `AdviceThread` line must follow, or it's a script error.
4. Any number of `Affects` lines come next (10 fit, with no bounds check).
5. At most one `AcquireAncillary` line comes next; an unknown ancillary is a script error.
6. Any number of `AdviceThread` lines come last (5 fit, with no bounds check).
   - An unknown thread, or a missing increment, is a script error.

A line out of that order ends the block: the reader skips lines until the next `Trigger`, without an error. For example:

- A second `AcquireAncillary` line is ignored, and so is everything after it.
- An `Affects` line after `AcquireAncillary` or `AdviceThread` is ignored.

Other things the parser lets through:

- **A `WhenToTest` that isn't an event's name drops the whole trigger, with no error.** Event names are case-sensitive.
- **An `Affects` trait that isn't found is not an error.** The lookup (`find_trait(const char*)`) ignores case, and a misspelt trait is kept as a null pointer, which the trigger reads through the first time it fires.
- **Duplicate trigger names are allowed.** Both triggers are kept and both fire; looking a trigger up by name (`find_trigger`, `0x8AABCA`) finds the later one.

## Trigger evaluation order

Triggers are processed **one at a time, in file order**, not collected and applied together.

- `fire_triggers` (`0x8AAE20`) runs once per event (such as `CharacterTurnEnd`) per character, and walks that event's trigger list from its head.
  - The parser adds each trigger at the end of its event's list, so file order is run order.
- For each trigger, it tests the conditions at that moment.
  - If they hold, it applies the trigger's lines in a fixed order, whatever their order in the file: the advice first, then the ancillary, then each `Affects` line in turn.
  - Each `Affects` line rolls its own `Chance` and calls add-points right away, before the next line or trigger.
- After the list, every trigger's tested and passed flags are cleared.
- **In a multiplayer game with more than one human player, nothing fires.** `fire_triggers` returns before the list, so no traits, ancillaries or advice are given.

What each line needs:

- **`Affects`:** the event must have a character record, and:
  - The trait's `Characters` must include the character's type; a record with no character on the map counts as type 5.
  - The culture of the record's faction must not be in the trait's `ExcludeCultures`.
- **`AcquireAncillary`:** the record must have a character on the map, not of type 4, and the culture of its faction must not be in the ancillary's `ExcludeCultures`.
- **`AdviceThread`:** a campaign setting (`campaign+0xb8`, not identified) must be 1.

**The `Chance` roll succeeds when `roll <= Chance`**, with the roll from 0 to 99 (the random function `0xF3AAD0`, NYD). So:

- `Chance 100` always succeeds.
- `Chance N` succeeds N + 1 times in 100, so even **`Chance 0` succeeds 1 time in 100**.
- Traits and ancillaries roll the same way, comparing in floating point against `1.0 × Chance`.

Consequences:

- A chain written in the order `Trait X = 0` → `= 1` → `= 2`, each adding enough points for the next level, takes a character from 0 to level 3 **in one turn**. Written in the order 2 → 1 → 0, it gives one level per turn.
- Triggers that remove a trait's levels written highest level first chain all the way down in one turn, which is still the buggy path.
- `Chance` is rolled separately for each `Affects` line, so a chain with a `Chance` below 100 can stop partway.
- `Chance 0` can't switch a line off; remove the line or give the trigger a condition that never holds.

## Removing traits past `NoGoingBackLevel` with triggers ("middle ground")

With `E(n)` meaning the effects listed for level n (each level lists its full effects, not an increment), take a bad trait at level L ≥ NGB:

| Step | Path | Visible level | Stored effects |
|---|---|---|---|
| Start | – | L | E(L) |
| 1. AntiTrait points | Correct; stops at NGB | NGB | E(NGB) |
| 2. Negative `Affects` down to k < NGB | Buggy | k | E(NGB) |
| 3. AntiTrait points to 0 | Correct; subtracts only E(k) | Removed | E(NGB) − E(k) |

- The best case is **k = NGB − 1**, which leaves **E(NGB) − E(NGB−1)** hidden permanently.
  - Step 2 must land exactly on that level; dropping further brings back more of the penalty.
- The leftover stacks each time the trait comes back and is removed this way again.
- For traits with **NGB 1**, k = 0 and the whole of E(1) is left over, so this gains nothing.
- A trait with **no AntiTraits at all** can't take step 3.
- For some traits the level below NGB has effects the NGB level doesn't, which makes this **worse than leaving the trait at NGB**.
- The points not used up in steps 1 and 3 go to the anti-trait, so check that it's a trait you want.

## Console commands

### `give_trait <char> <trait> <opt:level>`

The handler is `0xDE3FED`, and the level defaults to 1. It looks the trait up with the case-sensitive `find_trait(String)`, then calls set-trait `0x58F70E(trait, level, 1)` (NYD up to here). Set-trait (`CharacterRecord::set_trait`) does nothing for a character of type 4; otherwise it:

1. If the character already has that trait, subtracts its **current level's** effects from the stored attributes and deletes the trait entry.
2. Calls add-points with the requested level's threshold (`levels[level].threshold`).
   - That runs the AntiTraits check first, then creates the trait and adds that level's effects correctly.

Results:

- **It ignores `NoGoingBackLevel`.** The entry is deleted outright, and neither step checks it.
- **`give_trait <char> <trait> 0` removes the trait cleanly**, whatever its level.
  - Level 0's threshold is 0, and add-points skips an amount of 0 entirely, so the trait is deleted after its current effects are subtracted.
  - No anti-trait is touched.
- **`give_trait <char> <trait> k` sets exactly level k**, with the attributes kept correct.
  - If the character holds one of the trait's anti-traits, the points go into cancelling it first.
- **Don't ask for a level above the trait's maximum.** The clamp allows `level == level count`, one entry past the end of the table.

### `give_trait_points <char> <trait> <points>`

This calls add-points `0x58F9D7(trait, points, 1)` (at `0xDE4C41`; NYD), the **same path as a trigger's `Affects`**.

- Negative points have the same bug: the level drops, and the effects stay.
- On a trait the character doesn't have, it gives the trait with `|points|`.

### Clearing stacked hidden modifiers

**Neither giving the anti-trait nor `give_trait` clears them.** The leftover effects live only in the stored attribute array, and nothing records which trait produced them. A trait "removed" by a negative `Affects` is still there as a hidden level-0 entry:

- Giving its anti-trait (the correct subtract path) subtracts level 0's effects, which are none, and deletes the entry, so the leftover stays.
- `give_trait <char> <trait> 0` does the same.
- Giving the trait again at its old level adds the effects again.

**You can offset them with the console, using the bug.** For example, to cancel a leftover Influence −1:

1. `give_trait <char> T 1`, where T is a trait whose level 1 is exactly Influence +1 and nothing else; the bonus is added cleanly.
2. `give_trait_points <char> T -999`; T drops to hidden level 0, and the +1 stays.
3. `give_trait <char> T 0`; the hidden entry is deleted, and nothing is subtracted.

Caveats:

- It's a manual fix for one character at a time; triggers can't run console commands.
- The exact leftover amount can't be seen.
  - The scroll clamps Management and Influence to 0–10, so work it out from the trait history, or offset until the number visibly moves.

## Confirmed in-game

### The buggy path, and file-order processing

Test: two triggers with the **same name**, in this order in the file:

```
Trigger pontus_handi_GoodAdministrator
    WhenToTest CharacterTurnEnd

    Condition OriginalFactionType pontus
        and Trait GoodAdministrator < 3

    Affects GoodAdministrator  24  Chance  100

Trigger pontus_handi_GoodAdministrator
    WhenToTest CharacterTurnEnd

    Condition OriginalFactionType pontus
        and Trait GoodAdministrator = 3

    Affects GoodAdministrator  -24  Chance  100
```

What it showed:

- **Leftover effects:** each end of turn, the first trigger raises GoodAdministrator to level 3, which adds its effects.
  - The second drops it back to hidden level 0, and the effects stay, so they pile up turn after turn.
- **Sequential, file-order processing:** the second trigger's `Trait GoodAdministrator = 3` check sees the change the first trigger made in the same end of turn.
- **Duplicate trigger names are allowed:** both triggers ran.
- **`Affects <trait> 0` does nothing.** Changing the second trigger's value to 0 had no effect.
  - This matches add-points returning straight away when the amount is 0 (`0x58FA3B`), so a zero-point `Affects` can't force a level recheck or an attribute update.

### AntiTrait removal is clean

Test on a new game. GoodAdministrator reaches level 1 at 6 points, level 2 at 12 and level 3 at 24, with Management +1/+2/+3 and `AntiTraits BadAdministrator`.

```
Trigger pontus_handi_GoodAdministrator
    WhenToTest CharacterTurnEnd

    Condition OriginalFactionType pontus
        and Trait GoodAdministrator < 2

    Affects GoodAdministrator  17  Chance  100

Trigger pontus_handi_BadAdministrator_1
    WhenToTest CharacterTurnEnd

    Condition OriginalFactionType pontus
        and Trait GoodAdministrator = 2

    Affects BadAdministrator  6  Chance  100

Trigger pontus_handi_BadAdministrator_2
    WhenToTest CharacterTurnEnd

    Condition OriginalFactionType pontus
        and Trait GoodAdministrator = 1

    Affects BadAdministrator  6  Chance  100
```

**With 17, no Management is left over.** The first end of turn:

| Trigger | GoodAdministrator points | Level | Management | Path |
|---|---|---|---|---|
| +17 GoodAdministrator | 17 | 2 | +2 | Level up, effects added |
| +6 BadAdministrator | 11 | 1 | +1 | AntiTrait subtract (`0x58FF84`): removes level 2's effects, adds level 1's |
| +6 BadAdministrator | 5 | 0 | 0 | The same, down to level 0 |

The 6 points each time are fully used up cancelling GoodAdministrator, so BadAdministrator never appears.

**With 18, it ends at level 2 (Skilled Bureaucrat).** The first +6 brings the points to exactly 12, still at the level 2 threshold. The level doesn't change, and the `= 1` trigger doesn't fire.

GoodAdministrator stays as a hidden level-0 entry with 5 points left over, since it's only deleted at 0 or below. So in later turns, the 17 version should alternate:

- Turn 2: 22 → 16, ending at level 2 with Management +2.
- Turn 3: 10 → 4, ending at level 0 with Management 0.

Management stays in step with the visible level throughout. That alternation hasn't been checked in game.

## Not verified

- `give_trait` in game. Test on a governor:
  1. `give_trait <name> GoodAdministrator 3`, then `give_trait <name> GoodAdministrator 0`.
  2. Management should go up, then return to exactly its starting value, even though GoodAdministrator has `NoGoingBackLevel 3`.
- The console offset recipe.
- That the `spy, assassin` traits never reach assassins in game.
- What the runtime does with a null anti-trait entry (from a misspelt `AntiTraits` name).

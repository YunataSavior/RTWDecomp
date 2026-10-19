# Do wonder bonuses decay over turns?

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Verdict: partly.** The permanent wonder effects never decay (NYD). Capturing a wonder, though, adds a temporary public order bonus that shrinks by 1 each turn. It shows on the same "Wonder" line of the public order breakdown, so it looks as if the wonder itself is fading.

A wonder belongs to a **faction**, not to the settlement it stands in. So both the capture bonus and the permanent effects apply across the holding faction's settlements, regions or admirals, not just one city.

Sources:

- The decompiled [`src/code/landmark.cpp`](../../src/code/landmark.cpp) and [`src/code/landmark_db.cpp`](../../src/code/landmark_db.cpp), both matching the original.
- The faction's turn update in [`src/code/faction.cpp`](../../src/code/faction.cpp), matching the original (the rest of that unit isn't decompiled yet).
- The public order formula, `SettlementDD8::compute_order` in [`code/settlement_mechanics.cpp`](../../src/code/settlement_mechanics.cpp), which is functionally equivalent to the original but doesn't match it byte for byte.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the code outside those units, such as the trade, farming and movement code, which isn't decompiled yet (NYD).

Addresses are virtual addresses in that exe (image base `0x400000`).

## The data files

- `descr_sm_landmarks.txt` holds only each wonder's model, image, movie and faction-specific `*_rtm` files.
  - `load_landmark_descriptions()` (`0x8F7A09`) reads it.
- `data/text/landmarks.txt` holds only text: `<type>_title`, `_short_descr`, `_long_descr` and `_effects`.
- **The effects themselves are hardcoded in the exe**, so the `_effects` text can disagree with them (NYD; see [Other notes](#other-notes-nyd)).

## The landmark object

`Landmark` (0x44 bytes, declared in [`landmark.h`](../../src/code/landmark.h)):

| Offset | Member | Meaning |
|---|---|---|
| `+0x34` | `m_type` | The engine type ID (see below), or 10 for none |
| `+0x38` | `m_region` | The region it stands in |
| `+0x3c` | `m_faction` | The faction that holds it, or 0 when nobody does |
| `+0x40` | `m_description` | Its entry in `descr_sm_landmarks.txt` |

The holder is a `Faction`:

- After a load, `resolve_factions()` (`0x49D535`) looks the saved holder up as saved-pointer type 0.
  - Type 0 is what `Faction::load` (`0x4C23A0`) registers each faction as (NYD).
- `num_held(faction)` compares the holder's ID with a faction index.

`LandmarkManager` keeps the campaign's landmarks:

- `save()` (`0x49D455`) writes, per landmark, the type name, its 8-byte tile position and the holding faction, and `load()` (`0x49D302`) reads them back.
  - **There's no turn counter or acquisition timestamp.**
- `resolve_factions()` (`0x49D535`) turns the saved faction pointers back into live ones after loading.
- `num_held(faction)` (`0x49CFBA`) counts the wonders a faction holds, for the `Hold_all_wonders` triumph.
- `find(type)` (`0x49D035`) looks up a landmark by type ID.

### Engine type IDs

The hardcoded name-to-ID function `0xE3C9F6` gives them (NYD). They do **not** follow the order of `descr_landmarks_lookup.txt`.

| ID | Wonder |
|---|---|
| 1 | `pyramids_and_sphinx` |
| 2 | `statue` (Zeus) |
| 3 | `temple` (Artemis) |
| 4 | `mausoleum` |
| 5 | `colossus` |
| 6 | `pharos` |
| 7 | `stonehenge` (unused in vanilla) |
| 8 | `barge` (unused in vanilla) |
| 9 | `gardens` |
| 10 | none (an unknown name; `descr_sm_landmarks.txt` rejects it) |

## Where each effect is applied

The Colossus, Pharos and Gardens rows come from disassembly (NYD); the Statue of Zeus and Pyramids rows come from the decompiled public order formula.

| Wonder | Location | What the code does |
|---|---|---|
| Statue of Zeus (2) | `SettlementDD8::compute_order` | Sets the wonder order value to **2** (internal units) when the settlement's faction holds the Statue |
| Colossus (5) | `0x484DDD`–`0x484E9C` | Sums the region's trade, and if the region's faction holds the Colossus, adds `ceil(sum × 0.2)` |
| Pharos (6) | `0x43A4E0`, `0x58CC91` | Halves the ship-sinking chance for fleets of the holding faction, and gives its admirals 1.1 times the movement points |
| Gardens (9) | `0x5CDBE5` | Adds +20 to farming |
| Pyramids (1) | `SettlementDD8::compute_order` | Counts the population of the `egyptian` culture as the owner's own, for the culture share of public order ("loyalty of Egyptians") |

None of these read a turn number, a duration or a timer. They depend only on who holds the wonder, and on constants.

- The Pharos movement bonus is in `Character::reset_movement()` (`0x58CBBC`), which sets a character's movement points at the start of each turn.
  - An admiral gets ×1.1 when its faction holds the Pharos; otherwise, a character for which `0x581484` returns true gets ×1.4.
- The Colossus bonus is rounded up: `0xFC6CDA` is the C runtime's `ceil`.

## The decaying part: the capture bonus

`Landmark::set_faction(faction, captured)` (`0x49CD32`) is called when a wonder changes hands:

```cpp
void Landmark::set_faction(Faction* faction, bool captured)
{
    if (faction)
        notify(LandmarkEvent(faction, this));
    if (captured) {
        faction->m_wonder_order = 4;            // faction+0x1d0
        ...                                     // UI messages, and a trigger event for the port's character
    }
    m_faction = faction;
}
```

`Faction::m_wonder_order` (`+0x1d0`) is a per-faction counter:

- It's **set to 4** when the faction captures a wonder (above).
- It's **lowered by 1 per turn while it's above 0**, in the faction's turn update `Faction::unk_004CAC88()` (`0x4CAC88`):

  ```cpp
  if (m_triumph_order > 0)
      m_triumph_order--;
  if (m_wonder_order > 0)
      m_wonder_order--;
  ```

  - The same function winds down the triumph counter at `+0x1cc` in the same way.
  - The whole update, the countdown included, is skipped while `+0x178` (a flag `load_game` also reads) is set.
  - The campaign calls it for every faction at the start of each round (`Campaign::unk_0045361A`, in [`code/world.cpp`](../../src/code/world.cpp)), after the turn number goes up.
- It's saved and loaded with the faction, in `Faction::load` (`0x4C23A0`, the write at `0x4C34E7`; NYD).

The public order breakdown is built in `SettlementDD8::compute_order` (`0x5CC9D7`). It writes the positive factors into slots, and slot 6 is `SOF_WONDER`:

```
slot[5] (SOF_TRIUMPH) = faction[+0x1cc]
slot[6] (SOF_WONDER)  = statue_value + faction[+0x1d0]
```

Here `faction` is the settlement's faction.

After a capture, then, the "Wonder" line in **every settlement of the capturing faction** goes **4 → 3 → 2 → 1 → 0** over four turns, which is 20%, 15%, 10%, 5% and 0% (one unit is 5%; see [public order](public_order_findings.md)). It then settles at the permanent value: 2 (10%) if the faction holds the Statue, otherwise 0.

- A second capture while the bonus is winding down resets it to 4; the bonuses don't add up.

The `SOF_WONDER` modifier in `data/descr_settlement_mechanics.xml` scales the whole line, both the permanent and the decaying part. It has no time parameter (NYD).

## Other notes (NYD)

- **Statue of Zeus scope:** the code compares the Statue's holder with the settlement's faction.
  - So the bonus applies in every settlement of the faction that holds the Statue, as the description text says.
- **Colossus magnitude:** this code path uses 0.2, rounded up, not the "40%" in the description.
  - A second place could apply it again (for example on both ends of a trade route); that wasn't checked.
- **Colossus fluctuation:** the Colossus has no timer, so turn-to-turn changes in its bonus come from the trade income itself (partners, blockades, trade rights).
- **Capturing a wonder** also fires a trigger event through `fire_triggers` (`0x8AAE20`) (see [trait effects](trait_effects_findings.md#trigger-evaluation-order)), probably `GeneralCaptureWonder`.
  - In `export_descr_character_traits.txt` (`triumphs3`/`triumphs4`) that adds 5 points to `VictorOthersVirtue`/`VictorRomanVirtue`, which are character traits, not settlement order.
- The public order code (`SettlementDD8::compute_order`) and the farming code (`0x5CDB6A`) are split into `settlement_mechanics.cpp`, though which file they come from is a guess.
  - They sit in a run of code with no asserts (`0x5C7648`–`0x5CF3F9`) between `settlement_manager.cpp` and `settlement_verified.cpp`.
  - That file's path is the only one in the data between those two files' paths, apart from `settlement_plan.cpp`'s.

## Not verified

- That the campaign calls the faction's turn update exactly once per turn, though its contents (ageing, movement points) strongly suggest it.
- In-game checks that would confirm the above:
  1. Capture Olympia and watch the "Wonder" public order entry for five turns, in Olympia and in another of your settlements; it should step down by one unit per turn in both, then hold.
  2. Load a save where you've held a wonder for a long time; the entry should be steady.

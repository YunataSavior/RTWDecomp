# Do wonder bonuses decay over turns?

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Verdict: partly.** The permanent wonder effects never decay (NYD). Capturing a wonder, though, adds a temporary public order bonus that shrinks by 1 each turn. It shows on the same "Wonder" line of the public order breakdown, so it looks as if the wonder itself is fading.

Sources:

- The decompiled [`src/code/landmark.cpp`](../../src/code/landmark.cpp) and [`src/code/landmark_db.cpp`](../../src/code/landmark_db.cpp), both matching the original.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the code outside those units, such as the public order and settlement update functions, which aren't decompiled yet (NYD).

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
| `+0x3c` | `m_settlement` | The settlement that holds it, or 0 when nobody does |
| `+0x40` | `m_description` | Its entry in `descr_sm_landmarks.txt` |

`LandmarkManager` keeps the campaign's landmarks:

- `save()` (`0x49D455`) writes, per landmark, the type name, its 8-byte tile position and the holding settlement, and `load()` (`0x49D302`) reads them back.
  - **There's no turn counter or acquisition timestamp.**
- `resolve_settlements()` (`0x49D535`) turns the saved settlement pointers back into live ones after loading.
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

## Where each effect is applied (NYD)

| Wonder | Location | What the code does |
|---|---|---|
| Statue of Zeus (2) | `0x5CCDE4`–`0x5CCE35` | Sets the order value to **2** (internal units) if *this* settlement holds the Statue |
| Colossus (5) | `0x484DDD`–`0x484E9C` | Sums the settlement's trade, and if this settlement holds the Colossus, adds `round(sum × 0.2)` |
| Pharos (6) | `0x43A4E0`, `0x58CC91` | Halves the ship-sinking chance, and multiplies naval movement by 1.1 |
| Gardens (9) | `0x5CDBE5` | Adds +20 to farming |
| Pyramids (1) | `0x5CD1DD` | Looks up the `egyptian` culture (loyalty of Egyptians) |

None of these read a turn number, a duration or a timer. They depend only on who holds the wonder, and on constants.

## The decaying part: the capture bonus

`Landmark::set_settlement(settlement, captured)` (`0x49CD32`) is called when a wonder changes hands:

```cpp
void Landmark::set_settlement(Settlement* settlement, bool captured)
{
    if (settlement)
        notify(LandmarkEvent(settlement, this));
    if (captured) {
        settlement->m_wonder_order = 4;         // settlement+0x1d0
        ...                                     // UI messages, and a trigger event for the port's character
    }
    m_settlement = settlement;
}
```

The public order breakdown is built in `0x5CC9D7` (NYD). It writes the positive factors into slots, and slot 6 is `SOF_WONDER`. At `0x5CCFDF`–`0x5CD005`:

```
slot[5] (SOF_TRIUMPH) = settlement[+0x1cc]
slot[6] (SOF_WONDER)  = statue_value + settlement[+0x1d0]
```

So `settlement+0x1d0` is a per-settlement counter:

- It's **set to 4** when a wonder is captured into that settlement (above).
- It's **lowered by 1 per turn while it's above 0**, in the settlement update `0x4CAC88` (`0x4CAEB5`–`0x4CAECB`; NYD).
  - The same routine winds down the triumph counter at `+0x1cc` in the same way.
- It's saved and loaded with the settlement (`0x4C34E7`; NYD).

After a capture, then, the "Wonder" line in that settlement goes **4 → 3 → 2 → 1 → 0** over four turns. It then settles at the permanent value: 2 if the settlement holds the Statue, otherwise 0.

The `SOF_WONDER` modifier in `data/descr_settlement_mechanics.xml` scales the whole line, both the permanent and the decaying part. It has no time parameter (NYD).

## Other notes (NYD)

- **Statue of Zeus scope:** the code compares the Statue's holding settlement with the settlement whose order is being calculated.
  - So the bonus applies only in the settlement that holds the Statue, not in "all settlements" as the description text says.
- **Colossus magnitude:** this code path uses 0.2, not the "40%" in the description.
  - A second place could apply it again (for example on both ends of a trade route); that wasn't checked.
- **Colossus fluctuation:** the Colossus has no timer, so turn-to-turn changes in its bonus come from the trade income itself (partners, blockades, trade rights).
- **Capturing a wonder** also fires a trigger event through `fire_triggers` (`0x8AAE20`) (see [trait effects](trait_effects_findings.md#trigger-evaluation-order)), probably `GeneralCaptureWonder`.
  - In `export_descr_character_traits.txt` (`triumphs3`/`triumphs4`) that adds 5 points to `VictorOthersVirtue`/`VictorRomanVirtue`, which are character traits, not settlement order.

## Not verified

- How many on-screen percent one internal order unit is worth.
- That `0x4CAC88` runs exactly once per turn, though its structure strongly suggests an end-of-turn update.
- In-game checks that would confirm the above:
  1. Capture Olympia and watch the "Wonder" public order entry for five turns; it should step down by one unit per turn, then hold.
  2. Load a save where you've held a wonder for a long time; the entry should be steady.

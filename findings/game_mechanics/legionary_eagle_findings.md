# When a faction loses a legionary eagle

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. Unmarked claims come from the matching source in [`src/`](../../src/).

**Short answer:** an eagle is lost when a unit with the `command` attribute is **destroyed in a battle**, but only if it belongs to a **major faction** and its army's general is **not inside a settlement**. The eagle goes to the **first army of the other side**, not to the army that destroyed the unit.

Sources:

- The decompiled unit removal, `Army::remove_unit` ([`code/army.cpp`](../../src/code/army.cpp)), which runs whenever an army loses a unit for good, for example in `Army::kill_soldiers`.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for what reads the flag afterwards, marked NYD.
- Addresses are virtual addresses in that exe (image base `0x400000`).

## The check

When a unit is removed from its army, the eagle is lost only if all of these hold:

1. The army is in a battle (`+0x12a`, which only the battle code sets; NYD).
2. The army's faction is a major one: it has `custom_battle_availability` in `descr_sm_factions.txt` and not the flag at its description's `+0xa7` (`Faction::unk_00445B2B`, the same test as for [gladiator uprisings](public_order_findings.md)).
3. The army has a general (`+0xf0`), and that general isn't in a settlement (`+0xd4`).
4. The unit's type has the `command` attribute of `export_descr_unit.txt` (bit 14 of the attributes at `+0x200`).

Then:

- The battle's other side gives its **first army** the captured-eagle flag (`+0x138`), whichever of that side's armies destroyed the unit.
- If a campaign flag (`0x2921AE0`) and the battle's `+0x46` are both set, the losing faction gets a message of type `0x60` (`fn_00CB67A8`, in [`code/ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp)).
- The exe has the trigger events `LostLegionaryEagle`, `CapturedLegionaryEagle` and `RecapturedLegionaryEagle`, and the senate missions `sm_recapture_eagle.cpp` and `sm_negotiate_eagle_return.cpp`; what fires them wasn't traced.

## Consequences

1. **A garrison never loses its eagle.** A command unit destroyed while defending a city whose general is inside it doesn't trigger the check.
2. **An army without a general never loses its eagle.**
3. **Minor factions never lose eagles,** whatever their units' attributes.
4. **The credit can go to the wrong army.** With reinforcements, the eagle goes to the enemy side's first army, even if a reinforcing army destroyed the unit.

## Not verified

- Which vanilla units have `command`.
- What reads the captured-eagle flag at `+0x138`, and whether one army can hold more than one eagle.
- What the `+0xa7` description flag is.
- In-game checks: none yet.

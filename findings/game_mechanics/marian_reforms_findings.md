# When the Marian reforms happen

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. Unmarked claims come from the matching source in [`src/`](../../src/).

**Short answer:** the Marian reforms start at the end of the first round in which **any major faction** holds a **huge city in Italy other than Rome**. The faction doesn't have to be Roman: Carthage taking a huge Italian city brings them too. They then apply to every faction at once, and they can happen only once. `descr_strat.txt` can disable them or start the campaign with them.

Sources:

- The decompiled round-end pass, `Campaign::unk_004537F2` ([`code/world.cpp`](../../src/code/world.cpp)), and each faction's part of it, `Faction::unk_004CAEDB` ([`code/faction.cpp`](../../src/code/faction.cpp)).
- What the reforms do on the spot: `Faction::marian_reforms` ([`code/faction.cpp`](../../src/code/faction.cpp)), `Character::marian_reforms` ([`code/character.cpp`](../../src/code/character.cpp)) and `UnitDB::has_general_upgrade` ([`code/unit_db.cpp`](../../src/code/unit_db.cpp)).
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the rest, marked NYD.
- Addresses are virtual addresses in that exe (image base `0x400000`).

## The flags

| Field | Meaning |
|---|---|
| `campaign+0x2b6` | The reforms have happened. `descr_strat.txt`'s `marian_reforms_activated` sets it (and clears `+0x2b7`; `0x44D858`, NYD). |
| `campaign+0x2b7` | The reforms can't happen. `descr_strat.txt`'s `marian_reforms_disabled` sets it (and clears `+0x2b6`; `0x44D89D`, NYD). |
| `campaign+0x258` | The settlement named `Rome`. A new campaign sets it while reading `descr_strat.txt` (`0x8E83DC`, which complains "we already have assigned rome!" about a second one; NYD). A loaded game sets it again in `Settlement::after_load` (`0x5B2241`, Code), from the settlement whose region names it `Rome` in `descr_regions.txt`. `campaign+0x260` is Constantinople, found the same way; `Faction::destroy` (`0x4CE245`) reads both when `empire_west` or `empire_east` is destroyed, and tells every faction (message type `0x9c`) when either empire holds both. |

## The check

Once a round, the round-end pass runs `Faction::unk_004CAEDB` for every faction. It updates the faction's finances, its armies' tiles and its settlements, and then runs its plague tick ([plague](plague_findings.md)). It returns true when the faction holds a settlement for which all of these hold:

1. The faction is a major one: it has `custom_battle_availability` in `descr_sm_factions.txt` and not the flag at its description's `+0xa7` (`Faction::unk_00445B2B`, the same test as for [legionary eagles](legionary_eagle_findings.md)).
2. The settlement is at level 5, a huge city.
3. It isn't Rome (`campaign+0x258`).
4. Its region has the `italy` hidden resource (`descr_regions.txt`).

When a faction's check returns true and neither flag is set, the round-end pass shows the `marian_reform` message (the title and text are entries `0x2FB` and `0x31E` of `strat.txt`, through `strat_text`, `0xD93A0C`, in [`code/localisation/string_table_manager.cpp`](../../src/code/localisation/string_table_manager.cpp); their wording is in the game's text files, not the exe). `fn_00CB691A` ([`code/ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp)) sends it, as message type `0x7b`, to every major faction whose state (`faction+0x178`) is 0, so a faction that is dead doesn't get it.

- The message is shown once for each faction that passes in that round, so two factions passing in one round show it twice.

After every faction has been checked, if any passed, the pass sets `campaign+0x2b6` and runs `Faction::marian_reforms` for every faction.

## What happens at once

`Faction::marian_reforms` goes through the faction's characters. For each named character (type 5: a family member, not a hired general):

- When the faction owns a unit type with the `general_unit_upgrade` attribute in `export_descr_unit.txt` (`UnitDB::has_general_upgrade`), the character's bodyguard is replaced (`Character::unk_00581268`, NYD; probably with the upgraded bodyguard unit).
  - The attribute parser (`0x914617`, NYD) sets bit 25 of the unit type's attributes for `general_unit_upgrade`, and also bit 24, `general_unit`.
  - `Character::marian_reforms` checks the type again, through the character's `+0x80`, and then the faction's units, so the test runs twice.
- So factions without a `general_unit_upgrade` unit, such as the non-Roman factions in the original files, see no change here.

## What reads the flag later

- The senate's gift of a legion's first cohort needs it (`SenateFamily::check_gift`, [senate](senate_findings.md)).
- Which buildings and units it unlocks (the `marian_reforms` requirement of `export_descr_buildings.txt`) wasn't checked (NYD).

## Consequences

1. **Any major faction can bring the reforms.** The check doesn't look at the faction's culture, so a Greek or Carthaginian huge city in Italy brings them for the Romans too.
2. **Rome itself never counts**, however big it grows. Some other Italian city has to reach huge city size.
3. **The city has to be held at the end of a round.** The check runs only in the round-end pass, so a huge city that changes hands mid-round counts for whoever holds it then.

## Not verified

- What `Character::unk_00581268` does exactly, and whether it keeps the old bodyguard's experience and soldiers.
- The texts `0x2FB` and `0x31E`.
- Whether the parser reads the `marian_reforms` argument of `general_unit_upgrade` at all.
- Which settlement level names match levels 0 to 5 (village to huge city is the usual reading).
- In-game checks: none yet.

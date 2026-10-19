# How plague works on the campaign map

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short answer:** a settlement catches plague on its own only through **squalor**: at the end of each of its owner's turns, the chance is `(squalor − 3 − 2·health) / 2` in 1001, at most about 2.3%. A plagued settlement **loses 10% of its population a turn**, its garrison loses 4–10% of its soldiers a turn, and it **trades with no neighbour by land**. The plague ends after 4 of the faction's plague ticks. **Characters carry it**: one who enters a plagued settlement, fort or army catches it, and one who is sick infects what they enter. A sick character has a **5% chance a turn to die** of it, and their army loses 4–10% a turn. They recover after 4 sick turns, but **not while they stay in a plagued settlement**. The turn report undercounts the soldiers armies lose to it.

Sources:

- The decompiled plague roll, `Settlement::end_turn` ([`code/settlement.cpp`](../../src/code/settlement.cpp)), and the growth factors, `SettlementDD8::compute_growth` ([`code/settlement_mechanics.cpp`](../../src/code/settlement_mechanics.cpp)).
- The decompiled plague ticks: each settlement's and fort's, `SettlementBase::plague_tick` ([`code/settlement_base.cpp`](../../src/code/settlement_base.cpp)); each faction's, `Faction::plague_tick`, from the round-end pass `Campaign::unk_004537F2` ([`code/world.cpp`](../../src/code/world.cpp)) through `Faction::unk_004CAEDB` ([`code/faction.cpp`](../../src/code/faction.cpp)), and each character's, `Character::plague_tick` and `Character::catch_plague` ([`code/character.cpp`](../../src/code/character.cpp)).
- The decompiled army losses, `Army::kill_soldiers` and `Army::remove_unit` ([`code/army.cpp`](../../src/code/army.cpp)), and `Unit::kill_soldiers` ([`code/unit.cpp`](../../src/code/unit.cpp)).
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the rest, marked NYD: spreading it to and from characters, trade, and the turn order.
- Addresses are virtual addresses in that exe (image base `0x400000`).

## The state it keeps

| Where | Field | Meaning |
|---|---|---|
| Settlement | `+0xb0` (byte) | Has plague; the trigger condition `SettlementHasPlague` reads it (`0x89DA2B`, NYD). |
| Settlement | `+0xac` | Plague ticks so far. |
| Settlement | `+0xa78` | Population the plague killed in the last growth step, for the turn report. |
| Character | `+0xae` (byte) | Has plague. |
| Character | `+0xb0` | Plague ticks so far. |
| Character | `+0xd4` | The settlement the character is in, if any. |

The flag and the tick count at `+0xb0`/`+0xac` sit in a base class that forts share with settlements (`SettlementBase`); `SettlementBase::catch_plague` and `SettlementBase::plague_tick` ([`code/settlement_base.cpp`](../../src/code/settlement_base.cpp)) work on both.

## How a settlement catches it

### By squalor

At the end of each of its owner's turns (`Settlement::end_turn`, called from the faction's end-of-turn step at `0x4CB8CC`), a settlement **without** plague rolls:

```
squalor = growth factor 8              // 0..50
health  = growth factor 3              // 0..25
if (squalor > 3) {
    chance = (squalor - 3 - 2 * health) / 2
    if (Random::range(0, 1000) < chance)   // 0..1000 inclusive, so chance / 1001
        catch plague (flag set, tick count and report count cleared)
}
```

- The growth factors come from `SettlementDD8::compute_growth`:
  - **Squalor** (factor 8) is the governor's Squalor attribute plus `(pop + max(pop − 2·A, 0)) / 1500 + max(pop − B, 0) / 350`, capped to 0–50, where A and B come from a per-level table (`0x12DAFC8`/`0x12DAFD4`). It's the same squalor as in [public order](public_order_findings.md).
  - **Health** (factor 3) is the buildings' `population_health_bonus` plus the governor's Health attribute, capped to 0–25.
  - Health is also one of the factors that add to growth, so it helps a city grow as well as keeping plague away.
- So the chance is at most `(50 − 3) / 2 = 23` in 1001, about **2.3% a turn**, and a settlement is safe once `2·health ≥ squalor − 4` (the halving rounds 1 down to 0).
- Every point of health cuts the chance by 1 in 1001, and every point of squalor raises it by half that.
- This roll sets the flag directly. It doesn't infect the characters already in the settlement (see below).

### From characters, forts and events

`SettlementBase::catch_plague` gives a settlement or fort plague when it has none:

- It sets the flag, clears the tick and report counts, then gives plague to every character on the same map tile (`Character::catch_plague`).
- A settlement or fort that already has plague passes none on through this call, so characters already on its tile don't catch it here.
- It's called when a sick character enters a settlement or a fort (`Character::enter_settlement` and `enter_fort`), and through `Character::set_settlement`, all in [`code/character.cpp`](../../src/code/character.cpp).
- The `plague` event of `descr_events.txt` calls it too (`0x43A8A4`): for each position the event lists, it infects the settlement on that tile, or else the settlement of the region the tile is in, and fires a trigger event.

## What it does to a settlement

1. **Population:** growth factor 9 is 20 while the settlement has plague (`SettlementDD8::compute_growth`).
   - Growth is `F0 + F1 + F2 + F3 + F4 + F5 − F8 − F9 − F10`, clamped to −200…25, then the AI's difficulty bonus is added, and the population changes by `round(0.005 · growth · pop)` (`SettlementStats::apply_growth`, in [`code/settlement_mechanics.cpp`](../../src/code/settlement_mechanics.cpp)).
   - The population is then kept between two figures of the settlement level's table (`0x12DAFD0`, and three times `0x12DAFD4`, the squalor table's B); the table is filled at run time, so its values weren't read.
   - So plague takes **10 points of percentage growth**: a settlement that was growing 2% a turn shrinks 8%.
   - Its own share, `round(0.005 · 20 · pop)` (10% of the population), is stored at `+0xa78` for the turn report.
2. **Garrison:** each plague tick, the garrison army (`+0x40`) loses a percentage of its soldiers (below).
3. **Trade (NYD):** the land-trade step (`0x5CE559`) gives a plagued settlement **no trade with any neighbour**, and every neighbour skips it too (each `RegionNeighbour`'s trade value at `+0x14` is set to 0).
   - It does the same for a settlement under siege (`+0x84`).
   - Whether sea trade checks plague wasn't checked.
4. **Public order:** plague isn't one of the order slots; it affects order only through the population and its squalor.

## How long it lasts

The round-end pass, `Campaign::unk_004537F2` ([`code/world.cpp`](../../src/code/world.cpp)), runs `Faction::unk_004CAEDB` for each faction, which ends with `Faction::plague_tick`. That comes after the faction's end-of-turn step has rolled for new plague (NYD):

- **Settlements and forts** (`SettlementBase::plague_tick`): the tick count goes up by 1, and at 4 the plague ends (the flag and both counts are cleared).
  - The garrison is hit only on the ticks where the plague is still on, so **3 times** for an outbreak.
  - Nothing keeps a settlement sick: sick characters inside don't extend it, but one entering after it ends starts a new outbreak.
- **Characters** (`Character::plague_tick`, on each character in the faction's list at `+0xe8`): the tick count goes up by 1, then:
  - At 5 or more, the character **recovers**, unless they're in a settlement that still has plague; while they stay there, they stay sick.
  - Otherwise, they **die of plague** when `Random::range(0, 19)` is 0, a 1-in-20 chance (death type 7, `DET_PLAGUE`).
  - Unless they recovered, an army they lead (`+0xe4`) loses 4–10% of its soldiers (below), whether or not they died.
  - So a character sick for the usual 4 ticks dies with probability `1 − 0.95⁴ ≈ 18.5%`.

## Army losses

The garrisons and the sick characters' armies each roll their own `pct = Random::range(4, 10)`. `Army::kill_soldiers(pct)` ([`code/army.cpp`](../../src/code/army.cpp)) then takes that share from every unit of the army, last unit first, once a tick:

- Each unit (`Unit::kill_soldiers`, the unit's virtual `+0x2ac`) loses `floor(pct · soldiers / 100)` soldiers.
  - A unit led by a character (`+0x278`) always keeps at least 1 soldier.
  - Units below 10 soldiers lose none, since `pct` is at most 10.
- Units left with no soldiers are removed from the army and deleted (`Army::remove_unit`), unless any of the unit's four slots at `+0x298` is filled (`Unit::unk_007B9438`).
  - A removed unit's character (`+0x278`) is killed with it, but a unit led by a character keeps 1 soldier, so plague never removes it.

## How characters catch and spread it

`Character::catch_plague` gives a character plague when they have none: the flag is set, the tick count cleared, and `CharacterDamagedByDisaster` fires with the plague disaster (type 8, after earthquake, flood, horde, storm, volcano, dustbowl, locusts and famine). It's called on many paths; the ones checked:

1. **Entering a settlement or fort** (`Character::enter_settlement` and `enter_fort`): the character catches it from a plagued settlement, then, if sick, infects the settlement.
2. **`Character::set_settlement`**, which both of those call first: when the character or the settlement is sick, both end up sick.
   - So the checks after it in the enter functions never change anything.
3. **Joining an army** (`0x588F38`): the character catches it from a sick army leader, then, if sick, infects the army's leader and every character listed in the army (`0x725E9A`).
4. **Army moves and merges** (`0x71DD8A`, `0x721E6B`): an army's leader catches it from a sick character or a plagued settlement it meets.
5. Other paths call it too (`0x4C7392`, `0x5818E9`, `0x7B64D2`), but weren't traced.

So characters are what moves plague around the map: an army that stops in a plagued city carries it to the next one it enters.

## The turn report

`Faction::plague_tick` gathers the plagued settlements, their population losses, the sick characters and the soldiers lost per faction, and passes them to `0xCB3AAB` (probably the turn's plague message; NYD).

- Forts' garrison losses are counted, but the forts themselves aren't listed.
- It looks like a bug that the character loop **resets** its soldier-loss total to 0 whenever it meets a character without plague after their tick, including one who just recovered.
  - So the report counts only the army losses after the last healthy character in the faction's list, and usually undercounts them.
  - Only the reported number is wrong; the soldiers are removed either way.

## Consequences

1. **Health buildings only matter against squalor.** The chance is 0 while `2·health ≥ squalor − 4`, so a small town with little squalor never catches plague on its own, whatever its health.
2. **Big cities are the risk**, since squalor grows with population. At 50 squalor and no health, a city catches plague about once every 43 turns on average.
3. **Plague costs a city about a quarter to a third of its people.** Three or four growth steps at 20 points below normal take 22–34% of the population, depending on the city's normal growth and the step count.
4. **A spontaneous outbreak doesn't touch the governor.** The squalor roll infects only the settlement, so characters already inside stay healthy until they leave and come back, or a sick character comes in.
5. **Sitting in a plagued city keeps a general sick.** They don't recover while the settlement has plague, but the settlement recovers after 4 ticks regardless, so this adds at most a few turns.
6. **Small units are immune to the soldier losses**, since a unit of 9 or fewer loses `floor(pct · n / 100) = 0`.

## Not verified

- When in the turn the population growth with factor 9 is applied, and so whether an outbreak costs 3 or 4 growth steps.
- What `0x485627` counts for growth factors 2 (grain, 3 points each) and 7 (slaves); probably the region's resources of that type.
- What reads growth factor 7 (slaves, plus the governor's SlaveTrading), which growth doesn't add.
- Whether sea trade and ports check plague.
- What the four slots at a unit's `+0x298` hold, which keep a unit with no soldiers in its army.
- The paths in `0x4C7392`, `0x5818E9` and `0x7B64D2`, and what the default events `plague_in_italy`, `plague_in_macedonia`, `plague_in_alexandria` and `plague_in_asia_minor` (strings in the exe) are.
- In-game checks: none yet.

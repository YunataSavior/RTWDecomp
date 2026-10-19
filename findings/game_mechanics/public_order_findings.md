# Public order, riots and revolts

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short answer:** the public order formula itself has no dice in it. A settlement's "Unrest" line moves on its own because of things the player doesn't see or control: **foreign spies inside the city**, the governor's traits changing, and the countdown after a change of owner. Below 70%, a settlement **always** riots; only whether it revolts instead, and how much damage a riot does, are random.

Sources:

- The decompiled order formula, `SettlementDD8::compute_order`, and the turn start that reads the rebel chance, `SettlementDD8::start_turn`, and the function that recomputes the figures, `SettlementDD8::update`, in [`src/code/settlement_mechanics.cpp`](../../src/code/settlement_mechanics.cpp).
  - `compute_order` and `update` don't match the original byte for byte, but they're functionally equivalent; [`RTWDecomp_nonmatching.md`](../../../RTWDecomp_nonmatching.md) says why.
  - The getters it calls are decompiled too: the spy's Subterfuge in [`faction.cpp`](../../src/code/faction.cpp), the settlement's region in [`settlement.cpp`](../../src/code/settlement.cpp), and the region's rebel type in [`world_map.cpp`](../../src/code/world_map.cpp).
  - So are the garrison's soldier counts, `Army::garrison_soldiers` and `Army::unk_00722430`, and an army's strength, `Army::compute_strength`, in [`army.cpp`](../../src/code/army.cpp), the search for foreign spies, `foreign_spies` in [`strategy_map.cpp`](../../src/code/strategy_map.cpp), and the unit size scale, `Campaign::unit_scale` in [`world.cpp`](../../src/code/world.cpp).
- The check that catches foreign spies, `Settlement::check_spies` in [`settlement.cpp`](../../src/code/settlement.cpp), its roll, `Faction::catch_spy` in [`faction.cpp`](../../src/code/faction.cpp), and the owner's combined Subterfuge, `combined_subterfuge` in [`spy_actions.cpp`](../../src/code/spy_actions.cpp).
- The settlement's own turn start, `Settlement::start_turn`, which advances the countdown after a change of owner, and the owner change itself, `Settlement::change_owner`, which restarts it, in [`settlement.cpp`](../../src/code/settlement.cpp).
- The revolt itself, `Settlement::revolt`, its army's strength, `Settlement::revolt_strength`, and who goes over in a civil war, `Settlement::find_defectors`, in [`settlement.cpp`](../../src/code/settlement.cpp).
- The riot and revolt check, `Settlement::check_riots`, its damage, `Settlement::riot_damage` (and the buildings' part, `SettlementBuildings::damage` in [`settlement_construction.cpp`](../../src/code/settlement_construction.cpp)), and the choice of revolt, `Settlement::revolt_type` and `largest_share`, in the same file, and the test for the Romano-British, `Campaign::last_britain_region` in [`world.cpp`](../../src/code/world.cpp); like the others, they're functionally equivalent without matching byte for byte.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for everything else (NYD), mostly `settlement_mechanics.cpp` (`0x5C7648`–`0x5CF3F9`, the run of code with no asserts between `settlement_manager.cpp` and `settlement_verified.cpp`) and `settlement.cpp` (`0x5B1440`–`0x5C1815`). The trait attribute IDs come from the name lookup `0xE41B13` that [`character_traits_db.cpp`](../../src/code/character_traits_db.cpp) calls. Addresses are virtual addresses in that exe (image base `0x400000`).

## How the percentage is built

`SettlementDD8::compute_order` (`0x5CC9D7`) fills 25 slots in internal units (one unit is **5%** on screen). Slots 0–9 add order and slots 10–24 take it away:

```
net     = sum(slots 0..9) - sum(slots 10..24) + AI bonus      // AI bonus: ai_public_order(), -2/0/+4/+6 by difficulty
net     = clamp(net, -20, 80)
percent = 100 + 5 * net                                        // so 0% to 500%, in steps of 5
if (settlement level == 0 && percent <= 50) percent = 51       // the lowest level never drops below 51%
```

The AI bonus applies only to AI factions other than the rebels (see [difficulty](difficulty_findings.md)).

"Governor" below means the character governing the settlement (`settlement+0xa98`). The governor attributes are trait attributes (`CharacterRecord` `+0x2c` array): Influence (1), LocalPopularity (`0x19`), Health (`0x2c`), Squalor (`0x2d`), Unrest (`0x2e`) and Law (`0x2f`). "Buildings" means a value from the settlement's buildings, read through `0x5D48AE` with an effect ID.

| Slot | Factor (probably) | Value, in units (×5%) | Cap |
|---|---|---|---|
| 0 | Garrison | The garrison army's soldiers, peasants counting half, against the population (below) | 0–16 |
| 1 | Law | Buildings (`law_bonus`) + governor Law | 0–25 |
| 2 | Entertainment | Buildings (`happiness_bonus`) | 0–25 |
| 3 | Governor's influence | Governor Influence + LocalPopularity; 0 with no governor | 0–8 |
| 4 | Low taxes | 6 when the tax level (`+0xac8`) is 0, else 0 | |
| 5 | Triumph | The faction's triumph counter (`faction+0x1cc`; see [wonders](wonder_bonus_findings.md)) | |
| 6 | Wonder | 2 if the faction holds the Statue of Zeus, + the faction's capture counter (`faction+0x1d0`) | |
| 7 | Boom | Growth above `boom_threshold()` (4/6/8/10 by difficulty for the player, 4 for the AI) | |
| 8 | Not identified | `0x5BC80B()` / 5 | |
| 9 | Health | Buildings (`population_health_bonus`) + governor Health | 0–25 |
| 10 | Squalor | From population (see below) + governor Squalor | 0–20 |
| 11 | Distance to capital | (`0x5BCDC4()` − 10) / 5 | 0–16 |
| 12 | Culture | (100 − share of the population of the owner's culture) / 10 | 0–10 |
| 13 | No garrison | 3 when the settlement has no army in it (`+0x40` is 0) | |
| 14 | High taxes | 4 at tax level 2, 8 at level 3 | |
| 15 | **Unrest** | See [below](#the-unrest-line) | 0–16 |
| 16 | (unused) | Always 0 | |
| 17 | Not identified | `settlement+0x8c` × 2 | |
| 18 | Not identified | From the object at `settlement+0x184`, when `0x49E979` says it's active | |
| 19 | (unused) | Always 0 | |
| 20–24 | Religion (BI) | One fifth of each of five values from `0x5F4FEB` (NYD), only when religions are enabled (`0x8D078B`, BI) | |

Notes:

- **Garrison:** `soldiers × 700 / round(scale × pop) × 2 / 5`, capped to 0–16, where `scale` is the unit size against the normal one (`Campaign::unit_scale`).
  - `scale` is the unit size option (20, 40, 80 or 160 men a unit) × 0.025, kept within 0.33–4: 0.5 for small, 1 for normal, 2 for large and 4 for huge units.
  - So the unit size doesn't change the garrison line: a huge unit has 4 times the soldiers of a normal one, and they're weighed against 4 times the population.
  - The campaign keeps the scale in its settings, which are set up from the options when it starts (`0x415D33`, NYD), so changing the unit size during a campaign probably doesn't change it (not checked).
  - In one mode (a flag at `0x29450A1`, set by a front-end screen; probably multiplayer) the scale is always 1 (NYD).
  - `soldiers` is every soldier of the garrison's units, except that a unit with the `is_peasant` attribute in `export_descr_unit.txt` counts half its soldiers (`Army::garrison_soldiers`).
  - So a garrison of peasants keeps half the order of the same number of any other soldiers.
  - This only matters in Barbarian Invasion: the code is in the vanilla `RomeTW.exe` too, with no check for the campaign, but no unit in the Imperial Campaign's `data/export_descr_unit.txt` has `is_peasant`; 15 of BI's units do (`bi/data/export_descr_unit.txt`), and a mod can add it to vanilla units.
  - BI itself runs `RomeTW-BI.exe`, which hasn't been checked, though both are built from the same source tree (`Rome,BI-1.5,1.6`).
- **Building effects** are the IDs of their names in `export_descr_buildings.txt`'s list (`0xFEC790`, from `population_growth_bonus` at 0): law is `law_bonus` (27), entertainment `happiness_bonus` (26), and health `population_health_bonus` (2).
- **Squalor:** `(pop + max(pop − 2·A, 0)) / 1500 + max(pop − B, 0) / 350`, where A and B come from a per-level table (`0x12DAFC8`/`0x12DAFD4`). The governor's Squalor is added to that before the cap.
- **Culture:** the share counts the population of every faction with the owner's culture, from a per-faction byte array at `settlement+0x19c`.
  - It's 100% when the settlement's `+0x1bc` (probably the faction that built it) is the owner.
  - **Pyramids:** while the owner holds the Pyramids, Egyptian-culture population counts as the owner's culture too.
  - When a campaign flag (`campaign+0x2b5`) is set, the share is halfway between it and 100%. That flag also changes the garrison weight (1600 instead of 700 per soldier).
- **Slots 16 and 19** are local variables set to 0 and never changed, so they're always 0.
- The breakdown is computed again whenever something calls `SettlementDD8::update` (`0x5C8729`, from about 40 places), and the percentage is stored at `settlement+0x1098`.
  - `update` works out the building effects, then the growth (`0x5CC583`), the public order and the `+0xb0` block (`0x5CDB6A`, probably the income), in that order, so the order always sees this turn's population and buildings.
  - In BI it first copies the region's religions into the figures (`0x5CE3E8`).
  - It does nothing while the campaign's `+0x4` is set (not identified), and it marks the owner's `+0x4e4` (probably "a settlement changed") when asked to. At the start of each turn, `SettlementDD8::start_turn` (`0x5C865A`) computes it and copies it to `settlement+0xed8`, which keeps last turn's figures.

## The "Unrest" line

Slot 15 is the sum of four things, capped at 16 units (80%):

1. **A change of owner:** `6 − turns since the change`, while that's above 0.
   - So a captured city shows 30%, 25%, …, 5% of unrest, then none.
   - The count of turns is `settlement+0x1c0`, which goes up by 1 at the start of each of the owner's turns (`Settlement::start_turn`, called for each of the faction's settlements from `Faction::start_turn` in [`code/faction.cpp`](../../src/code/faction.cpp)).
2. **Foreign spies in the city:** the Subterfuge of each spy on the settlement's tile whose faction isn't the owner (`foreign_spies`).
   - The search looks only at the spy's faction: not at whether the owner can see it, nor at the two factions' relations.
   - Each spy's Subterfuge counts at most 10 (`CharacterRecord::subterfuge`), and all spies together at most 10, so they can add **up to 50%**.
   - This counts spies of **every** other faction, allies included, and spies the player can't see.
3. **The governor's Unrest attribute**, from traits and ancillaries.
4. **The region's rebel chance:** the `chance` of the region's rebel type in `descr_rebel_factions.txt`, in units of 5%, whenever the owner isn't the faction `descr_regions.txt` names for the region (below).

The four parts don't depend on each other: the countdown doesn't look at the region's faction, and the rebel chance doesn't look at the countdown.

### Every change of owner restarts the countdown

`Settlement::change_owner` (`0x5B7F5E`), the settlement's virtual function that hands it to a new owner, resets the count to 0 for any change of owner. It doesn't compare the new owner with the old one, with the faction `descr_regions.txt` names for the region, or with the faction that built the settlement.

- So when faction X takes a city from Y and Y takes it back, Y gets the same 30% countdown as X did.
- The only exception is reason code 5, which starts the count at 5, so the new owner gets 5% of unrest for one turn.
- Occupying, enslaving and exterminating make no difference to the countdown.
  - The settlement changes owner (reason 1, through `0x49B15C`) as soon as the army takes it in `0x575A4B`, before the player picks what to do with it.
  - Only five places write the count: the two constructors (0), an initialiser at `0x5B152D` (10, which is past the countdown; probably for a new settlement), loading a save (`0x5B1BCE`), the settlement's turn start (`Settlement::start_turn`) and the owner change.
  - So none of the three choices touches it, and each of them leaves the new owner the same 30%, 25%, …, 5%.
  - The choices still change public order through the other lines, mainly the smaller population (squalor) and, possibly, the culture share; that part isn't checked (NYD).

Besides the countdown, every change of owner also does these things to the settlement:

- **The tax rate goes back to normal** (`m_tax_level = 1`), whatever the old owner had set.
  - So a captured city starts with no low-tax order bonus (slot 4) and no high-tax unrest (slot 14).
- **Games or races are set again:** games (setting 0) if its buildings can stage games, otherwise races if they can stage races, otherwise the setting stays as it was.
- **Its rebels are set** (`set_rebels`, `settlement+0xb4` and `+0xb8`), which is probably who rises if it revolts:
  - When an army takes it, they're the capturing army's (`army+0x164`, `+0x168`).
  - When it goes without a captor to a faction with a flag in its `descr_sm_factions` entry (`description+0xa7`, probably the rebels), and it has no rebel faction yet, they're the region's rebel type and the faction `descr_regions.txt` names for it.
  - When the old owner has that flag, they're cleared first.
- **After a trade (reason 5), it remembers the old owner** (`settlement+0x98`; 21 otherwise); what reads this wasn't followed.
- **The old owner is checked for having lost everything** (`0x4CF14F`) when it has no settlements left (`faction+0x178` and `+0xa8` both 0).
- **Its port goes to the new owner too** (`settlement+0x184`, probably the port), unless a fleet in it, of a faction that isn't at war with the new owner (`Faction::stance` isn't 600), holds it (`Army::0x72309A`, not identified).

The callers pass these reason codes:

| Code | When | Caller |
|---|---|---|
| 1 | An army takes the settlement | A character action (`0x575A4B`) |
| 2 | The settlement revolts | The riot and revolt check (`Settlement::check_riots` → `Settlement::revolt`, `0x5B38D1`) |
| 5 | The settlement is traded in a diplomatic deal | The deal's region transfers (`0x59C247`); also a virtual function at `0xDE8088` (not identified) |
| 6 | All of a faction's settlements go to another faction at once | `0x4CE245` (not identified when) |
| 7 | The settlement is bribed | The deal's bribe (`0x59C640` → `0x571842`) |
| 9 | Not identified | `0x5B8812` |

### The rebel chance

The rebel chance isn't random.

1. **Where it comes from:** each region's line in `descr_regions.txt` names a faction and a rebel type.
   - The region keeps the faction as `Region::m_creator_faction` (`+0x68`; 21 when there's none) and the rebel type's name as `Region::m_rebel_type` (`+0x1c0`).
   - At the start of each of the owner's turns, `SettlementDD8::start_turn` looks that rebel type up in `descr_rebel_factions.txt` and keeps its `chance`, raised to 0 if it's negative.
2. **When it counts:** `compute_order` adds the chance whenever the owner's ID isn't the region's faction (and the settlement has a region).
   - It doesn't matter how the owner got the settlement, or how long ago.
   - Only the faction `descr_regions.txt` names for the region is spared, whoever held the region at the start of the campaign.
3. **What it adds:** the chance is a number of 5% units, added in full every turn, so a rebel type with `chance 2` adds 10% for as long as the settlement isn't that faction's.
   - It shares the 16-unit (80%) cap with the countdown, the spies and the governor.
4. **Its name:** despite the name, the order formula doesn't roll anything with it.
   - This is the only read of the `chance` field found; the code that passes the rebel type on (to raise rebel armies, `0x5B38D1` and others) wasn't followed.

### Catching the spies

At the start of each of the owner's turns, `Settlement::start_turn` calls `Settlement::check_spies` (`0x5B69CF`), which gives every foreign spy inside one chance of being caught.

1. **The owner's strength** is worked out once per settlement:
   - It takes the owner's spies and named characters on the settlement's tile (`PathFinder::unk_004A99D9`, NYD), which includes the governor.
   - Their Subterfuge (0 to 10 each) is combined as `sqrt(Σ (Subterfuge + 1)²) − 1` (`combined_subterfuge`), or 1 when there are none.
   - The strength is that, plus 1, plus the governor's `PublicSecurity` attribute (from traits and ancillaries; not capped).
2. **The chance** for each foreign spy is `18% × strength ÷ the spy's Subterfuge`, rounded down (`Faction::catch_spy`).
   - The roll is `Random::percent()` (0 to 100, so 101 values), so a chance of 100 catches the spy 100 times in 101, and only a chance of 101 or more always does.
3. **A caught spy is never executed.** The code picks between two outcomes, but the test for the execution one (`roll ≥ 80 + chance ÷ 5`, while also `roll < chance`) can never pass.
   - So every caught spy gets outcome 3, which goes through the spy's action queue (`Character::unk_005840D0`, NYD; probably throwing it out of the settlement).
   - Either way, the game records a diplomatic incident between the two factions (`DiplomacyMatrix::unk_005A1417` with code `0x1E`, NYD) and shows a message (`fn_008B1C3F`, in [`code/faction_event.cpp`](../../src/code/faction_event.cpp), through `0xCBC87A`, NYD).
     - For outcome 0 (execution) that function also runs `0xCB09C7` on the spy (NYD), but that outcome never happens.
4. **Quirks:**
   - A settlement with **nobody** of the owner's inside has strength 2, but one whose only defender is a governor with Subterfuge 0 and no `PublicSecurity` has strength 1, so a plain governor **halves** the chance of catching a spy.
   - A spy with Subterfuge 0 is never caught: the division by 0 makes the chance negative.
   - The search for the spies (`PathFinder::unk_004A9C59`) is the unrest one's twin, so a spy that adds unrest is also one that can be caught.

| Strength | Who's inside (examples) | Spy Subterfuge 1 | 2 | 3 | 5 | 10 |
|---|---|---|---|---|---|---|
| 1 | A governor with Subterfuge 0 | 18% | 9% | 6% | 3% | 1% |
| 2 | Nobody | 36% | 18% | 12% | 7% | 3% |
| 4 | One spy with Subterfuge 3 | 72% | 36% | 24% | 14% | 7% |
| 6 | One spy with Subterfuge 5 | 100% | 54% | 36% | 21% | 10% |

So the better the enemy spy, the more unrest it adds (up to 10 units, 50%) and the harder it is to catch: a Subterfuge 10 spy in a city with only a plain governor is caught 1% of the time.

## Why unrest seems to come out of nowhere

None of these involve a dice roll in the order formula. They change without anything visible happening:

1. **A spy walks in.** An enemy (or allied) spy inside the city adds its Subterfuge to Unrest, up to 50%. Nothing in the breakdown names the spy.
2. **The governor gains or loses a trait.** Trait triggers fire with a chance each turn (see [trait effects](trait_effects_findings.md)). A new trait can change the governor's Law, Unrest, Squalor, Health, Influence or LocalPopularity, and so five of the lines above at once.
3. **The governor leaves.** Slot 3 drops to 0, and slots 1, 9, 10 and 15 lose the governor's part.
4. **Growth changes.** Boom (slot 7) counts only growth above a threshold, and squalor rises with population, so a good harvest or a plague moves both.
5. **The garrison moves.** Slot 0 depends on the army inside, and an empty settlement gets slot 13's −15%.
6. **The triumph and wonder counters run down** by 1 a turn ([wonders](wonder_bonus_findings.md)).

## Riots and revolts

At the end of the owner's turn (`Settlement::end_turn`, called from the faction at `0x4CBA54`), the game rolls for plague, then calls `Settlement::check_riots` (`0x5B2AC0`), which decides on riots and revolts. It reads the percentage worked out last, not a new one.

Nothing happens when any of these hold:

- The owner is a faction with the `description+0xa7` flag (probably the rebels), or `faction+0x4ec` is non-zero (not identified).
- The owner is a "small AI" faction (`AICheatManager::not_small_ai()` is false; see [difficulty](difficulty_findings.md)).
- `settlement+0x84` is non-zero (not identified).
- The order is **above 65%**.

Otherwise, the settlement **revolts** or **riots**. There's no roll for whether a riot happens: every check at 65% or below that doesn't end in a revolt is a riot.

### The revolt chance

The revolt chance is 0 unless **all** of these hold:

- It has been at least 3 turns since the last revolt (`settlement+0xa80`, which counts up each turn; NYD where).
- Last turn's order (`+0xed8`) was 70% or below.
- It has been at least 2 turns since the change of owner (`+0x1c0` > 1).
- It rioted at the previous check (`+0xa7d`).

When they hold:

```
chance = 80 - order% + 5 * N
if (order% < 50 || no garrison army)
    chance += 30
else if (garrison soldiers > population / 2)
    chance = 0                         // a big enough garrison stops revolts while order is 50% or more
chance = clamp(chance, 0, 90)
revolt if Random::percent() <= chance  // 0..100, so (chance + 1) in 101; a flag at +0xa7c forces a revolt
```

- `N` counts the neighbouring regions held by a faction with the `description+0xa7` flag (probably the rebels).
  - For a civil war revolt (type 2, below), it also counts those held by a faction whose `shadowed_by` or `shadowing` is the owner.
- "Garrison soldiers" here is every soldier of the garrison army (`Army::unk_00722430`), so peasants count in full, unlike in the order's garrison line.
  - It also adds each unit's filled slots at `+0x298`, up to 4 a unit (what they hold isn't identified).
- The roll is skipped when the chance is 0, so a forced revolt is the only way to revolt at 0%.

### What a revolt does

- It resets the 3-turn counter and the forcing flag, and hands the settlement over with reason code 2, raising an army there (`Settlement::revolt`; see [the rebel army](#the-rebel-army)).
- It fires `GovernorCityRebels` when there's a governor and then `CityRebels` (`fn_008B4820`, in [`code/faction_event.cpp`](../../src/code/faction_event.cpp)), and records the revolt in the faction's events when they're being recorded.
- After the handover, it kills 0–10% of the population (rolled), through the same damage function as a riot (below); it kills no characters, and the garrison's roll is 0.
  - Each building is damaged with a 5% chance, losing 10–30 health.
- When the owner's `unk_004584F1()` holds (`faction+0x4ec` non-zero, or `faction+0x4f0` names a faction; not identified), it adds 1 to `settlement+0xa7e`.
- When the new army has a leader, a message is shown (`0xCB42BB`, NYD), and the leader and the characters with his army come into the settlement (`Settlement::unk_ac`).
  - Each one becomes the governor when he's a named character and there's no governor or his Management is higher than the governor's (`Settlement::better_governor`).

### What a riot does

A riot rolls its **severity** with `Random::range(0, 30)`, which includes 30 (each of the 31 values about equally likely). Then, with `low = severity / 3` and `high = severity × 2 / 3` (rounded down):

- **Population:** it kills a percentage rolled from `low` to `high` of the population, but never takes it below a floor (`SettlementDD8::unk_005C8AD7`, not identified).
- **Characters:** each character in the settlement, **the governor included**, is killed with a chance rolled once from `low` to `high` percent.
  - So a riot of severity 30 kills each of them with a chance of 10–20%; one of severity 0 or 1 kills nobody.
  - The killed are marked with cause 2 (`record+0x1c0`), and when one is a named character (a family member), the faction's `+0x8ac` is set to 21 (not identified).
- **Garrison:** the garrison army is passed a number rolled from `low` to `high` (`Army::unk_0071FE0D`, not identified; probably the soldiers it loses).
- **Buildings:** each building is damaged with a chance of `severity` percent, rolled for each building (`SettlementBuildings::damage`).
  - A damaged building loses 10–60 health (out of 100), or 30–100 when the roll is also below `severity − 25`, so only riots of severity 26 or more can do heavy damage.
  - The walls (the `defenses` strand) are never damaged, and riots and revolts also spare the port (`port_buildings`) and the hinterland buildings (names starting `hinterland_`, such as roads and farms).
  - A building with no health left isn't damaged again.
  - At 0 health a building is destroyed, and `BuildingDestroyed` and `GovernorBuildingDestroyed` fire (`Building::damage`, `0x5D755F`, NYD).
  - When any building was damaged, the owner's `unk_004D3E6F(0)` is called (not identified).
- It fires `GovernorCityRiots` (with a governor) and `CityRiots`, shows a message (`0xCB4122`, NYD), and sets the "rioted" flag that makes a revolt possible at the next check.

These rules are in `Settlement::riot_damage` (`0x5B26B0`), the damage function riots and revolts share.

So the sequence is: order falls to 65% or below → riots every turn → from the second riot in a row on, a chance to revolt each turn.

### Who a revolt goes to

`Settlement::revolt_type` (`0x5B33D0`) picks the kind of revolt, taking the first of these that applies:

| Type | When | Probably |
|---|---|---|
| 4 | The region has the `britain` hidden resource, its owner isn't `romano_british`, no other `britain` region is held by a major faction (one with `custom_battle_availability` and without the `+0xa7` flag), and no such last region has changed owner yet (`Campaign::last_britain_region`) | BI's Romano-British |
| 2 | The owner has a `shadowed_by` faction (`descr_sm_factions.txt`) whose state (`faction+0x178`) is 0 or 3; then the same for a `shadowing` faction | BI's civil wars (the empires and their rebel halves) |
| 3 | The owner has a `spawns_on_revolt` faction whose state is 3 | |
| 0 | A dead faction (state 0), other than the owner, has more than 60% of the population (the largest share), and its `+0x4f4` is clear | The old owner re-emerges |
| 1 | The campaign's `+0x2b9` is set | Rebels |
| 5, 6, 7 | The owner has `custom_battle_availability` and not the `+0xa7` flag, and the settlement has an `amphitheatres` building, at its first, second or higher level | A gladiator uprising |
| 1 | Otherwise | Rebels |

- What state 3 means isn't known; 0 seems to be a faction with nothing left.
- No vanilla faction has `shadowed_by`, `shadowing` or `spawns_on_revolt`, so in the Imperial Campaign only types 0, 1 and 5–7 can come up (and 4, if a region has `britain`).
- `Settlement::revolt` works the gladiator type out again, as the amphitheatre's level **plus 4**, where `revolt_type` uses the level plus 5 (see [the rebel army](#the-rebel-army)).
- Type 4 can happen only once a campaign: when the last `britain` region held by a major faction changes owner, for any reason, `Settlement::change_owner` sets the campaign's `+0x2c4` and calls `Campaign::unk_00457C3C` (NYD; probably what spawns the Romano-British).

### The rebel army

`Settlement::revolt` (`0x5B38D1`) hands the settlement over and raises an army in it. From the riot check, it's always called with reason code 2 and no faction given, so the faction comes from the type:

| Type | New owner | Units and names from |
|---|---|---|
| 0 | The dead faction with the largest share | That faction |
| 2 | The owner's `shadowed_by` faction, or its `shadowing` one when it has no `shadowed_by` | That faction |
| 3 | The owner's `spawns_on_revolt` faction | That faction |
| 4 | `romano_british` | That faction |
| 1 | The rebels (the faction index at `0x1181378`) | The region's `descr_regions.txt` faction (its "creator"), with the region's rebel type |
| Gladiators | The rebels | The region's faction, with the `gladiator_uprising` rebel type |

- `revolt` doesn't trust the gladiator type it's given: unless the campaign's `+0x2b9` is set, it looks for the amphitheatre again and sets the type to its level plus 4.
  - So an amphitheatre at its first level (`m_level` 0) gives type **4**, which `revolt` treats as the Romano-British revolt (below), but with the rebels as the new owner; the second level gives 5 and the third 6, so 7 never comes up.
  - That type 4 also sets the campaign's `+0x2c4`, which stops the real Romano-British revolt for the rest of the campaign.
  - This is read from the code and not tested in game.
- With the campaign's `+0x2b9` set (not identified), every revolt that isn't type 0, 2, 3 or 4 is a rebel one.

**Units.** Types 0, 2, 3 and 4 make a list of units before the handover:

- Type 4 lists every unit in `export_descr_unit.txt` of category 0 or 1 (probably infantry and cavalry) that the new owner owns; the others list what the settlement's buildings could recruit for the new owner (`Settlement::recruitable_units`, with the buildings' effects for that faction from `0x5C58A1`, NYD).
  - That's the land units the buildings can train, each with the experience its building gives plus `recruits_exp_bonus`, and the level of `armour` and of its weapon's `weapon_*` effect.
  - A unit whose weapon can be upgraded (bit 0 of the unit type's `+0xd0`) is left out when its weapon effect is below 0.
  - The revolt doesn't ask for the units to be ones the new owner can have, so the list doesn't check the units' ownership.
- Then each entry of the region's `+0x88` list (not identified) adds its unit, rounded down from a float count, with its experience.
- When the list ends up empty, the settlement just changes owner, with no army.
- The rebels and the gladiators get their units from their rebel type instead (`Army::unk_007201D6`, NYD).

**The leader.**

- In a civil war (type 2), some of the characters in the settlement go over (see **Who goes over in a civil war** below).
  - The first of them leads, unless a later one has more command than the first; then the last such one leads.
  - The others' bodyguards join the leader's army, and so do the garrison's units that have no general, and they all go over to the new faction (`Character::unk_0057FAE2`, NYD).
- Otherwise, a new character aged 20–50 is made, with a name from the "units and names" faction above.
  - It's a `named character` (a family member) in a civil war when the new faction's state (`faction+0x178`) is 0 or 3, and for types 3 and 4 when the state is 3; otherwise it's a `general`.
  - A named character is adopted by a man of the faction with fewer than 4 children (preferring a living one), and `OfferedForAdoption` fires.
- For types 2, 3 and 4, the new faction gets the leader through `Faction::unk_004CDFCF` ([`code/faction.cpp`](../../src/code/faction.cpp)), which does something only when the faction is dead (`faction+0x178` is 3):
  - The faction comes back to life, with a new AI.
  - When it has no character with the record flag at `+0x1d8` bit 17, the leader gets bits 17 and 16 (not identified), and becomes the faction leader (`Faction::set_leader`, in [`code/faction.cpp`](../../src/code/faction.cpp)).
    - `set_leader` takes only a record with bit 16, fires `CeasedFactionLeader` for the old leader and `BecomesFactionLeader` for the new one, and sets the new one's bit 3 (the leader flag).
  - The player gets a message ([`code/ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp)): type `0x92` (`fn_00CBB55B`) for a faction with shadowed_by or shadowing, or type `0x96` (`fn_00CBB5FC`) for any other faction but the slavs.
    - The `0x92` message isn't shown when the faction that comes back is the player's own shadowed_by faction.

**Who goes over in a civil war.** `Settlement::find_defectors` (`0x5B4ED3`) picks them:

- Nothing goes over unless the campaign's `+0x2b8` is set (not identified; probably whether civil wars are on).
- Only `named character`s (family members) in the settlement can go over, and only those that aren't the new faction's already, nor have their record's flag 3 set (not identified).
- At most as many can go over as the new faction has room to adopt.
  - That room is the sum, over the new faction's men aged 16 or more with record flag 16 (probably "in the family tree"), of 4 less each one's children (dead men count too).
  - But it's just **1** when none of the new faction's records has flag 17 (perhaps "faction leader"), so a civil-war faction that has only just appeared gets at most one defector.
- The candidates are taken in the order the settlement lists them, until the room runs out, and then sorted by influence, most first (ties keep the later one first).
- Each candidate then goes over when `percent() < chance`, with the chance clamped to 1–90, so from 1 to 90 in 101:

```
chance = 100 - 10 * (loyalty - quarter_influence)
       + 10 per region next to this one held by a faction in a civil war with the owner
       + besiegers
```

- `loyalty` is the character's Loyalty attribute, from 0 to 10, and `quarter_influence` starts at 0 and goes up by a quarter of the Influence of each character that goes over.
  - So a loyal character (10) rarely goes over (1 in 101 to start with), a disloyal one (0) nearly always does (90 in 101, about 89%), and each influential defector makes it likelier for the rest.
- `besiegers` goes up by 10 for each army besieging the settlement whose faction is in a civil war with the owner, **each time a candidate is checked**, and it's never reset.
  - So with one such army, the first candidate gets +10, the second +20, and so on; this looks like a bug (the count was probably meant to be worked out once).
  - "Besieging" is a guess: the settlement's `+0x48` list holds up to 7 armies, and riots can't happen while it has any.
- The civil-war faction's "in a civil war with the owner" test is the owner being the faction's `shadowed_by` or its `shadowing` faction.

**Strength.** `Settlement::revolt_strength` (`0x5B4C85`) works out how strong the army is, before the handover:

```
nearby  = total strength of the owner's armies in this region and the regions next to it
          / how many there are (at least 1)
overall = total strength of all the owner's armies / how many (at least 1)
s = (nearby + overall) / 2
s *= 2 - owner's share of the population / 100   // 1x when it's all the owner's, 2x when none is
if (region+0x1b0) s *= 1.5                        // not identified
if (garrison)
    s = clamp(s, 2, 3 * garrison strength)
```

- "Strength" is an army's `+0x140`, the sum of a value for each unit, rounded (`Army::compute_strength`).
  - Outside battle each unit counts its campaign value (`0x4335A9`, NYD), from its type, soldiers, `+0x298` slots and other state.
  - The strength is 0 while the army's `+0x139` flag is set (not identified).
- Then, by type: a civil war gets `(s − its leader's army's strength) × 0.5`, and the Romano-British get `s × 1.3`; the rest get `s`.
  - Reason code 5 would give `(s − army) × 0.25` and code 9 clamps to 20–100, but the riot check uses neither.
- The army is filled to that strength with between a lowest and a highest number, by settlement level (probably the fewest and most units; `Army::unk_00720D26` and `unk_007201D6`, NYD):

| Settlement level | 0 | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|---|
| Fewest | 3 | 4 | 5 | 6 | 8 | 10 |
| Most | 8 | 9 | 12 | 15 | 20 | 20 |

So a revolt is strongest where the owner keeps big armies nearby and holds little of the population, and a garrison caps it at 3 times its own strength.

**After the handover.**

- Types 3 and 4 also give the new faction a diplomat (character type 2) aged 20–40 in the settlement.
- The handover is `Settlement::unk_0049B15C` with a leader that went over, or `unk_0049B043` otherwise (NYD); both end in `change_owner`.

### Plague

In the same turn step, a settlement without plague catches it when `Random::range(0, 1000)` is below `(G8 − 3 − 2·G3) / 2`, where G8 and G3 are growth factors 8 and 3 (squalor and health, from `SettlementDD8::compute_growth`), and only when G8 is above 3. Plague adds a 20-point growth penalty (growth factor 9). It isn't one of the order slots, so its effect on order comes only through growth and population. See [plague](plague_findings.md) for the rest.

## Not verified

- The names of slots 0–15 come from what each one reads; the game's own names (the `SOF_*` modifiers in `descr_settlement_mechanics.xml`) weren't matched to slots, except Triumph and Wonder.
- What `settlement+0x84` (which stops riots) and slots 8, 17 and 18 are.
- When reason codes 6 and 9 are used, and what calls `0xDE8088`.
- Whether `descr_regions.txt`'s faction is what the game calls the region's "creator" faction (the name `creator_faction` is a guess).
- What the faction state at `faction+0x178` is, beyond 0 (probably dead).
- In-game checks:
  1. Put your own spy in your city and an enemy's spy in it (by save editing or by watching an AI spy walk in), and compare the Unrest line.
  2. Hold a city at 65% and at 70%: it should riot every turn at 65% and never at 70%.
  3. Leave your own spies in enemy cities for many turns: none of them should die from being caught, only be thrown out (and a Subterfuge 0 spy should never be caught).
  4. Compare how often an enemy spy is caught in a city with a plain governor and in the same city with nobody inside.


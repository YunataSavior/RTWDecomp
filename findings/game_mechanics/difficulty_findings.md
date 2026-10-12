# What campaign difficulty actually changes

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short version:** campaign difficulty is a set of hardcoded "cheat" settings in `RomeTW.exe`, keyed on the **human player's** campaign difficulty.

- On Hard and Very Hard, AI factions get hidden public order, extra population growth, a cash payment every turn and more aggressive targeting of the player, and the player's own tax and farm income is cut.
- On Easy, the player gets more income, and the AI gets less public order and growth.
- None of it shows in the settlement breakdowns, except the player's "Boom" public order term (NYD).
- The AI's 1.1× income multiplier and the per-turn cash for "small" AI factions apply **even on Easy**.

Sources:

- The decompiled cheat manager, [`src/code/ai/ai_cheat_manager.cpp`](../../src/code/ai/ai_cheat_manager.cpp) (`AICheatManager`, matching the original).
- The decompiled save game code, [`src/code/romans_file.cpp`](../../src/code/romans_file.cpp) (matching the original), one of its callers.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for its other callers, which aren't decompiled yet (NYD).

Addresses are virtual addresses in that exe (image base `0x400000`). This is the original CA engine, not `REX.exe`, a third-party mod that modernises the engine and may behave differently.

Nothing here is in a data file:

- `data/descr_difficulty.txt` is read only by `REX.exe` (NYD).
- The faction-select tooltip only says "this relates to income levels".

Difficulty levels: **0 Easy, 1 Medium, 2 Hard, 3 Very Hard.**

## Where difficulty is stored (NYD)

- The setup screen writes the chosen campaign difficulty into each player slot (`slot+0x10`; 0x1e4-byte slots, an array at game object `+0xe4`).
  - The battle difficulty goes in `slot+0x14`.
- At campaign start (`0x446920`), every faction's entry in `campaign+0xc0 + faction*4` is set to **1 (Medium)**, then the human slots get their chosen value (`0x446A9A`).
  - **So AI factions are always "Medium" in this array.**
- The getter is `difficulty(faction)` (`0x4529E3`) on the campaign object (`[0x18CD818]`).
  - It has exactly five callers: `AICheatManager::update()`, and the two pairs under [Other direct uses](#other-direct-uses-of-the-difficulty-value-nyd).

## `AICheatManager::update()` (`0x4FC0A8`)

It's called at the end of `load_game` (`0x413BD6`, the call at `0x413E6F`), so the settings are worked out again whenever a saved game is loaded. It's also called from `0x452ACF` and `0x453BE2` (probably campaign start and turn processing; NYD). It takes the first human faction (`human_faction(0)`, `0x457158`) and switches on **that faction's** difficulty, treating anything other than 0, 2 or 3 as Medium.

| Global | Easy | Medium | Hard | V.Hard | Read by | Effect (NYD) |
|---|---|---|---|---|---|---|
| `s_ai_population_growth` | −1 | 0 | +2 | +4 | `ai_population_growth` | AI population growth |
| `s_ai_public_order` | −2 | 0 | +4 | +6 | `ai_public_order` | AI public order |
| `s_ai_extra_count` | 0 | 0 | 1 | 2 | `ai_extra_count` | AI only; not decoded (`0x7B7C98` uses value+1 as a count) |
| `s_ai_value_against` | 0 | 0 | 10 | 30 | `ai_value_against` | No callers found |
| `s_ai_turn_cash` | 0 | 200 | 400 | 700 | `ai_turn_cash` | AI per-turn cash |
| `s_boom_threshold` | 4 | 6 | 8 | 10 | `boom_threshold` | The player's "Boom" threshold (always 4 for the AI) |
| `s_war_filters` | 0 | 1 | 1 | 1 | `may_target`, `flag_war` | AI war-target filters |
| `s_target_player_bonus` | 0 | 0 | 0 | 1 | `target_player_bonus` | +5000 AI target score against the player |
| `s_unknown_10da998` | 0 | 0 | 1 | 1 | `unknown_10da998` | Not decoded |
| `s_unknown_12d2f92` | 0 | 1 | 1 | 1 | `unknown_12d2f92` | No callers found |
| `s_player_objective_bonus` | 0 | 0 | 1 | 1 | `0x4EA38A` | AI objective bonus against player-owned targets |
| `s_unused_scale` | 1.0 | 1.0 | 1.2 | 1.4 | `unused_scale` | No callers found |
| `s_ai_brigand_scale` | 1.0 | 1.0 | 0.88 | 0.8 | `brigand_spawn_scale` | Brigand spawns in **AI** regions |
| `s_player_brigand_scale` | 1.0 | 1.0 | 1.12 | 1.2 | `brigand_spawn_scale` | Brigand spawns in **player** regions |
| `s_player_tax_scale`, `s_player_farm_scale` | 1.2 | 1.0 | 0.92 | 0.85 | `tax_income_scale`, `farm_income_scale` | **Player** tax and farm income |
| `s_ai_income_scale` | 1.0 | catch-up | catch-up | 1.0 | `tax_income_scale`, `farm_income_scale` | AI tax and farm income are 1.1 × this |
| `s_war_turns` | 20 | 10 | 4 | 2 | `punish_long_war` | Turns at war before the strongest neighbour targets the player |
| `s_player_target_value` | 1.0 | 1.8 | 2.2 | 3.0 | `player_target_value` | AI value of **player** settlements as targets |

Notes on the table:

- The values and accessors come from the matching source; the Effect column comes from the accessors' callers.
- The **catch-up factor** (Medium and Hard only) is `1 + clamp(0.1 × (N − 10) / 20, 0, 0.2)`, where N is the player's region count.
  - It applies only when N > 10 **and** the map has more than 30 regions, which is always true on the imperial map (NYD).
  - The maximum is 1.2, at N ≥ 50.
- `s_player_target_value` takes the values shown only when the player has a positive value in the per-faction AI table at `0x12D0F78 + faction*64`.
  - Otherwise it's 1.0 / 1.0 / 1.0 / 1.5.
- The accessors check the faction first: those marked "AI" return the neutral value (0 or 1.0) for human factions, and some also for the rebel faction (`[0x1181378]`).
  - `ai_population_growth` and `ai_public_order` exclude the rebels, while `ai_extra_count` and `ai_value_against` don't.

### Per-faction flags (`s_flags`, `0x12D2E70`)

`update()` rebuilds one `FactionCheatFlags` (6 bools) per faction, relative to the human player P:

| Byte | Member | Meaning |
|---|---|---|
| +0 | `human` | The faction is human (`faction+0xb8 == 1`) |
| +1 | `at_war_and_bordering` | At war with P (stance 600, the value `at_war_with` sets) **and** borders P |
| +2 | `allied` | Its stance with P is 0 (allied) |
| +3 | `bordering` | Borders P (P's neighbour bitmask `faction+0x13c`, built from region adjacency in `0x4CB391`; NYD) |
| +4 | `strongest_neighbour` | P's strongest neighbour (the pair table `0x12D1552`, set in `0x4E18FA` from the strength metric at `0x12D0F74`; NYD), with extra conditions |
| +5 | `small_ai` | A "small AI faction": AI, `campaign+0x2e4 < 100`, `0x445B2B` true, `[faction+0xb4]+0x18 == −1`, and no more than half P's regions |

The exact meaning of the `small_ai` fields isn't confirmed. With no human faction, all flags are cleared.

## Effects, per area

### Public order (NYD)

The public order function is `0x5CC9D7`. One unit is 5%: `order% = 100 + 5 × units`, clamped to [−20, 80] units.

| | Easy | Medium | Hard | V.Hard |
|---|---|---|---|---|
| AI settlements: a hidden flat modifier (`0x5CDA88`, added after all factors, not in the breakdown) | **−10%** | 0 | **+20%** | **+30%** |
| Player: "Boom" (`SOF_BOOM`) counts only growth above this threshold (`0x5CCDBE`) | 4 | 6 | 8 | 10 |
| AI: Boom threshold | 4 | 4 | 4 | 4 |

The AI modifier doesn't apply to the human player or the rebels.

### Population growth (NYD)

In `0x5C7DED`, at `0x5C7EF0`, the AI-only value is added to the summed growth points after they're clamped to [−200, 25]. One point is 0.5% per turn.

| | Easy | Medium | Hard | V.Hard |
|---|---|---|---|---|
| AI settlements, extra growth per turn | −0.5% | 0 | **+1.0%** | **+2.0%** |

### Settlement income (NYD)

Settlement income is built in `0x5CDB6A`.

| | Easy | Medium | Hard | V.Hard |
|---|---|---|---|---|
| Player: `SIF_TAXES` and `SIF_FARMS` multiplier | **×1.2** | ×1.0 | ×0.92 | ×0.85 |
| AI: the same two lines | ×1.1 | ×1.1 × catch-up | ×1.1 × catch-up | ×1.1 |

- Taxes: the base is `√population × 40 × multiplier`, which feeds a further formula that also takes buildings and the tax rate.
- Farms: `(fertility + 100) × 80 × … / 100 × multiplier`.
- Mining, trade and the other lines aren't touched.

### AI treasury: cash every turn

In the faction's end-of-turn finance update (NYD; `0x4D8197`, `this = faction+0x500`; `+0x504` is the treasury and `+0x508` the turn's income), after the normal income is added:

```
bonus = AICheatManager::ai_turn_cash(faction)
if bonus != 0:
    treasury += bonus × num_settlements + rand(0..100)
```

`num_settlements` is `faction+0x114`, the size of the faction's settlement list (NYD). `ai_turn_cash()` (`0x4FC961`), with `base` = `s_ai_turn_cash` (0 / 200 / 400 / 700), checks these cases in order:

| Case | Result |
|---|---|
| Human | 0 |
| `has_cash_crisis()` (`faction+0x4ec != 0`) | base + 3000, plus the deficit if the treasury is negative |
| The `small_ai` flag | base + 1000 (**also on Easy**) |
| Rebels | 0 |
| Otherwise | base × scale, where scale is 1.0 for ≤5 regions, then 0.9 / 0.8 / 0.7 / 0.6 for 6–9, and 0.5 for ≥10 |

So a rebel faction in a cash crisis, or flagged `small_ai`, would get the bonus too.

The normal case per turn, before the random part:

| Regions | Medium | Hard | V.Hard |
|---|---|---|---|
| 1 | 200 | 400 | 700 |
| 3 | 600 | 1,200 | 2,100 |
| 5 | 1,000 | 2,000 | 3,500 |
| 8 | 1,120 | 2,240 | 3,920 |
| 10 | 1,000 | 2,000 | 3,500 |
| 20 | 2,000 | 4,000 | 7,000 |

The AI's budget planner (`0x4FEEC2`) forecasts income with the same value (NYD).

### Brigands and pirates

Difficulty only touches the land (brigand) side, through `brigand_spawn_scale(faction)` (`0x4FCABE`), called A8 below:

- It's 1.0 / 1.0 / 1.12 / 1.2 in the **player's** regions, and 1.0 / 1.0 / 0.88 / 0.8 in **AI** regions.
- When the campaign's prologue flag (`campaign+0x2b5`, `is_prologue()`) is set, it's 0.33 everywhere.
  - That flag also changes the tax and garrison formulas (NYD).
- A8 scales both the brigand spawn chance and the spawned army's size (NYD).

The full spawn system is in [Brigand and pirate spawning](#brigand-and-pirate-spawning-nyd) below.

### AI war targeting and aggression

The war-target code is around `0x515070`, `0x51578F` and `0x516D1D` (NYD). The accessors' logic comes from the matching source.

| Accessor (caller) | Difficulty | Effect |
|---|---|---|
| `target_player_bonus` (`0x516F8D`) | **V.Hard only** | The AI's target score for the player gets **+5000**, unless that AI is allied with the player (NYD) |
| `punish_long_war` (`0x516104`) | 20 / 10 / 4 / 2 turns | Once the player has been at war with anyone for more than N **consecutive** turns, the player's strongest neighbour gets **+800** priority on the player (NYD) |
| `player_target_value` (`0x50C3BE`) | ×1.0 / 1.8 / 2.2 / 3.0 | The value of a **player-owned** settlement as an invasion target (base value from `0x4E4C70`: settlement level 5 / 30 / 60 / 80 / 100 / 120, plus population and other terms; NYD) |
| `may_target` (`0x5158AC`) | Medium and up | An AI not allied with the player won't target a neighbour that's at war with and bordering the player, so in effect the player's enemies are left alone |
| `flag_war` (`0x51530C`) | Medium and up | When an AI is at war with a faction that's fighting the player, it sets a flag on that war (meaning not decoded; NYD) |
| `player_objective_bonus` (`0x4EA38A`) | Hard and up | An AI objective whose target belongs to the player gets an extra 400 (via `0x4EDE03`); exact meaning not confirmed (NYD) |

- The consecutive-turns counter (`s_player_war_turns`) is kept by `update_war_turns()`, reset whenever the player is at peace, and saved with the game.
- `may_target` has further rules that don't depend on difficulty:
  - An AI can't target the player's strongest neighbour unless it's allied with the player.
  - The player's strongest neighbour can only target human factions.
  - A faction at war with and bordering the player can't be targeted while its value in the table at `0x12D0F78` is 0 or less.
  - Between AI factions, a target that's in a cash crisis (or has a faction at `+0x4f0`, not identified) is left alone by attackers with fewer than 7 regions, and by all of them if the target is `small_ai`.
  - The rebels can always be targeted.

### Other direct uses of the difficulty value (NYD)

- **Agent and roll-based actions** (`0x60EC39`): the target owner's difficulty adds −4 / 0 / +2 / +4 (table `0xFECD34`) to the roll's value, which is then clamped to [−10, 20] and scaled `×10 + 150`.
  - AI factions are always Medium here, so only rolls against the **player's** assets shift.
  - Which side benefits wasn't traced.
- **Wealth ratio** (`0x571BB9`, used by `0x572174` and `0x572395`): the treasury divided by a reference of 70,000 / 40,000 / 35,000 / 30,000, capped at 200,000.
  - It uses the owning faction's difficulty, so AI factions always get 40,000.
  - Its purpose isn't decoded.

## Brigand and pirate spawning (NYD)

All of it is in one per-turn routine, `0x4B31E9`. The army itself is created by `0x4B406C`.

### The `descr_strat.txt` values

`brigand_spawn_value` and `pirate_spawn_value` are optional floats:

- They're parsed at `0x44D944` and stored at `campaign+0x2bc` and `+0x2c0`.
- They **default to 10** when missing (`0x446778`).
- They're read only by this routine; higher means rarer, roughly in inverse proportion.

Stock Barbarian Invasion uses 10 for brigands and 12 for pirates. The stock imperial campaign's values weren't checked.

### How a turn's spawning works

1. The routine counts the rebel faction's existing armies (`campaign+0x2d4` is the rebel faction), as **B** brigand armies and **P** pirate fleets.
2. It walks every region on the map (`[0x18CD80C]+0x17130` regions, 0x1d8 bytes each, at `+0x70`):
   - A land region that already contains a rebel army is skipped.
   - A sea region is skipped if it holds more rebel stacks than `region[+0x154] / 1000`.
3. For each region R, it walks R's neighbours N, skipping sea neighbours.
   - Spawning **doesn't stop after one success**: B or P goes up, which lowers later chances.

### Brigands (R is land)

R's owner's faction record must not have flag `+0xa7`, which nothing in the parser appears to set.

```
p = 0.001
  × min(30, distance between R's and N's settlements)
  × (1.0 if the R–N border has spawn tiles, else 0.2)
  × (1 + region[+0xf0] / 20)                  # field not identified
  × A8(owner of R)                            # difficulty, see above
  × (1 + 0.1 × (10 − B))                      # 2.0 with no brigands, 1.0 at 10, 0 at 20 or more
  × 10 / brigand_spawn_value
spawn if rand(0..1) < p
```

- With the default value of 10, settlements 30+ tiles apart, a border with spawn tiles, no brigands yet and A8 = 1, that's **6% per border pair per turn**.
- B = 20 or more stops brigand spawning.
- A very large `brigand_spawn_value` (such as 123456789) makes p effectively zero.

The army is placed on a border tile (up to 25 positions are tried) and created by `0x4B406C`:

- **Size:** `v = rand(0..1) × A8²`; v < 0.5 gives 1–5 units, < 0.7 gives 1–6, < 0.9 gives 1–7, < 0.95 gives 2–7, and otherwise 2–8.
- **Leader:** with 4 or more units, there's a 1-in-3 chance it's led by a named character.

Unit counts by region owner:

| Region owner (A8²) | 1–5 units | 1–6 | 1–7 | 2–7 | 2–8 |
|---|---|---|---|---|---|
| Any, Easy or Medium (1.0) | 50% | 20% | 20% | 5% | 5% |
| Player, Very Hard (1.44) | 35% | 14% | 14% | 3% | **34%** |
| AI, Very Hard (0.64) | 78% | 22% | – | – | – |

### Pirates (R is sea)

Difficulty doesn't enter here. The gates:

- R's sea field `region[+0x154]` must be above 40; it's probably a traffic or trade value, but it isn't identified.
- There must be fewer than 16 pirate fleets overall.
- N must be a land region.

Then, per (R, N) pair per turn:

```
k = 1    if N's owner's record has flag +0xa7   (apparently never set)
    2.5  if N's owner's record field +0x18 != −1  (a faction index; possibly the protectorate overlord, not confirmed)
    8    otherwise
spawn if rand_int(0 .. round(pirate_spawn_value × k) − 1) == 0
```

- With the stock BI value of 12, that's 1 in 96 per pair per turn normally, or 1 in 30 when `+0x18` is set.
- A random sea tile on the R–N link is chosen, and the fleet is 1–3 ships.

## What wasn't found (NYD)

- No free units, no upkeep discount, and no stat or experience bonuses for AI armies on the campaign map.
- No difficulty-based autoresolve modifier among these consumers; battle difficulty (`slot+0x14`) wasn't followed.
- The accessors' caller lists come from direct `call`/`jmp` scans.
  - Calls through function pointers would be missed, but no accessor's address appears anywhere else in the image, apart from one coincidental byte match inside an instruction at `0xAC8BD6`.

## Not verified

- There are no in-game tests yet. Quick checks that would confirm the main claims:
  1. On Very Hard, compare an AI faction's treasury between turns with its income on the finance scroll; the gap should be about `350–700 × settlements`.
  2. Easy vs Very Hard on the same save: the player's tax and farm income lines should differ by 1.2 / 0.85.
  3. On Very Hard, an AI settlement's public order should be 30% above what its visible breakdown adds up to.
- The unidentified fields: `campaign+0x2e4`, the table at `0x12D0F78`, region `+0xf0` and `+0x154`, and the faction record's (`faction+0xb4`) flag `+0xa7` and field `+0x18`.
  - Record `+0xa4` is `custom_battle_availability`.
- Whether the `rand(lo, hi)` helper `0xF3ABB0` includes its upper bound, which affects the unit and ship counts above.

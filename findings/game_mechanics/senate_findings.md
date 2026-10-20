# How the senate works

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short answer:** each Roman family has a **senate standing** and a **popular standing**, both from 0 to 99, worked out again every turn.

- **Popular standing** comes from the family's average public order, its share of the map's regions, its share of the wonders, and its characters' PopularStanding.
- **Senate standing** starts from 50.
  - It rises while the family holds up to half of Rome's military strength and falls once it holds more.
  - It rises with middling popularity and falls with high popularity.
  - It follows the human's record of senate missions.
  - It adds the faction leader's SenateStanding (0 to 4) and the senate's appreciation, less its displeasure.
- An AI family the senate likes (standing above 80) is sometimes given a legion's first cohort, once the [Marian reforms](marian_reforms_findings.md) have happened.
- **Offices go to each family's three best men**, picked at random by Electability, Influence and the family's popularity; see [How offices are filled](#how-offices-are-filled).
- **Only the human's family is given missions**, picked at random by weight from the senate's plans for the other factions; see [Missions and rewards](#missions-and-rewards-nyd-in-part).
  - The plan for a faction comes from how the senate sees it and how strong the Roman armies near it are against its own; see [How the plans are set](#how-the-plans-are-set).
- **Most of the tuning in `descr_senate.txt` does nothing.** The four attitude coefficients, the six attitude boundaries and `probability_of_random_mission` are read, but nothing in the game reads them back.
  - Neither do three of the four city-mission bounds, and the appreciation and displeasure durations start dates that nothing checks.
- **A mission's reward or penalty is drawn at random from those of its table's code**, and several don't do what their names suggest; see [Rewards and penalties](#rewards-and-penalties).
  - The fines are the wrong way round: `minor_fine` takes the whole of a turn's net income, and `major_fine` a tenth.

Sources:

- The decompiled [`code/ai/ai_senate_faction.cpp`](../../src/code/ai/ai_senate_faction.cpp): the `descr_senate.txt` and office parsers, the standings, the senate's gift and its turn.
- The decompiled [`code/sm_demand_suicide.cpp`](../../src/code/sm_demand_suicide.cpp): the Demand_Suicide mission.
- The decompiled [`code/senate_mission.cpp`](../../src/code/senate_mission.cpp): the rewards and penalties, the mission part of `descr_senate.txt`, and the classes every mission builds on.
- The decompiled missions step and its candidates, the plans and the offices step, in `ai_senate_faction.cpp` too.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the rest, marked NYD.
- Addresses are virtual addresses in that exe (image base `0x400000`).

## Who's who

- The senate is an AI faction (`AISenateFaction`) whose faction slot is **3**.
- It treats slots **0, 1 and 2** as the three families: the Julii, the Brutii and the Scipii, in the faction order of the original campaign.
  - These numbers are fixed in the code, so a mod that reorders `descr_sm_factions.txt` changes which factions the senate treats as families.
- The family the human plays (`senate+0x2f8`) gets the advisor's comments, no gifts, and all the missions.

## Popular standing

`SenateFamily::update_popularity` (`0x5290FB`) works it out from scratch each turn:

```
order   = average of the family's settlements' public order percentages, kept to 0..200
target  = all the map's regions * popularity_land_percentage / 100     // the option is 27 by default
land    = regions the family holds * 100 / target
wonders = wonders the family holds * 25 / all wonders                  // 0 when the map has none
people  = average PopularStanding of all the family's characters, rounded towards 0

popularity = order / 10 + land / 2 + wonders / 2 + people
```

- `popularity` is kept from 0 to 99.
  - When the campaign's `+0x2b4` flag is set (not identified), it's kept to 65 at most.
- So 100% average order gives 10 points, and holding 27% of the map's regions gives 50.
- `popularity_land_percentage` is read from the command line or the config (`0x40A4C8`).
- When a family's popularity rises above 66, the family gets a message of type `0x82` (`fn_00CBAE4F`, in [`code/ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp)).
- A family that's out of the game (`faction+0x178` of 2 or more) gets 0.
- The same step lowers the family's appreciation and displeasure by 2 each turn (see below).

## Senate standing

`AISenateFaction::update_senate_standing` (`0x52FBAF`) also works it out from scratch each turn:

```
standing = 50                                 // kept scrambled in memory
         + strength term
         + popularity term
         + missions term
         + the faction leader's SenateStanding, kept to 0..4
         + appreciation - displeasure
```

`standing` is kept from 0 to 99.

- It's **0** while the family is at war with the senate (faction 3), or out of the game.
- The 50 is stored scrambled (`0xFBD6AAC9`, decoded by `0x4247FE`).
  - That's perhaps to stop memory editors from finding it.

**Strength term.** This is the family's share of Rome's military strength: the AI's total strength of its armies and fleets, out of the senate's and every family's that isn't at war with the senate.

| Share | Under 33% | 33–39% | 40–44% | 45–49% | 50–56% | 57–64% | 65% or more |
|---|---|---|---|---|---|---|---|
| Points | 0 | +8 | +16 | +24 | −8 | −16 | −24 |

- The strengths are the AI's totals of each army's and fleet's strength (`0x4E282B` and `0x4E28AA`, the same figure the revolt code uses).
- So the senate likes a family that carries a good share of Rome's armies, and turns sharply against one that has more than the rest put together.

**Popularity term.** This uses the family's popular standing from the same turn.

| Popularity | Under 30 | 30–38 | 39–47 | 48–56 | 57–65 | 66–74 | 75 or more |
|---|---|---|---|---|---|---|---|
| Points | 0 | +6 | +12 | +6 | −6 | −12 | −18 |

So the senate likes a family the people know but don't love too much.

**Missions term.** This is `8 × succeeded / (succeeded + failed) − 4`, rounded down, so from −4 to +4, and 0 before any mission ends.

- The counts are the senate's own (`senate+0x310` and `+0x314`), and `AISenateFaction::mission_finished` (`0x52B168`) adds to them.
- Missions go only to the human's family, but every family's standing uses these counts.
  - So the AI families' standings would rise and fall with the human's record. This is read from the code and not tested in game.
- When a mission ends, the trigger event `LeaderSenateMissionSuccess` or `LeaderSenateMissionFailed` fires for the faction leader.

**Appreciation and displeasure.** These are two counters on each family (`+8` and `+0xc`).

- Each goes down by 2 a turn while it's above 0, so an odd value ends at −1, not 0, and stays there.
  - A leftover −1 would cost 1 point of standing for good (appreciation) or give 1 (displeasure).
- No mission reward or penalty sets them, not even `senate_gratitude` or `senate_displeasure` (see [Rewards and penalties](#rewards-and-penalties)); what does set them is NYD.
- `descr_senate.txt`'s `appreciation_duration` and `displeasure_duration` start two separate dates on the senate, that many turns ahead (`start_appreciation` and `start_displeasure`, from the `senate_gratitude` reward and the `senate_displeasure` penalty).
  - **Nothing checks those dates.** Only the senate's setup, its save and load, and those two functions touch them.
    - This was checked across the whole exe, by searching all code for their offsets, not only in the decompiled code.

## The senate's gift

For each AI family, `SenateFamily::check_gift` (`0x529593`) runs every turn:

- If its senate standing is above 80, and more than 10 turns have passed since its last gift, it has a 49.5% chance (`percent() > 50`, with `percent()` from 0 to 100).
- Then the family gets a unit: `Faction::give_first_cohort` (`0x4D245E`) picks "roman legionary first cohort i" or "ii", 50–50.
  - It gives nothing until the [Marian reforms](marian_reforms_findings.md) have happened (the campaign's `+0x2b6`).
  - The unit appears at the family's capital, as the senate's unit rewards do (see [Where the units appear](#where-the-units-appear)).
- The human's family never gets this gift.

## The advisor

`AISenateFaction::update_families` (`0x52E178`) also has the advisor comment on the human's standings, outside the prologue:

- When the senate standing rises into the 90s, 60s or 30s, or falls into the 40s or 10s.
- The same five for popular standing.
- Each comment waits more than 10 turns before it can repeat.
- The messages are the texts `0x4D0`–`0x4D9` through `0xD93A0C` (NYD which texts), sent to the player as message type `0x87` by `fn_00CBB282` ([`code/ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp)).
  - It passes the senate-or-popular flag once and the rising-or-falling flag twice, as the message's three values.

## descr_senate.txt

`load_senate_description` (`0x529E71`) reads the file once at start-up. Each key must appear in this order, or loading fails:

| Key | Kept as | Used |
|---|---|---|
| `appreciation_duration` | an int | Yes: the senate's appreciation date (see above) |
| `displeasure_duration` | an int | Yes: the senate's displeasure date |
| `probability_of_random_mission` | a float | **No** |
| `turn_coefficient`, `strength_coefficient`, `relative_strength_coefficient`, `combined_strength_coefficient` | floats | **No** |
| `attitude_boundary_1` … `attitude_boundary_6` | floats | **No** |
| `Begin_Offices` … `End_Offices` | the offices (below) | Yes |
| The rest (fines, rewards and each mission's table) | `load_senate_missions` (`0x5470FE`), in [`code/senate_mission.cpp`](../../src/code/senate_mission.cpp) | Yes, except three bounds (below) |

- "No" means only the parser refers to where the value is stored.
  - This was checked across the whole exe, by searching all code for the address, not only in the decompiled code.
- So changing the attitude coefficients or boundaries, or the random mission chance, has no effect.

`load_senate_missions` reads the rest in this order, and fails if one is missing:

1. `major_fine`, `moderate_fine` and `minor_fine`, then `major_reward`, `moderate_reward` and `minor_reward` (see [Rewards and penalties](#rewards-and-penalties)).
2. Each mission's section, found by its name: from the next line, its `Duration` and its table, to its `End` (`SenateRewardTable::load`).
   - Help_Player, Demand_Suicide, Attack_Outlaw_Faction, Give_Back_City, Exact_Tribute, Subjugate, Broker_Peace, Make_Trade_Agreement, Get_Map_Info, Appease, Make_Alliance, Give_Cash, Annex_City and Destroy_Rebel_City have only that.
   - Assassinate, Break_Alliance, Cease_Hostilities, Declare_War, Blockade_Port, Capture_Rebel_City and Take_City are read by their own files' code, which reads their extra keys too, such as `difficulty_boundaries_0` and `reissue_delay_in_turns` (NYD what those do).
   - A table line is an attitude (−3 to 3), a difficulty (`E`, `M` or `D`; any other letter fails) and a code; a line with an unknown code is skipped, and a code not given is 0 (none).
3. `rebel_city_lower_boundary` and `rebel_city_higher_boundary` after Destroy_Rebel_City, and `take_city_lower_boundary` and `take_city_higher_boundary` after Take_City.
   - **Only `rebel_city_lower_boundary` is ever used.** The four functions that give the bounds to the mission code all return it.
   - The bounds split the ratio of strengths that sets a city mission's difficulty (`0x5621DA`, in `sm_take_city.cpp`, used by all the city missions; NYD in detail): below the lower bound it's `D`, at or above the higher one it's `E`, and between them `M`.
   - So with both bounds at `rebel_city_lower_boundary` (0.8 in the file), a city mission is `M` only at a ratio of exactly 0.8.
  - The file's own comment says to tune them with "ai_senate_faction.cpp debug window outputs", so they were probably used by a debugging tool or an earlier design.

### Offices

The office parser (`load_office`, `0x527A49`) reads each office as its name line, then `Rank`, `Quantity`, `Duration` and `Sittings`. Then come `Restrictions` … `End_restrictions`, `Senate_benefits` … `End_senate_benefits`, and `End`.

- The name must be exactly one of these, or the office gets type 6 and an unknown text (`0x51B`):

| Name | Type | Texts |
|---|---|---|
| `Quaestor` | 0 | `0x106`, `0x107` |
| `Aedile` | 1 | `0x108`, `0x109` |
| `Praetor` | 2 | `0x10A`, `0x10B` |
| `Consul` | 3 | `0x10C`, `0x10D` |
| `Censor` | 4 | `0x10E`, `0x10F` |
| `Pontifex Maximus` | 5 | `0x110`, `0x111` |

- `Quantity` makes that many identical copies of the office in the list.
- Restrictions are stored as numbers: `not_consecutively` 0, `quaestor_tenure` 1, `aedile_tenure` 2, `praetor_tenure` 3, `consul_tenure` 4.
  - An unknown word is kept as 5.
- Benefits: `no_censor_suicide` 0 and `censor_immunity` 1.
  - An unknown word is skipped.
- Each copy also keeps its rank, duration, sittings and the date it was made.
- What each benefit does is NYD; what the restrictions do is below.

### How offices are filled

`AISenateFaction::update_offices` (`0x52FFC8`) runs every senate turn, before the missions step.

1. **Terms end.** A holder leaves his office when any of these is true:
   - His term ends today.
   - He is dead.
   - His faction is at war with one of the families or with the senate (`AISenateFaction::at_war_with`, `0x52B8F2`; the same flags as the senate's view of it).
2. **Leaving** fires the `Ceased<Office>` trigger event (`CeasedQuaestor` … `CeasedPontifexMaximus`) and adds the term to the man's career (below).
3. **Nothing more happens unless someone left an office in step 1, or an empty office's term ends today.**
   - An office whose holder left early, by death or war, stays empty until the date his term would have ended.
4. **Offices are filled from the last in `descr_senate.txt` to the first**, so the offices listed later get the first pick of the men.
   - Only offices whose term ends today are filled.
5. **The candidates for each office** come from each family that has a faction, isn't at war as in step 1, and isn't left out by the reward or penalty below.
   - Only that family's named characters (type 5) can stand, and only when the office allows them (below).
   - A man's weight is `popularity / 8 + Electability + Influence`, using his family's popular standing.
   - He must have a weight above 0.
   - Each family puts forward at most its 3 heaviest men.
6. **If no family puts anyone forward**, the senate's own named characters stand instead.
   - Any of them out of office can stand, with weight `Electability + 12`, kept between 1 and 20.
7. **The winner** is picked at random by weight (`Random::range(0, total − 1)`, then the weights are taken off in turn until it falls to 0 or below).
8. **If there are no candidates at all**, a new named character is made for the senate at its capital (`0x8E24E2`) and gets the office.
   - The office stays empty if that fails (for example, when the senate has no capital).
   - Its date has then passed, so the election never fills it again.
9. **Taking office** fires the `Become<Office>` trigger event (`BecomeQuaestor` … `BecomePontifexMaximus`).
   - His term ends `Duration` turns from today.
   - For each of the other families and the senate, a per-faction level on him (or on each man in his army) is raised to 3, its top value (`0x580620` or `0x71EBF6`, through `0x4A332A`; NYD what the level is, perhaps what that faction may see of him).
10. **The player is told** of the offices the human's family's men got (`0xCBB01C`), from the second turn on.

Restrictions and other rules:

- **A man can hold only one office at a time.**
- **`quaestor_tenure` (and the other tenures)** need him to have held that office before, at any time.
- **`not_consecutively`** only stops him from being given the office again on the turn he left it.
  - It looks at his last term only.
- **The election doesn't check `Rank` or `Sittings`.**
  - Other code reads the offices list (`0x52B94D`–`0x52DFDD`), so whether they matter elsewhere is NYD.
- **Each man's career** (his character record's `+0x1b0`, saved as `SENATE_SERVICE_HISTORY`) keeps his current office, since when, and a list of his past terms.

The reward and the penalty about offices (from the reward parser's names, `0x545F5D` and `0x545D05`):

- **`promise_of_appointment`** (a reward) means only the human's family is considered at the next appointments.
- **`offices_witheld`** (a penalty; the code's spelling) leaves the human's family out of them.
- Both last until the next turn that gets past step 3, and then are cleared.
- If both are set, neither applies.
- The senate's setup (`0x52BA7A`; NYD when it runs) sets `offices_witheld`.

## How the senate sees the other factions

Each turn the senate works out a view of every faction (`SenateView`, 21 of them at `senate+0x48`).

- Its flags come from diplomacy with the families and the senate (`update_diplomacy`, `0x52E911`): at war (stance 600), stance 400 (probably hostile), allied (stance 0), and bit 3 of the relation's flags (`0x5A0F3E`).
  - A family also gets a separate flag when it's at war with the senate.
- Its three figures are sums of three figures each family's AI keeps for each faction (`add_family_view`, `0x52E7D2`).
  - They're in a 21-entry table the AI builds at `0x517247`, which is probably its war-target code (the same code as in [AI war targeting](difficulty_findings.md#ai-war-targeting-and-aggression)); what each figure measures is NYD.
- A family's AI "has it in view" when that table's region (`+0x1c`, while `+0x18` is set) is held by the faction, or when the faction's entry in the AI's global strategy director has its `+0x1e` flag set.
- From those, `SenateView::update_state` gives each faction a state:

| State | When |
|---|---|
| 0 | A family at war with the senate |
| 1 (enemy) | At war with a family or the senate; or a family's AI has it in view and its first figure is above its third; or it's in slot 7, 10, 15 or 20 |
| 2 | Stance 400 with one of them, and its first figure is above 0 and above its third |
| 4 (friend) | Allied with one of them; or its first figure is 0, its third is above 0, and it's not at stance 400 with any of them |
| 5 | The other diplomacy flag |
| 3 (neutral) | Anything else |

- Slots 7, 10, 15 and 20 are Carthage, the Gauls, the Greek cities and the rebels in the original campaign, so the senate always counts them as enemies.
- For each family, `choose_enemy` (`0x52EC0D`) then picks the neighbour the senate wants it to fight, or none (21).
  - Only factions whose regions border the family's count, and never dead ones, the ones in a cash crisis or with a faction at `+0x4f0` (`Faction::unk_004584F1`) or those with the description's `+0xa7` flag.
  - Any it's at war with comes first: the one with the highest first figure.
  - Otherwise it's the one with the highest first figure that isn't in state 4 or 5.
  - The missions use it: a city, Assassination or Blockade_Port candidate against that faction gets 5 more weight.

## Missions and rewards (NYD in part)

The missions step, `AISenateFaction::unk_00530A0D` (`0x530A0D`), is decompiled, along with the candidates it picks from.

- `AISenateFaction::update` (`0x52A3D4`) runs it only while there's a human family that's still in the game, so only the human's family is given missions.
- `SenateFamily::standing_level` turns the family's senate standing into a level, `floor(standing × 0.07)`, so 0 to 6.
  - Each mission type's table is looked up by that level and a difficulty column (`E`, `M` or `D`; for example `0x54F8C2` for Blockade_Port), so the tables' seven attitude rows (−3 to 3) are levels 0 to 6.
  - The unused attitude boundaries play no part.
- The reward codes are 0 none, 1 `Outlaw`, 2 `p3`, 3 `p2`, 4 `p1`, 5 `r1`, 6 `r2`, 7 `r3` (from the mission parser, `0x5466C1`–`0x5467DA`).
- The senate keeps a list of the missions it has given (`senate+0x318`), and `AISenateFaction::given_recently(type, family, turns)` is true when it gave a mission of that type fewer than `turns` turns ago.
  - Each entry keeps the type, a faction index and the date.
  - The faction is the human's own for Demand_Suicide, Help_Player and the first turn's Occupy_City, and the target's for the others.
- The rewards and penalties each code gives, and what they do, are in [Rewards and penalties](#rewards-and-penalties).

### The missions step, in order

1. Nothing happens while the human's family is at war with the senate (the senate's stance towards it is 600 or more).
2. The family's missions under way (`faction+0x184`) that can no longer be done are cancelled and dropped (`Faction::check_missions`; see [How a mission ends](#how-a-mission-ends)).
3. The senate judges each of last turn's faction events ([`code/faction_event.cpp`](../../src/code/faction_event.cpp)) that it hasn't judged yet, through the event's own virtual function (NYD what each one does).
4. **Leave_Region** (type 18) is given for each of the family's armies that stands in a region held by a faction the senate counts as a friend (view state 4 or 5).
   - It's skipped when the family has a diplomatic flag with that faction (bit 1 of the relation's flags, probably military access).
   - It's skipped when the senate asked about the same region in the last 3 turns.
   - Its penalty depends on the family's level: `p3` below level 2, `p2` below level 4, `p1` otherwise (`SenateFamily::penalty`).
   - Several can be given in one turn, one for each army.
5. **Demand_Suicide** (type 11) is given on the conditions [below](#demand_suicide-the-senate-asks-the-faction-leader-to-fall-on-his-sword).
6. On the first turn (turn 0), outside the prologue, **Occupy_City** (type 12) is given for a fixed settlement: Segesta for the Julii, Apollonia for the Brutii and Syracuse for the Scipii.
   - The family is matched by its faction name (`romans_julii`, `romans_brutii`, `romans_scipii`), so a mod's renamed family gets no first mission.
   - Its reward depends on the family's level: `r1` below level 2, `r2` below level 4, `r3` otherwise (`SenateFamily::reward`).
7. If a mission failed since the last step, the senate clears that and gives nothing else this turn.
8. It gives nothing else while the family has any mission under way, or in the prologue.
9. Otherwise it lists the candidates its plan for each other faction gives (below), and picks one at random, each as likely as its weight.
   - The roll is 0 to the total weight (`Random::up_to`), and the weights are taken from it in list order until it's 0 or less, so the first candidate is one point more likely than its weight.
   - The chosen one is given with the reward its type's table gives at the family's level and the candidate's difficulty column.
   - When that code is 0 (none), nothing is given this turn.
10. When there are no candidates, **Help_Player** (type 7) is given if it wasn't given in the last 20 turns, it's after turn 10, and the family's level is 4 or more.
    - Its reward is its table's code at the family's level, column `E`, and code 0 gives nothing.

### How a mission ends

A faction keeps its missions under way at `faction+0x184` and the missions it's been offered at `+0x190` ([`code/faction.cpp`](../../src/code/faction.cpp)). A mission under way ends in one of three ways.

1. **It succeeds** when its own type's code sees the goal met.
   - That code gives the reward, with its message, and drops the mission (`Faction::drop_mission`, `0x4D136A`).
2. **It's cancelled** when it can no longer be done, for example when its target is gone.
   - The senate's missions step checks each one (`Faction::check_missions`), and the type's own check (vtable `+0x38`) shows the "cancelled" message.
   - There's no penalty, and the result, `MISSION_CANCELLED`, counts as neither a success nor a failure.
3. **It fails** when time runs out, at the end of the family's turn (`Faction::end_missions`, `0x4D11EE`, from `0x4CB8CC`).
   - Each type's check (vtable `+0x2c`) fails the mission on the turn it has 1 turn left (`turns_left`, its `Duration` less the turns since it was given), and gives the penalty with its message.
   - The pass stops at the first failed mission for which a check on the senate (`0x14` of its second vtable, with the family's index; not identified) is true, and the missions after it wait for the next turn.

Each mission dropped is counted (`AISenateFaction::mission_finished`, which feeds the [missions term](#senate-standing)) and then deleted.

- No mission is counted while the senate's own faction is out of the game (`faction+0x178` of 2 or more).
- At the end of the family's turn in that state, every mission under way is deleted without a reward, a penalty or a message.

An offer the family declines is deleted without being counted (`Faction::drop_offer`); one it accepts joins the missions under way (`Faction::accept_offer`).

After a load, each mission is rebuilt from what the save kept of it, and then sends the family its message again (`Faction::build_missions`, from `0x456DE0`).

### The candidates

Each faction's `SenateView` has a plan (`+0x18`), which picks the missions the senate considers against it ([How the plans are set](#how-the-plans-are-set)).

| Plan | Mission (type) | Target | Weight |
|---|---|---|---|
| 0 | Exterminate_City (15) | Each of its settlements near the family (see below) | 15 − nearness − 3 × difficulty, +5 against the family's enemy |
| 1 | Enslave_City (14) | The same | The same |
| 7 | Occupy_City (12) | The same | The same |
| 2 | Subjugate (21) | The faction | 20 − cost − difficulty |
| 3 | Assassination (8) | Each of its named characters | 19 − difficulty, +5 against the family's enemy |
| 5 | Blockade_Port (19) | Each of its ports that a fleet or a port of the family's reaches in under 5 turns | 15 − turns, +5 against the family's enemy |
| 6 | Broker_Peace (26) | The faction | 20 − cost − difficulty |
| 8 | Annex_City (22) | One of its settlements (see below) | 20 − cost − difficulty |
| 9 | Exact_Tribute (20) | The faction | 20 − cost − difficulty |
| 13 | Appease (25) | The faction | 20 − cost − difficulty |
| 14 | Get_Map_Info (23) | The faction | 20 − cost − difficulty |
| 15 | Make_Trade_Agreement (24) | The faction | 20 − cost − difficulty |
| 4, 10, 11, 12, 16, 18, 20, 21, 22 | None | | |

- Each type's own file gives the difficulty (0 to 2, the tables' `E`, `M` and `D`) and the cost, or difficulty 3 to leave the target out (for example `0x5572F2` in `sm_exact_tribute.cpp`; NYD how each one judges).
  - Blockade_Port's two functions always give difficulty 0, so it's always looked up in column `E`.
- A candidate with a weight of 0 or less isn't listed.
- "The family's enemy" is the neighbour `choose_enemy` picked for it (see [How the senate sees the other factions](#how-the-senate-sees-the-other-factions)).
- A faction whose description has the flag at `+0xa7` gets none of the faction-wide missions (Subjugate, Broker_Peace, Annex_City, Exact_Tribute, Appease, Get_Map_Info, Make_Trade_Agreement), only the city, Assassination and Blockade_Port ones.
- **The city missions' nearness** is the best of the family's regions' routes to the settlement's region.
  - A land route of fewer than 3 steps gives a nearness of twice its steps (`PathFinder::unk_004B13A2`, NYD in detail).
  - An army's march of 2 turns or fewer gives a nearness of its turns (`0x51E6C7`).
  - A settlement that neither reaches isn't a candidate.
- **Annex_City's target is always the settlement of the last region in the faction's list.**
  - The search for the nearest region never lowers its best distance, so every region passes and the last one wins.
- **Make_Trade_Agreement** is a candidate only when one of the regions bordering the family's belongs to that faction, and the two don't trade already (`DiplomacyMatrix::unk_005A0ABE`, probably).
- **Blockade_Port** needs a port with its `+0x3c` above 0, for which `SettlementUnk184::unk_0049E979` is false (neither identified).
  - The route's length comes from the campaign's path finder (`0x49060E` for a fleet, `0x4908F6` from a port), NYD in detail.

### How the plans are set

`AISenateFaction::update_plans` (`0x52EDA6`) sets every faction's plan each turn, right after the views and the families' enemies.

It weighs two ratios:

- **Local ratio:** the strength of the Roman armies near the faction, against its own armies' strength + 1.
  - The strengths are the AI's figures for each faction's armies, the same ones the [senate standing](#senate-standing) uses.
  - The Roman factions are the Julii, the Scipii, the Brutii and the senate, in that order.
  - A family counts only while the senate's stance towards it is below 100, i.e. allied; the senate always counts.
  - "Near" means one of its armies could reach the faction's regions in 3 turns or fewer (`0x51E515`, NYD in detail), or their land borders touch (the AI's border length, `0x4E3FAD`).
  - **Once one of them is near, the ones after it count too**, near or not, because the flags aren't reset for each of them.
- **Rome's ratio:** the senate's armies and those of the families at peace with it, against 1 + the armies of every living faction the senate counts as an enemy (state 1).

The plan then depends on the faction's state (see [How the senate sees the other factions](#how-the-senate-sees-the-other-factions)):

| Faction | Plan |
|---|---|
| The senate itself | 21 (none) |
| The view's `+0x04` diplomacy flag is set, whatever its state | 3, Assassination |
| Dead | 16 (none) |
| State 0 (a family at war with the senate) | 22 (none) |
| State 4 (friend) | 18 (none) |
| State 5 | 20 (none), but never reached: state 5 needs the `+0x04` flag, which gives Assassination first |

**State 1 (enemy):** the first rule that holds wins.

1. If near: fewer than 3 settlements, fewer than 3 characters on the map, and a local ratio above 3 give **Exterminate_City** (0).
2. If near: a local ratio above 2 and slot 16 of the public order of the senate's capital above 0 give **Enslave_City** (1).
   - Slot 16 is [always 0](public_order_findings.md), so the senate never plans Enslave_City.
3. Fewer than 4 settlements, fewer than 5 characters, a local ratio above 1.5, and no `0x5A0F3E` flag with any faction give **Subjugate** (2).
   - This one doesn't need the faction to be near.
4. A local ratio below 0.3, the description's `+0xa7` flag clear, not being any family's chosen enemy, and no Broker_Peace in the last 10 turns give **Broker_Peace** (6).
5. At war with a family or the senate, a port, Rome's fleets stronger than its fleets, and no Blockade_Port in the last 5 turns give **Blockade_Port** (5).
6. Otherwise **Occupy_City** (7).

**State 2 (stance 400):**

1. If near: a local ratio above 3, Rome's ratio above 1.2, and no Annex_City in the last 5 turns give **Annex_City** (8).
2. If near: fewer than 5 settlements, a local ratio above 1.8, Rome's ratio at most 1.2, and no Subjugate in the last 5 turns give **Subjugate** (2).
3. If near and a family's AI has it in view: 12 (none).
4. A local ratio above 1.5 and no Exact_Tribute in the last 10 turns give **Exact_Tribute** (9).
5. Otherwise **Assassination** (3).

**State 3 (neutral):** none (16) unless it's near.

1. Stance 400 with a family or the senate, and Rome's ratio below 0.5, give **Appease** (13).
2. A shared land border (being in reach isn't enough), and either no human family or no trade agreement between it and the human's family (`0x5A0ABE`, probably that), give **Make_Trade_Agreement** (15).
3. More settlements than half of the senate's and the families' together, and no Get_Map_Info in the last 20 turns, give **Get_Map_Info** (14).
4. Otherwise **Assassination** (3).

Notes:

- The "last N turns" checks are `given_recently`, so they look at the missions the human's family was given about that faction.
  - The 5- and 10-turn waits are kept scrambled in the code (`SecureInt`); their values came from running the decoder (`0x4247FE`) on the constants.
- "Characters" are the faction's characters on the map (`faction+0xe8`), and "a port" is any entry in `faction+0x130`.
- The `0x5A0F3E` flag is bit 3 of a diplomatic relation's flags, between living factions whose `+0x178` is 0; what it is is NYD.
- Faction `+0xa8` is its capital (`Faction::set_capital`, `0x4CF09A`, which also moves the settlements' `+0xa88` flag).

### Demand_Suicide: the senate asks the faction leader to fall on his sword

The mission itself is decompiled ([`code/sm_demand_suicide.cpp`](../../src/code/sm_demand_suicide.cpp)), and so is the missions step that gives it.

**When the senate asks.** The missions step offers this mission (type 11) to the human's family when all of these hold:

1. The family's standing level is 0, so its senate standing is below 15.
2. The faction has a character just after its leader (`faction+0xb0`), probably the heir.
3. The senate hasn't given the family this mission in the last 3 turns.
4. A check through the senate's second vtable (`+0x44`, slot `+0x18`) passes (not identified).

**The penalty is always Outlaw.** The step passes the penalty in, as reward code 1, so the mission's table in `descr_senate.txt` isn't used.

- `DemandSuicideBuilder::create` (`0x5556A8`) looks up the table (at the family's level, column `E`) only when it's given no penalty, and makes no mission when the penalty is "none".
- `choose_reward` (`0x54610E`) turns code 1 into the `outlaw` penalty, never the "none" one, so the mission is always made.
- So changing the Demand_Suicide rows in `descr_senate.txt` should change nothing. This is read from the code and not tested in game.

**What answering does** (decompiled):

- Accepting (`DemandSuicideMission::accepted`, `0x55586D`):
  1. Kills the faction leader's character (`Character::kill`).
  2. Records the faction "romans_senate" as the one that killed him (`faction+0x898+0x14`, the same field a riot's deaths set).
  3. Shows message `0xDE` with the faction's name, and closes the mission.
- Refusing (`DemandSuicideMission::declined`, `0x555902`) shows message `0xDD` with the faction's name, and closes the mission.
  - The base mission's code then gives the penalty (`SenateMissionOffer::report_declined`): Outlaw, so **refusing makes the senate declare war on the family**.
- Both close the mission at once, so it's never kept as an active mission (its `Duration` is 0).

Notes:

- Standing that low usually comes from holding 65% or more of Rome's strength (−24) and popularity of 75 or more (−18); see [Senate standing](#senate-standing).
- The Censor's `no_censor_suicide` benefit is a different path (the censor's investigation), not this mission.

## Rewards and penalties

A mission's table gives a code for the family's level and the mission's difficulty, and `choose_reward` (`0x54610E`) turns the code into one of the 24 rewards and penalties.

- Codes 2 to 7 pick at random, each choice as likely as the others except where the table says.
- **The same reward or penalty is never chosen twice running.** The last one chosen, for any mission, is kept, and the draw is made again until it gives another.
- "Losing money" means the turn's figures so far show more costs than income (`FactionFinance::net`).

| Code | In the file | Gives |
|---|---|---|
| 0 | (none) | `null_payback`, which does nothing; no mission is made |
| 1 | `Outlaw` | `outlaw` |
| 2 | `p3` | `censor_investigates_family` or `quaestor_investigates_for_corruption` |
| 3 | `p2` | `censor_investigates_faction` or `quaestor_investigates_for_financial_irregularities` |
| 4 | `p1` | `quaestor_investigates_for_accounting_irregularities`, `senate_displeasure`, `secret_information_exposure` or `offices_witheld` |
| 5 | `r1` | `minor_reward` when losing money; else `throw_games` when one of its settlements is at 70% public order or less; else `one_buildable_unit`, `minor_reward`, `promise_of_appointment`, `senate_gratitude` or `throw_games` |
| 6 | `r2` | `moderate_reward` when losing money; else `one_best_buildable_unit`, `one_minor_exotic_unit`, `moderate_reward` or `one_triarii` (`one_triarii` 2 in 5, the others 1 in 5) |
| 7 | `r3` | `major_reward` when losing money; else `major_exotic_unit`, `two_minor_exotic_units`, `two_best_buildable_units` or `major_reward` |

- **`two_triarii` is never chosen.** It exists, and a save can name it, but no code picks it.
- A mission that succeeds gives its reward (`SenateMissionOffer::report_accepted`), and one that fails gives its penalty (`report_declined`).
  - Either way the message shows what it did.

What each does:

| Name | Effect |
|---|---|
| `outlaw` | **The senate declares war on the faction** (`DiplomacyMatrix::declare_war`, stance 600; the senate's faction is the first in the campaign's list at `+0x2c8`) |
| `censor_investigates_family` | One of the family's named characters, picked at random but never the faction leader, is killed, with the senate recorded as his killer; nothing happens when the family has only one |
| `censor_investigates_faction` | One of its spies, assassins, diplomats or generals (not admirals or family men), picked at random, is killed in the same way |
| `quaestor_investigates_for_corruption` | A fine: the net income divided by `major_fine` (below) |
| `quaestor_investigates_for_financial_irregularities` | A fine: the net income divided by `moderate_fine` |
| `quaestor_investigates_for_accounting_irregularities` | A fine: the net income divided by `minor_fine` |
| `senate_displeasure` | Only starts the senate's displeasure date, which nothing checks, so **it does nothing**, and its message has no text |
| `secret_information_exposure` | Only fires the `SenateExposure` trigger event for the faction leader, so only the traits' and ancillaries' triggers can make anything of it |
| `offices_witheld` | The family is left out of the next appointments (see [How offices are filled](#how-offices-are-filled)) |
| `major_exotic_unit`, `one_minor_exotic_unit`, `two_minor_exotic_units` | One or two units from a fixed list of exotic ones (below) |
| `one_buildable_unit`, `one_best_buildable_unit`, `two_best_buildable_units` | One or two units its buildings can train: one of the dearer half at random, or the dearest (below) |
| `one_triarii` | One "roman triarii" unit |
| `major_reward`, `moderate_reward`, `minor_reward` | That many denari (10000, 5000 and 1000 in the file), booked as income line 6 |
| `promise_of_appointment` | Only the family is considered at the next appointments (see [How offices are filled](#how-offices-are-filled)) |
| `senate_gratitude` | Only starts the senate's appreciation date, which nothing checks, so **it does nothing** |
| `throw_games` | Games are held, for free, in the faction's settlement with the lowest public order (`Settlement::unk_005BC88A`; NYD what games do) |

**The censors' and quaestors' investigations do nothing when the first Censor (or Quaestor) office is empty, or held by one of the faction's own men** (`AISenateFaction::censor_protects` and `quaestor_protects`).

- Then the message says so (texts `0x412` and `0x413`).
- The offices' `no_censor_suicide` and `censor_immunity` benefits play no part in this.

**The fines are a division, not a percentage.**

- The fine is the turn's net income (its first four income lines less its first two cost lines), taken as a positive amount and divided by the file's figure.
  - It's paid from the treasury and booked as cost line 6.
- So a faction that's losing money is fined too, by the size of its loss.
- `descr_senate.txt` calls the figures "percentages of current annual income", but with its figures (10, 5 and 1) the fines are a tenth, a fifth and **the whole** of the net income.
- **So the mildest penalty code, `p1`, can give the biggest fine**, and the harshest, `p3`, the smallest.
- Each investigation also fires a trigger event for the faction leader: `QuaestorInvestigationMajor`, `QuaestorInvestigation` and `QuaestorInvestigationMinor`.

**The unit rewards** come from the faction's own code, in [`code/faction.cpp`](../../src/code/faction.cpp) (`Faction::give_major_exotic_units` and the others):

- A major exotic unit is one of "roman arcani", "roman samnite gladiator", "roman mirmillo gladiator", "merc bastarnae", "merc elephants", "merc horse archers", "merc sarmatian cavalry" and "naval deceres", and after the [Marian reforms](marian_reforms_findings.md) also "roman legionary first cohort i" and "ii".
  - Each is as likely as the others.
- A minor exotic unit is one of "roman velite gladiator", "merc rhodian slingers", "merc greek hoplites", "merc barbarian infantry", "merc numidian cavalry", "merc barbarian cavalry" and "roman scorpion", each as likely as the others.
- **The buildable units are ranked by their cost in custom battles**, the last figure of `stat_cost` in `export_descr_unit.txt`, not by their campaign cost (`Faction::buildable_units`).
  - The list holds each unit type that any of the faction's settlements' buildings can train, sorted from cheapest to dearest.
  - "Best" is the dearest; `one_buildable_unit` picks at random from the dearer half (from index n/2 to n−1 of the n types).
  - General units (`general_unit`) are taken out of the list, but the count isn't lowered, so with general units trainable (a mod; no vanilla building trains one) the picks can land past the list's end.
    - Since `Array::erase` leaves the old items there, the "best" is then always the dearest of the full list, general unit or not.
  - With nothing trainable it gives nothing.
- The message names the last unit made and shows where it is, or says there's none to give (text `0xFA`), which it also does when the second of two can't be made.

### Where the units appear

`Faction::give_units` (`0x4D1BF3`) makes each unit in the capital's region and puts it at the capital; the senate's unit rewards and its gift all use it.

- A ship goes into the capital's port when the port's `+0x3c` is above 0, or else into the first of its settlements' ports where it is.
  - That's a level from 0 to 3, which some finished buildings raise by one (`0x5D924F`), so probably the port building's level (a guess); [Blockade_Port](#the-candidates) needs it too.
  - Failing those, it goes to the last settlement's port, and with no port there the game would probably crash (not tested).
- Any other unit joins the capital's garrison when it has room (fewer than 20 units), making a garrison army when there's none.
- Otherwise it's a new army on a free tile beside the capital (`Settlement::unk_005BA4CA`; NYD what happens when there's no free tile, since the code ignores that).
- When a unit can't be made, the rest aren't either.

## Not verified

- That `campaign+0x24c` is the turn number, and what `+0x2b4` is.
- What stance 400 is (probably hostile, between neutral at 200 and war at 600).
- In-game checks:
  1. Change `attitude_boundary_1` and the coefficients in `descr_senate.txt` to extreme values; nothing about the senate should change.
  2. As the Julii, build armies until you hold well over half of Rome's military strength, and watch your senate standing fall by about 24 points.
  3. Fail a senate mission as the human, and check whether the Brutii's and Scipii's senate standings drop as well.
  4. Set every Demand_Suicide row in `descr_senate.txt` to `p1`, let your senate standing fall below 15, and refuse the demand; you should still be made an outlaw.

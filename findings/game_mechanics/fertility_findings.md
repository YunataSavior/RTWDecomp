# How family members have children, marry and die of old age

> Every claim here comes from the decompiled source in [`src/`](../../src/); none is NYD (Not Yet Decompiled).

**Short answer:** each turn, every married family member on the map rolls for a child. The base chance comes from **how big the family is for the faction's land**: 32% for a small family, falling to a **floor of 1%** once the counted family is about 1.17 × (0.75 × regions + 1) or more. **Fertility multiplies that base** by `1 + Fertility / 3`, so 3 points double it, and on the 1% floor that's still only about 2%. On top of the roll, the father must have a **wife of 50 or younger**, **fewer than 4 children** (adopted sons count), and **no child born in the last 2 years**. An unmarried general never has children; he gets a wife by chance, 1 in 21 a turn (1 in 11 for the leader or the heir).

Sources:

- The decompiled per-turn character update, `Character::unk_0057DD3B` (`0x57DD3B`), in [`src/code/character.cpp`](../../src/code/character.cpp).
  - It works the same as the original; it differs only in one unused stack slot (99.8%).
  - `Faction::unk_004CAC88` ([`src/code/faction.cpp`](../../src/code/faction.cpp)) runs it for each of the faction's characters at the end of its turn.
- The decompiled leader and heir code, `Faction::set_heir`, `choose_leader` and `choose_heir`, in [`src/code/faction.cpp`](../../src/code/faction.cpp).
  - They work the same as the original; the last two differ only in two swapped stack slots (99.96%).
- The decompiled character record, [`src/code/character_record.cpp`](../../src/code/character_record.cpp), for ageing (`update_age`), adoption and the record's fields.
- The two decompiled family counts the chance uses, `FactionUnk88C::family_per_region` (`0x4D2CCA`) and `FactionUnk88C::child_slots` (`0x4D3120`), in [`src/code/faction.cpp`](../../src/code/faction.cpp).
  - Both match the original byte for byte.
- The decompiled marriage and adoption offers, `FactionUnk88C::make_offers` (`0x4D2ACF`) with `MarriageOffer` and `AdoptionOffer`, in [`src/code/faction.cpp`](../../src/code/faction.cpp).
  - They match the original byte for byte.
- Addresses are virtual addresses in `RomeTW.exe` (Steam Rome: Total War Gold, image base `0x400000`).

## Who rolls

The roll is made once a turn for each character that:

1. Is a named character (character type 5) on the map.
   - Women aren't on the map, so the roll is the husband's.
2. Is in the family tree (the record's `m_flag16`).
3. Belongs to a faction whose description's `+0xa7` flag is clear.
   - What that flag marks isn't known; the AI senate code skips such factions too.

## The chance

The chance is worked out as a float percentage, in this order.

1. **The family's size for its land** (`family_per_region`): `ratio = family / (0.75 × regions + 1)`.
   - "Regions" is the number of regions the faction holds.
   - "Family" counts the faction's living family-tree members who are any of these:
     - men who have come of age and are on the map as named characters;
     - boys who haven't come of age yet but are 14 or older;
     - unmarried women aged 12 or older.
2. **The base chance** is `8 × (4 − round(3 × ratio))`, kept between 1 and 100.
   - The rounding is to the nearest whole number, with halves going to the even one (the FPU's default).
3. **Fertility** multiplies it by `Fertility / 3 + 1`.
   - Fertility is attribute 31, the sum of the character's `Effect Fertility` lines from traits and ancillaries.
   - A negative total lowers the chance, but the clamp in step 5 keeps it at 1% or more.
4. **It's halved when the family has 2 child slots or fewer** (`child_slots`).
   - Slots are counted over the family's living men on the map as named characters, aged 16 or more with fewer than 4 children, at 4 minus each man's children.
5. **It's kept between 1 and 100 again.**
6. **The roll** is `percent()`, a whole number from 0 to 100, and it succeeds when it's below the chance.
   - So the real chance is the chance rounded up, out of 101: a 1% chance is 1 in 101, and 32% is 32 in 101.

### What the base chance comes to

The table gives the smallest counted family that reaches each base chance, by regions held.

| Regions | 32% | 24% | 16% | 8% | 1% (the floor) |
|---|---|---|---|---|---|
| 1 | 0 | — | 1 | 2 | 3 |
| 3 | 0 | 1 | 2 | 3 | 4 |
| 5 | 0 | 1 | 3 | 4 | 6 |
| 10 | 0 | 2 | 5 | 8 | 10 |
| 15 | 0 | 3 | 7 | 11 | 15 |
| 20 | 0 | 3 | 8 | 14 | 19 |
| 30 | 0 | 4 | 12 | 20 | 28 |

So a faction whose counted family is about as large as its number of regions is on the floor, where every married general has about a 1% chance a turn, or about 2% with 3 points of Fertility.

## What has to be true after the roll

The roll comes first; a success still makes no child unless all of these hold:

1. He has a wife, and she is 50 or younger.
2. He has fewer than 4 children.
   - Adopted sons take these slots too (`CharacterRecord::adopt` adds them to the father's children).
3. Every child he has was born at least 2 years earlier, by calendar year (`campaign year − child's birth year ≥ 2`).

## The child

1. **It's a son or a daughter, 50-50** (`up_to(1) == 0` gives a son).
2. **It's born at age 0**, joins the family tree, and takes the father's surname when the faction is the one whose names he has.
   - The age is a scrambled constant in the code (`0x756B1143`), which unscrambles to 0.
3. **It goes in the father's next child slot.**
4. **The birth is then announced** (`fn_008B2BAF`, in [`src/code/faction_event.cpp`](../../src/code/faction_event.cpp)).
   - It shows the birth's message (`fn_00CB1397`), fires `Birth` for the child and `CharacterBecomesAFather` for the father, and adds the birth to the faction's records.

## Marriage

After the child roll, a family member on the map who is a man and has no wife may marry.

1. **The chance is 1 in 21 a turn, or 1 in 11 for the faction leader or the heir** (`up_to(20)` or `up_to(10)` coming out 0).
   - The heir is `Faction::m_heir`, whose record has the `m_heir` bit; `Faction::set_heir` fires `CeasedFactionHeir` and `BecomesFactionHeir`.
2. **The wife is a new character of his age plus −30 to +10, kept between 12 and 50.**
   - She's made of age (`m_flag2` cleared), and joins the family tree.
3. **His age plays no part in the chance**, so a 70-year-old still marries, to a wife of 40 to 50.
   - She can have children only until she's past 50.

## Old age

At the start of the same update, a character over 60 dies of old age when `range(60, 127)` comes out over 122. That's 5 chances in 68, about 7.4% a turn.

- Their death cause is set to 1.
- When the character was the faction leader, he stands down (`Character::stand_down`) and the faction chooses a new leader (`Faction::choose_leader`).
  - The heir becomes leader, and a new heir is chosen, without an announcement.
  - With no heir, the leader is the family member on the map with the most Influence whose character isn't dead (on a tie the first of them, but among men with 0 Influence the last), and then a new heir is chosen.
  - The new leader is then announced (`fn_008B3EE1`, in [`src/code/faction_event.cpp`](../../src/code/faction_event.cpp)), with the new-leader message (`fn_00CB0E15`).
- When the character was the heir, the faction has no heir until a new one is chosen and announced (`Faction::choose_heir`, and `fn_008B41DA` for the message).
  - The new heir is the family member on the map, other than the leader, with the most Influence whose character isn't dead (on a tie the first of them, but among men with 0 Influence the last).
- A record that isn't on the map ages in `CharacterRecord::update_age` instead, where the chance is over 120 (7 in 68) and every record dies at 127.

## The famine message

Last, `CharacterDamagedByDisaster` fires for famine (disaster 7) when the region the character is in has a famine threat of 2 or more.

## Why a Fertile general may have no children

1. **He has no wife.** He then has only the 1-in-21 marriage chance each turn.
2. **His wife is over 50.**
3. **He already has 4 children, adopted sons included.**
4. **He had a child within the last 2 years.**
5. **The family is large for the faction's land**, which puts the base at 1%, so 1 to 3 points of Fertility only bring it from 1 in 101 to 2 in 101.
   - Daughters of 12 or more count against the family until they marry, so married-off daughters raise the chance.
   - Conquering regions raises it too.
6. **The trait's Fertility may not be what it seems.**
   - Removing traits through triggers can leave old effects in the attributes; see [trait_effects_findings.md](trait_effects_findings.md).

## Marriage and adoption offers

A human faction may also be offered a suitor for a daughter or a man to adopt, once a turn after its characters' updates (`FactionUnk88C::make_offers`).

1. **A suitor is offered 1 turn in 4** (`up_to(3) == 0`), when no marriage offer is waiting.
   - The bride is the first of the faction's living unmarried women of 12 to 50 who is in one of its settlements, when that settlement isn't besieged and has room for a unit or a free tile beside it.
   - The suitor is a new family-tree man of her age −10 to +30, kept between 16 and 75, and `OfferedForMarriage` fires when he's made.
   - Accepted, he marries her when he's still alive; declined, he's removed.
2. **A man is offered for adoption when the family is small for its land**, and only when neither offer is waiting, so never on the turn a suitor is offered.
   - It's always offered when `family_per_region` is under 0.6, offered on a `percent()` under 20 (20 in 101) when it's under 0.8, and never otherwise.
   - The father is the first living family-tree man of 16 or more on the map as a named character, with fewer than 4 children, in a settlement that isn't besieged and has room.
   - The adoptee is a new family-tree man of 20 to 30, and `OfferedForAdoption` fires when he's made.
   - Accepted, he's adopted (taking one of the father's 4 child slots); declined, he's removed.
3. **Both offers help the birth chance's family count.**
   - Marrying a daughter off takes her out of the count, and an adoption is only offered when the count is low, which is when the birth chance is high too.

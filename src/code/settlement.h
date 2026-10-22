// Settlements (code\settlement.cpp).
#ifndef SETTLEMENT_H
#define SETTLEMENT_H

#include "unsorted.h"
#include "core_strings.h"
#include "env/tracked_object.h"
#include "settlement_base.h"
#include "settlement_mechanics.h"
#include "capabilities.h"
#include "building.h"

class SettlementUnk184;

// What a settlement hands to its +0xe0 and +0x120 when it changes owner.
struct SettlementOwnerChange {
    Settlement* m_settlement;
    Faction* m_faction;                 // the new owner
};

// What a settlement's +0xe0 is (not identified).
class SettlementUnkE0 {
public:
    void unk_005BF3A1(SettlementOwnerChange* change);  // 0x005BF3A1

    char m_pad0[0x40];
};

// What a settlement's +0x120 is (not identified).
class SettlementUnk120 {
public:
    void unk_005BF675(SettlementOwnerChange* change);  // 0x005BF675

    char m_pad0[0x64];
};

// What a settlement's +0x1e0 is (not identified).
class SettlementUnk1E0 {
public:
    void unk_005C2497();                // 0x005C2497
    void unk_005C26DB();                // 0x005C26DB: after a load

    char m_pad0[0x138];
};

// What a settlement's +0x318 is (not identified).
class SettlementUnk318 {
public:
    void unk_005C5559();                // 0x005C5559
    void unk_005C56E9();                // 0x005C56E9: after a load

    char m_pad0[0x1ec];
};

// What a riot or a revolt does to a settlement: ranges to roll each loss from, and more (not identified; 0x38 bytes).
struct SettlementDamageParams {
    SettlementDamageParams();           // 0x005BE897: all 0
    unsigned int building_chance() { return m_building_chance; }
    unsigned int heavy_chance() { return m_heavy_chance; }

    int m_0;                            // +0x00: a range rolled for SettlementDD8::unk_005C8CE1 (not identified)
    int m_4;
    int m_population_min;               // +0x08: the population killed, in percent
    int m_population_max;
    int m_character_min;                // +0x10: the chance that each character in it is killed, in percent
    int m_character_max;
    int m_garrison_min;                 // +0x18: the garrison's losses (passed to Army::kill_soldiers)
    int m_garrison_max;
    unsigned int m_building_chance;     // +0x20: the chance that each building is damaged, in percent
    int m_building_min;                 // +0x24: the health it loses
    int m_building_max;
    unsigned int m_heavy_chance;        // +0x2c: below this roll (out of 100), it loses the heavy amount instead
    int m_heavy_min;                    // +0x30
    int m_heavy_max;
};

// Up to 50 characters or buildings a riot or a revolt killed or damaged.
struct SettlementDamageList {
    SettlementDamageList(const SettlementDamageList& other)
    {
        int unused = 0;
        for (unsigned int i = 0; i < 50; i++)
            m_values[i] = other.m_values[i];
    }

    int m_values[50];
};

// What a riot or a revolt did to a settlement, for the messages (not identified; 0x1a8 bytes). Its copy constructor
// (0x005BE7CA) is the compiler's.
struct SettlementDamage {
    SettlementDamage(Settlement* settlement, int, unsigned int population, int garrison);  // 0x005BCC29
    void add_character(Character* character);  // 0x005BCCD3: a character it killed
    void add_building(Building* building);      // 0x005BCD0B: a building it damaged

    Settlement* m_settlement;
    int m_4;
    unsigned int m_population;          // +0x08: the population it killed
    int m_garrison;                     // +0x0c: the garrison's losses
    unsigned int m_num_characters;      // +0x10: how many characters it killed
    SettlementDamageList m_characters;  // +0x14
    unsigned int m_num_buildings;       // +0xdc: how many buildings it damaged
    SettlementDamageList m_buildings;   // +0xe0
};

// A settlement's buildings (not identified).
class SettlementBuildings {
public:
    Building* unk_005C2B69(unsigned int index);  // 0x005C2B69: the same as unk_005C2B81
    Building* unk_005C2B81(int index);  // 0x005C2B81
    void damage(SettlementDamageParams* params, SettlementDamage* damage, bool skip_outside);  // 0x005C2D29
    unsigned int count() { return m_count; }

    char m_pad0[0x200];
    unsigned int m_count;               // +0x200
    char m_pad204[0x564 - 0x204];
};

// What Settlement::unk_005B5975 returns (not identified; 8 bytes).
struct SettlementUnk5B5975 {
    SettlementUnk5B5975(const SettlementUnk5B5975& other);

    int m_0;
    int m_4;
};

// What each level of settlement has (16 bytes; not identified).
struct SettlementLevel {
    int m_0;                            // public order: squalor grows faster above twice this population
    int m_4;
    int m_8;
    int m_c;                            // public order: and again above this population
};

// What a new unit's soldiers start with in a settlement (not identified; 0x30 bytes).
struct UnitUnk4D469D {
    UnitUnk4D469D();                    // 0x004D469D

    char m_pad0[0x1c];
    unsigned int m_1c;                  // +0x1c
    char m_pad20[4];
    int m_24;                           // +0x24
    char m_pad28[0x34 - 0x28];
};

extern "C" {
extern SettlementLevel lbl_012DAFC8[];
}

// A settlement (probably code\settlement.cpp).

class Settlement : public SettlementBase {
public:
    bool is_capital() const { return m_is_capital; }
    virtual FogOfWarEntry* unk_74();                        // +0x74: 0x005B1440
    virtual int region_id();                                // +0x8c: 0x005BE77E
    virtual Faction* unk_9c();                              // +0x9c: the same function as owner()
    virtual Faction* owner();                               // +0xa0: 0x005BE753
    virtual void change_owner(CharacterRecord* captor, Faction* faction, int reason);  // +0xa8: 0x005B7F5E
    virtual void unk_ac(Character* leader);                 // +0xac: 0x005B9745, after a revolt

    void unk_0049B680(Faction* faction, int reason);        // 0x0049B680
    int unk_005B1486();                                     // 0x005B1486: fn_008F6174 of the owner
    bool better_governor(Character* character);             // 0x005B67A7
    void set_governor(Character* character);                // 0x005B9959
    void set_rebels(RebelType* type, int faction);          // 0x0049C42E: m_rebel_type and m_rebel_faction

    Character* character(unsigned int index, bool governor);  // 0x0049C13F
    void start_turn();                                      // 0x005B6476
    void unk_005BD3CE(int faction);                         // 0x005BD3CE: at the start of each faction's turn
    void end_turn();                                        // 0x005B656F
    bool unk_005BC86B();                                    // 0x005BC86B: m_entertainment % 3 > 0
    void unk_005BC88A(int);                                 // 0x005BC88A
    void unk_005B68EA();                                    // 0x005B68EA
    bool unk_0049A46D();                                    // 0x0049A46D: whether it has sieges (also called on forts)
    unsigned int num_buildings() const { return m_buildings.m_count; }
    SettlementDamage riot_damage(SettlementDamageParams* params, bool, bool, bool, bool skip_outside);  // 0x005B26B0
    void check_riots(BuildingEffects* effects);             // 0x005B2AC0
    void largest_share(int* faction, unsigned char* share, bool not_owner);  // 0x005B3312
    int revolt_type();                                      // 0x005B33D0
    Army* revolt(int type, int, int reason, int faction);   // 0x005B38D1: the rebels' army, or 0
    float revolt_strength();                                // 0x005B4C85
    void find_defectors(Array<Character*>* defectors, Faction* faction);  // 0x005B4ED3: who goes over in a civil war
    int unit_cost(BuildingEffects* effects);                // 0x005B9BFC: in percent
    void recruitable_units(RecruitmentItems* units, BuildingEffects* effects, bool ships, bool owned_only);  // 0x005B9E7C
    void unk_005C58A1(int faction, BuildingEffects* effects);  // 0x005C58A1
    void unk_0049B043(Faction* faction, int reason, bool);  // 0x0049B043: ends with change_owner
    void unk_0049B15C(Character* leader, int, int reason, bool);  // 0x0049B15C: ends with change_owner
    void unk_0049A894(bool);                                // 0x0049A894
    void unk_0049BC60(Character* character);                // 0x0049BC60
    Character* governor() const { return m_governor.m_character; }
    int level() const { return m_level; }
    int tax_level() const { return m_tax_level; }
    int plague_deaths() const { return m_plague_deaths; }
    void check_spies();                                     // 0x005B69CF: may throw out each foreign spy inside
    void unk_005B6BDB(BuildingEffects* effects);            // 0x005B6BDB
    void unk_005B6C52(BuildingEffects* effects);            // 0x005B6C52
    void unk_005B6F77(Faction* faction);                    // 0x005B6F77
    bool unk_005B7AEA();                                    // 0x005B7AEA
    void unk_005BAE31(TilePosition* position);              // 0x005BAE31
    void unk_005BD74B();                                    // 0x005BD74B
    void unk_005BD77B(int, bool);                           // 0x005BD77B
    int unk_005BD9DA();                                     // 0x005BD9DA
    bool can_stage_games();                                 // 0x005BC748: has the stage_games effect
    bool can_stage_races();                                 // 0x005BC79D: has the stage_races effect
    void set_entertainment(int entertainment);              // 0x005BC824
    int entertainment();                                    // 0x005BC859
    void unk_005BCA50(BuildingEffects* effects);            // 0x005BCA50: works out its building effects
    bool unk_005BA3F0(TilePosition* position);              // 0x005BA3F0
    bool unk_005BA4CA(TilePosition* position, int, int, int);  // 0x005BA4CA: a free tile beside it (a guess)
    int unk_005BB0ED(Unit* unit, UnitUnk4D469D& experience);  // 0x005BB0ED
    void unk_005B9837(Unit* unit);      // 0x005B9837: updates its mechanics (SettlementDD8::update) after a unit leaves
    void unk_005BBDAA(Unit* unit, int, int);  // 0x005BBDAA: takes the unit out of its queue (+0x1e0)
    void unk_005B5D57();                                    // 0x005B5D57
    void unk_005B5F04();                                    // 0x005B5F04: at the end of a round
    int owner_id();                                         // 0x005B14AB
    Region* region();                                       // 0x005B7D41
    void unk_005B5849(int* religion, int* religion_effects);  // 0x005B5849: BI's religion
    int unk_005B5AEB();                                     // 0x005B5AEB
    int unk_005B5B56(SettlementReligion* religion);         // 0x005B5B56
    SettlementUnk5B5975 unk_005B5975();                     // 0x005B5975
    int unk_005BC80B();                                     // 0x005BC80B: public order slot 8, times 5
    int unk_005BCDC4();                                     // 0x005BCDC4: public order slot 11 (distance to the capital?)
    void after_load();                                      // 0x005B2241
    void unk_005BACCE(TilePosition* position);              // 0x005BACCE: puts it on the map there (a guess)

    RebelType* m_rebel_type;            // +0xb4: who rebels in it (a guess, from what sets it)
    int m_rebel_faction;                // +0xb8: the faction they're for (21 for none)
    char m_padbc[0xe0 - 0xbc];
    SettlementUnkE0 m_e0;               // +0xe0
    SettlementUnk120 m_120;             // +0x120
    SettlementUnk184* m_184;            // +0x184
    char m_pad188[0x190 - 0x188];
    UniString m_name;                   // +0x190: its name as shown (a guess)
    Faction* m_owner;                   // +0x194
    char m_pad198[0x19c - 0x198];
    unsigned char m_population_share[0x1b4 - 0x19c];  // +0x19c: each faction's share of the population, by faction
    int m_level;                        // +0x1b4: 0 for the smallest
    char m_pad1b8[4];
    int m_creator;                      // +0x1bc: the faction that built it (a guess)
    unsigned short m_owner_turns;       // +0x1c0: turns since its owner changed
    char m_pad1c2[2];
    int m_region_id;                    // +0x1c4
    char m_pad1c8[0x1e0 - 0x1c8];
    SettlementUnk1E0 m_1e0;             // +0x1e0
    SettlementUnk318 m_318;             // +0x318
    SettlementBuildings m_buildings;    // +0x504
    TilePosition m_a68;                 // +0xa68: a position (where it was last placed? a guess)
    char m_pada70[0xa78 - 0xa70];
    int m_plague_deaths;                // +0xa78: the population the plague killed in the last growth step

    bool m_force_revolt;                // +0xa7c: the next riot check revolts
    bool m_rioted;                      // +0xa7d: it rioted at the last check
    unsigned char m_a7e;                // +0xa7e: counts the revolts while the owner's unk_004584F1() holds
    char m_pada7f[1];
    unsigned int m_turns_since_revolt;  // +0xa80
    int m_entertainment;                // +0xa84: 0-2 hold games, 3-5 races, 6-8 both (at three levels? a guess)
    bool m_is_capital;                  // +0xa88: its owner's capital (Faction::set_capital)
    char m_pada89;
    bool m_a8a;                         // +0xa8a: cleared by a change of owner
    bool m_a8b;                         // +0xa8b: cleared by a change of owner
    char m_pada8c[0xa94 - 0xa8c];
    WatchedCharacter m_governor;        // +0xa94
    WatchedCharacter m_a9c;             // +0xa9c
    char m_padaa4[0xabc - 0xaa4];
    Array<void*> m_abc;                 // +0xabc: type 8 of the saved pointers (not identified)
    int m_tax_level;                    // +0xac8: 0 low to 3 very high
    char m_padacc[0xdd5 - 0xacc];
    bool m_dd5;                         // +0xdd5
    char m_padd6[0xdd8 - 0xdd6];
    SettlementDD8 m_dd8;                // +0xdd8
};

#endif

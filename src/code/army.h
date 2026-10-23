// Armies (code\army.cpp).
#ifndef ARMY_H
#define ARMY_H

#include "unsorted.h"
#include "spying_info.h"

// An army's first 0xc bytes (not identified).
class Army0 {
public:
    virtual void unk_00();
    char m_pad4[8];
};

// An army (probably code\army.cpp).
class Army : public Army0, public ArmySpyingInfo {
public:
    Army(Character* character, int);    // 0x00719056
    int num_units() const { return m_units.size(); }
    Unit* unit_at(int index) { return m_units[index]; }
    int unk_134() const { return m_134; }
    bool unk_ac() const { return m_ac; }
    Army* carried() const
    {
        Army* army = m_carried;         // the original's expansion copies it through a local
        return army;
    }
    Region* unk_007199A3();             // 0x007199A3: the region it's in (a guess)
    int unk_007216D2(int, int, TilePosition* position);  // 0x007216D2
    void raise_spying_level(int faction, int level);  // 0x0071EBF6: on it and everyone in it
    int unk_bc() const { return m_bc; }

    TilePosition* unk_00719BB2();       // 0x00719BB2
    void unk_00719E61();                // 0x00719E61
    Unit* unk_0071DADD(Character* character, int, int, int, int);  // 0x0071DADD
    void unk_007201B3(RebelType* type, int faction);  // 0x007201B3: m_rebel_type and m_rebel_faction
    Unit* unit(int index);              // 0x0072212A
    int garrison_soldiers();            // 0x00722341
    Character* leader() const { return m_leader; }

    void remove_unit(Unit* unit);       // 0x0071E460: and deletes it; a command unit lost in the field gives the enemy its eagle
    StrategyObject* unk_11c() const { return m_11c; }
    void unk_0071E2C8(Unit* unit);      // 0x0071E2C8: takes it out of m_units
    void update_strength();             // 0x0071B41D
    unsigned int compute_strength();    // 0x0071B451: the sum of its units' values, rounded
    int unk_00722430();                 // 0x00722430: its soldiers (each unit's +0x280 and unk_007B9438())
    int kill_soldiers(int percent);     // 0x0071FE0D: each unit loses that share of its soldiers; returns the dead
    unsigned int strength();            // 0x0071B3EB
    void unk_00720D26(int type, float strength, int min_units, int max_units, RecruitmentItems* units);  // 0x00720D26
    void unk_007201D6(int type, float strength, int min_units, int max_units, RebelType* rebel);  // 0x007201D6
    int unk_007225D5(bool, bool);       // 0x007225D5: how many units are ... (not identified)
    bool unk_007233E0(bool);            // 0x007233E0: it has room for that many more units, up to 20
    void unk_0071D66C(Unit* unit, int); // 0x0071D66C: takes the unit in (a guess)
    void unk_00721E6B(TilePosition* position);  // 0x00721E6B: puts it on the map there (a guess)
    void unk_00724043(int, int, int, int);  // 0x00724043
    void unk_00725C23(Army* army);      // 0x00725C23
    Faction* unk_00721587();            // 0x00721587: its faction (+0x54)
    Character* unk_007216B9();          // 0x007216B9: its leader (+0xf0)
    void unk_0071B214(Character* character, int);  // 0x0071B214: takes it out of m_characters (a guess)
    void unk_0071E3F1(Unit* unit);      // 0x0071E3F1: takes it out (a guess)
    bool unk_0072309A();                // 0x0072309A

    char m_pad30[0x54 - 0x30];
    Faction* m_faction;                 // +0x54
    char m_pad58[0x5c - 0x58];
    Array<Unit*> m_units;               // +0x5c
    char m_pad68[0xac - 0x68];
    bool m_ac;                          // +0xac: it's a fleet (a guess)
    char m_padad[0xb4 - 0xad];
    Army* m_carried;                    // +0xb4: the army a fleet carries, or 0 (a guess)
    char m_padb8[0xbc - 0xb8];
    int m_bc;                           // +0xbc
    char m_padc0[0xf0 - 0xc0];
    Character* m_leader;                // +0xf0
    char m_padf4[0x10c - 0xf4];
    Array<Character*> m_characters;     // +0x10c: the characters with it besides its leader (a guess)
    char m_pad118[0x11c - 0x118];
    StrategyObject* m_11c;              // +0x11c: where it is (a settlement or fort it's in?)
    char m_pad120[0x129 - 0x120];
    bool m_129;                         // +0x129: set when a battle is set up with it (0x0071C970), cleared at 0x0071CFA4
    bool m_in_battle;                   // +0x12a: set by the battle code (a guess)
    char m_pad12b[0x134 - 0x12b];
    int m_134;                          // +0x134
    bool m_captured_eagle;              // +0x138: it took an enemy's eagle (a guess)
    bool m_139;                         // +0x139: while it's set, its strength is 0
    char m_pad13a[0x140 - 0x13a];
    unsigned int m_strength;            // +0x140
    char m_pad144[0x164 - 0x144];
    RebelType* m_rebel_type;            // +0x164: who a settlement it takes rebels for (a guess)
    int m_rebel_faction;                // +0x168
};

#endif

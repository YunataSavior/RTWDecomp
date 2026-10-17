// A character on the strategy map (code\character.cpp).
#ifndef CHARACTER_H
#define CHARACTER_H

#include "unsorted.h"

class CharacterRecord;

// A strategy object with 0xc more bytes (not identified): Character's first base.
class Character24 : public StrategyObject {
public:
    virtual void unk_74();
    virtual int unk_78();

    char m_pad24[0xc];
};

// What kind of character it is (not identified: Character's base at +0x30).
class CharacterKind {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual Faction* faction();                 // +0x08
    virtual int type();                         // +0x0c: 0 spy, 1 assassin, 2 diplomat, 3 admiral, 4 general,
                                                //   5 named character (a script command's names, 0x00B0EFF1)
    virtual Army* kind_army();                  // +0x10: the army (a guess)
};

// What a character's +0x154 is (not identified).
class CharacterUnk154 {
public:
    bool unk_0056BE44();                        // 0x0056BE44

    char m_pad0[4];
};

// What a character's +0x58 is (not identified).
class CharacterUnk58 {
public:
    Random* random();                           // 0x00B4DC87: the campaign's
};

// What a character's +0x7c points to (not identified).
struct CharacterUnk7C {
    Faction* m_faction;
};

// What a character's +0x80 points to (not identified).
struct CharacterUnk80 {
    int m_type;                                 // as CharacterKind::type()
};

class Character : public Character24, public CharacterKind {
public:
    bool unk_a0() const { return m_a0; }
    CharacterRecord* record() const { return m_record; }
    void start_turn() { reset_movement(); }
    int unk_b4() const { return m_b4; }
    float unk_c4() const { return m_c4; }
    bool has_plague() const { return m_plague; }
    Army* led_army() const { return m_army; }

    void catch_plague();                        // 0x0058AD99
    bool plague_tick(int* soldiers_lost);       // 0x0058ADEA

    Faction* unk_00580C1E();                    // 0x00580C1E: the first item of m_7c
    bool unk_0058D0C5();                        // 0x0058D0C5
    StrategyObject* unk_00586278(Array<int>* tiles);  // 0x00586278
    bool unk_00585CF4(Array<int>* tiles, int);  // 0x00585CF4
    Army* army();                               // 0x00580C40: the army it leads, or 0
    bool unk_0058B8D9(Faction* faction);  // 0x0058B8D9
    void unk_00580DC3(Army* army);              // 0x00580DC3
    bool unk_005814BA();                        // 0x005814BA
    float unk_005815D7();                       // 0x005815D7
    void unk_00581619(float);                   // 0x00581619
    void unk_0057DD3B();                        // 0x0057DD3B
    void unk_00587731();                        // 0x00587731
    void unk_00587D27();                        // 0x00587D27
    int unk_00586C03();                         // 0x00586C03
    Settlement* unk_0058AFA9();                 // 0x0058AFA9: the settlement it governs, or 0 (a guess)
    void reset_movement();                      // 0x0058CBBC: the turn's movement points (Pharos: admirals x1.1)
    void unk_004B7B27(int);                     // 0x004B7B27
    bool unk_00583B60();                        // 0x00583B60
    void set_death_cause(int cause);            // 0x00580576: the record's +0x1c0 (2 in a riot; a guess)
    void kill();                                // 0x005804E0: marks it dead (m_a0)
    void unk_005840D0(Settlement* settlement, int, int, int);  // 0x005840D0: through m_154 (a guess: out of the settlement)
    Unit* unk_00581119();                       // 0x00581119: its bodyguard (a guess)
    Army* unk_00581089(Unit* unit);             // 0x00581089: a new army for it
    void unk_005877ED(int);                     // 0x005877ED
    void unk_00588F38(Army* army);              // 0x00588F38
    void unk_00581E49(Faction* faction);        // 0x00581E49
    void unk_0057FAE2(Faction* faction, int);   // 0x0057FAE2: goes over to the faction
    void unk_00587657();                        // 0x00587657
    void unk_004B7AE0();                        // 0x004B7AE0

    char m_pad34[0x58 - 0x34];
    CharacterUnk58 m_58;                        // +0x58
    char m_pad59[0x78 - 0x59];
    CharacterRecord* m_record;                  // +0x78
    CharacterUnk7C* m_7c;                       // +0x7c
    CharacterUnk80* m_80;                       // +0x80
    char m_pad84[0xa0 - 0x84];
    bool m_a0;                                  // +0xa0
    char m_pada1[0xae - 0xa1];
    bool m_plague;                              // +0xae
    char m_padaf[1];
    int m_plague_ticks;                         // +0xb0
    int m_b4;                                   // +0xb4
    char m_padb8[0xc4 - 0xb8];
    float m_c4;                                 // +0xc4
    char m_padc8[0xd4 - 0xc8];
    Settlement* m_settlement;                   // +0xd4: the settlement it's in, or 0
    char m_padd8[0xe4 - 0xd8];
    Army* m_army;                               // +0xe4: the army it leads, or 0
    char m_pade8[0x154 - 0xe8];
    CharacterUnk154 m_154;                      // +0x154
};

#endif

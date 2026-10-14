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
    virtual int type();                         // +0x0c: 3 general?, 4 admiral?, 5 named character? (guesses)
    virtual Army* kind_army();                  // +0x10: the army (a guess)
};

class Character : public Character24, public CharacterKind {
public:
    bool unk_a0() const { return m_a0; }
    void start_turn() { reset_movement(); }
    int unk_b4() const { return m_b4; }
    float unk_c4() const { return m_c4; }

    Faction* unk_00580C1E();                    // 0x00580C1E: the first item of m_7c
    bool unk_0058D0C5();                        // 0x0058D0C5
    StrategyObject* unk_00586278(Array<int>* tiles);  // 0x00586278
    bool unk_00585CF4(Array<int>* tiles, int);  // 0x00585CF4
    Army* army();                               // 0x00580C40: the army it leads, or 0
    bool unk_0058B8D9(Settlement* settlement);  // 0x0058B8D9
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

    char m_pad34[0x78 - 0x34];
    CharacterRecord* m_record;                  // +0x78
    char m_pad7c[0xa0 - 0x7c];
    bool m_a0;                                  // +0xa0
    char m_pada1[0xb4 - 0xa1];
    int m_b4;                                   // +0xb4
    char m_padb8[0xc4 - 0xb8];
    float m_c4;                                 // +0xc4
};

#endif

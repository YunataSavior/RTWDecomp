// Battles (code\battle.cpp).
#ifndef BATTLE_H
#define BATTLE_H

#include "unsorted.h"

// One army of a battle's side (0x6c bytes).
struct BattleSideArmy {
    Army* m_army;
    char m_pad4[0x6c - 4];
};

// A side of a battle (not identified).
class BattleSide {
public:
    bool unk_0() const { return m_0; }
    Army* army(int index) { return m_armies[index].m_army; }
    Army* first_army() { return army(0); }
    unsigned int num_armies() const { return m_1b58; }
    unsigned int num_reinforcements() const { return m_1bfc; }

    bool m_0;                           // +0x00: attacking? (a guess)
    char m_pad1[0x5c - 1];
    BattleSideArmy m_armies[63];        // +0x5c
    char m_pad1af0[0x1b58 - 0x1af0];
    unsigned int m_1b58;                // +0x1b58
    char m_pad1b5c[0x1bfc - 0x1b5c];
    unsigned int m_1bfc;                // +0x1bfc
};

// The battle being fought (not identified).
class Battle {
public:
    int unk_8() const { return m_8; }
    bool unk_c() const { return m_c; }
    bool unk_24() const { return m_24; }
    bool unk_46() const { return m_46; }

    int side_of(Army* army);            // 0x005FF179: -1 for neither
    BattleSide* side(int index);        // 0x005FF3BB
    BattleSide* unk_005FF3D9(int index);  // 0x005FF3D9: the same as side()
    int side_of_faction(int faction);   // 0x005FF3F7
    void force_victory(int side);       // 0x005FD5D1: every other side loses (the force_battle_ commands)
    unsigned int num_sides() const { return m_num_sides; }

    char m_pad0[8];
    int m_8;                            // +0x08
    bool m_c;                           // +0x0c
    char m_padd[0x24 - 0xd];
    bool m_24;                          // +0x24
    char m_pad25[0x46 - 0x25];
    bool m_46;                          // +0x46
    char m_pad47[0xe1f8 - 0x47];
    int m_side_of_faction[21];          // +0xe1f8
    unsigned int m_num_sides;           // +0xe24c
};

extern "C" {
extern Battle* lbl_018BF5E0;        // the battle being fought
}

#endif

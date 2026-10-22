// Hordes (code\horde.cpp).
#ifndef HORDE_H
#define HORDE_H

#include "unsorted.h"

// Something of a faction's, updated at the start of its turn (not identified).
class FactionUnk4F8 {
public:
    int unk_4() const { return m_4; }

    void unk_004DBCBD();                // 0x004DBCBD

    char m_pad0[4];
    int m_4;                            // +0x04: a faction, 21 for none
};

// The strength of the armies in some regions, split by whose they are (not identified).
struct ArmyStrengths {
    void clear();                       // 0x004DD16A

    int m_own;                          // +0x00: the faction's own armies
    int m_own_count;
    int m_enemy;                        // +0x08: the armies of factions it's at war with
    int m_enemy_count;
    int m_other;                        // +0x10: the rest
    int m_other_count;
};

#endif

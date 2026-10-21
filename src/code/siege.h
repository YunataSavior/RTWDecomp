// Sieges (code\siege.cpp).
#ifndef SIEGE_H
#define SIEGE_H

#include "unsorted.h"

// An army besieging a settlement (a guess, from how a settlement's list of them is checked for enemies).
class Siege {
public:
    Character* unk_00717C38();          // 0x00717C38: m_army's leader

    char m_pad0[0x10];
    Army* m_army;                       // +0x10
};

// One of a settlement's sieges (8 bytes).
struct SettlementSiege {
    Siege* m_siege;
    int m_4;
};

#endif

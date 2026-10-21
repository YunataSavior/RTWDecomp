// Forts (code\fort.cpp).
#ifndef FORT_H
#define FORT_H

#include "unsorted.h"
#include "settlement_base.h"

// A fort (code/fort.cpp).
class Fort : public SettlementBase {
public:
    void unk_00498B82(int faction);                         // 0x00498B82: at the start of each faction's turn
};

// The campaign's forts (not identified).
class CampaignForts {
public:
    void unk_0049954D(Fort* fort);      // 0x0049954D: takes it off the map (a guess)

    Fort** m_items;
    int m_capacity;
    unsigned int m_count;
};

#endif

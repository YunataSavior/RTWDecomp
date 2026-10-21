// The factions' diplomatic relations (code\faction_diplomacy.cpp).
#ifndef FACTION_DIPLOMACY_H
#define FACTION_DIPLOMACY_H

#include "unsorted.h"

// How one faction stands towards another.
class DiplomaticRelation {
public:
    char m_pad0[4];
    int m_stance;                       // +0x04: 0 allied, 600 at war
    unsigned int m_flags;               // +0x08: bit 1 is probably military access
    char m_padc[0x74 - 0xc];
};

// One faction's relations with every faction.
class FactionRelations {
public:
    int stance(int other) const { return m_relations[other].m_stance; }

    DiplomaticRelation m_relations[21];
};

// Every pair of factions' relations (probably code\faction_diplomacy.cpp).
class DiplomacyMatrix {
public:
    int stance(const int& faction, const int& other) const { return m_factions[faction].stance(other); }
    bool is_at_war(int faction);        // 0x005A11E9: at war with anyone
    void unk_005A2EEE();                // 0x005A2EEE: at the end of a round
    void unk_005A2C6A();                // 0x005A2C6A
    void unk_005A1417(int faction, int other, int incident);  // 0x005A1417: an incident between two factions
    void declare_war(int faction, int other);   // 0x005A21DD: sets their stance to 600 (war), with what follows
    bool unk_005A0ABE(int faction, int other);  // 0x005A0ABE: probably whether they have a trade agreement
    bool unk_005A0F3E(int faction, int other);  // 0x005A0F3E: bit 3 of their relation's flags
    bool unk_005A0FB1(int faction);     // 0x005A0FB1: 0x005A0F3E's flag with any faction

    FactionRelations m_factions[21];
};

#endif

// The factions of descr_sm_factions.txt (code\faction_db.cpp).
#ifndef FACTION_DB_H
#define FACTION_DB_H

#include "unsorted.h"
#include "romans_utils.h"

// A faction's entry in descr_sm_factions (probably code\faction_db.cpp).
struct FactionDescription40 {
    int m_0;                            // a culture? (a guess)
};

class FactionDescription {
public:
    bool has_shadowed_by() const { return m_shadowed_by < 0xffffffff; }
    bool shadowed_by_set() const { return m_10 != 0; }
    int shadowed_by() const { return shadowed_by_set() ? m_shadowed_by : 21; }
    bool has_shadowing() const { return m_shadowing < 0xffffffff; }
    bool shadowing_set() const { return m_1c != 0; }
    int shadowing() const { return shadowing_set() ? m_shadowing : 21; }
    bool has_spawns_on_revolt() const { return m_spawns_on_revolt < 0xffffffff; }
    bool spawns_on_revolt_set() const { return m_28 != 0; }
    int spawns_on_revolt() const { return spawns_on_revolt_set() ? m_spawns_on_revolt : 21; }
    bool unk_a7() const { return m_a7; }

    int m_id;                           // +0x00: its faction's index
    char m_pad4[8];
    unsigned int m_shadowed_by;         // +0x0c: its shadowed_by faction (0xffffffff for none)
    int m_10;                           // +0x10: non-zero once m_shadowed_by is set (a guess)
    char m_pad14[4];
    unsigned int m_shadowing;           // +0x18: its shadowing faction (0xffffffff for none)
    int m_1c;                           // +0x1c
    char m_pad20[4];
    unsigned int m_spawns_on_revolt;    // +0x24: its spawns_on_revolt faction (0xffffffff for none)
    int m_28;                           // +0x28
    char m_pad2c[0x40 - 0x2c];
    FactionDescription40* m_40;         // +0x40
    char m_pad44[0xa4 - 0x44];
    bool m_custom_battle_availability;  // +0xa4
    char m_pada5[2];
    bool m_a7;                          // +0xa7
    char m_pada8[0xdc - 0xa8];
    int m_dc;                           // +0xdc: lets the AI keep looking for wars with up to 9 targets
};

extern "C" {
FactionDescription* fn_008F60D5(const String& name);  // the faction description of this name
int fn_008F6278(String name);       // the index of the faction of this name
}

#endif

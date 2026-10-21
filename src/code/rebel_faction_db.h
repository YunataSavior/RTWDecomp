// The rebel types of descr_rebel_factions.txt (code\rebel_faction_db.cpp).
#ifndef REBEL_FACTION_DB_H
#define REBEL_FACTION_DB_H

#include "unsorted.h"
#include "romans_utils.h"

// A rebel type in descr_rebel_factions.txt (code\rebel_faction_db.cpp; 0xb74 bytes).
struct RebelType {
    char m_pad0[4];
    String m_name;                      // +0x04
    char m_padc[0x14 - 0xc];
    int m_chance;                       // +0x14
};

extern "C" {
RebelType* fn_00903C7E(const String& name);    // the rebel type of this name, or 0
}

#endif

// The ancillaries' definitions, from export_descr_ancillaries.txt (code\character_ancillaries_db.cpp).
#ifndef CHARACTER_ANCILLARIES_DB_H
#define CHARACTER_ANCILLARIES_DB_H

#include "unsorted.h"

#include "character_traits_db.h"

// An ancillary (0x74 bytes).
class AncillaryDescription {
public:
    int m_index;                                    // +0x00: its position in the file
    char m_pad4[0x1c - 4];
    bool m_unique;                                  // +0x1c: only one character can have it (a guess)
    char m_pad1d[3];
    FixedArray<AncillaryDescription*, 3> m_excluded;    // +0x20: the ancillaries a character with it cannot have
    unsigned int m_num_excluded;                    // +0x2c
    BitSet<32> m_exclude_cultures;                  // +0x30
    Array<TraitEffect> m_effects;                   // +0x38
    TraitReligion m_religious_belief;               // +0x44
    TraitReligion m_religious_order;                // +0x4c
    char m_pad54[0x74 - 0x54];
};

class AncillaryDatabase {
public:
    static Array<AncillaryDescription> s_ancillaries;   // 0x014DAA40
};

#endif

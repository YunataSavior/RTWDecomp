// Buildings (code\building.cpp).
#ifndef BUILDING_H
#define BUILDING_H

#include "unsorted.h"
#include "romans_utils.h"

// A type of building in export_descr_buildings.txt.
struct BuildingType {
    bool defenses() { return m_defenses; }
    bool port() { return m_port; }
    bool hinterland() { return m_hinterland; }

    char m_pad0[0x5c];
    bool m_defenses;                    // +0x5c: the defenses strand (walls)
    bool m_port;                        // +0x5d: the port_buildings strand
    bool m_5e;
    bool m_temple;                      // +0x5f: its name starts with temple_
    bool m_hinterland;                  // +0x60: its name starts with hinterland_ (roads, farms)
    char m_pad61[0x68 - 0x61];
    String m_name;                      // +0x68
};

// A building of a settlement.
struct Building {
    void damage(unsigned int amount);   // 0x005D755F: takes amount off its health; at 0 it's destroyed
    BuildingType* type() { return m_type; }
    unsigned int health() { return m_health; }

    char m_pad0[0x2c];
    BuildingType* m_type;               // +0x2c
    unsigned char m_level;              // +0x30: 0 for its type's first level
    char m_pad31[0x4c - 0x31];
    unsigned int m_health;              // +0x4c: 0 to 100
};

#endif

// The unit types of export_descr_unit.txt (code\unit_db.cpp).
#ifndef UNIT_DB_H
#define UNIT_DB_H

#include "unsorted.h"

class String;

struct UnitUnk50 {
    char m_pad0[0x20];
    const UniString* m_name;            // +0x20: its name's text, as shown
    char m_pad24[0x2c - 0x24];
    int m_2c;                           // +0x2c: its category (0 infantry, 1 cavalry, 4 ships, 5 animals; a guess)
    char m_pad30[0x80 - 0x30];
    unsigned short m_turns;             // +0x80: stat_cost's turns to train it
    unsigned short m_cost;              // +0x82: stat_cost's cost
    unsigned short m_upkeep;            // +0x84: stat_cost's upkeep
    unsigned short m_weapon_cost;       // +0x86: stat_cost's cost of a weapon upgrade
    unsigned short m_armour_cost;       // +0x88: stat_cost's cost of an armour upgrade
    unsigned short m_custom_cost;       // +0x8a: stat_cost's cost in a custom battle
    char m_pad8c[0xc0 - 0x8c];
    unsigned short m_c0;                // +0xc0: its soldiers before the unit size scales them (a guess)
    char m_padc2[0xd0 - 0xc2];
    unsigned int m_d0;                  // +0xd0: bit 0 when its weapon can be upgraded (a guess)
    char m_padd4[0xe0 - 0xd4];
    int m_weapon;                       // +0xe0: its weapon's kind, 0 simple to 4 other (a guess)
    char m_pade4[0x154 - 0xe4];
    int m_154;                          // +0x154: added to its soldiers (not identified)
    char m_pad158[0x1f4 - 0x158];
    unsigned int m_ownership;           // +0x1f4: the factions that own it, a bit each
    char m_pad1f8[0x200 - 0x1f8];
    unsigned int m_attributes_0 : 7;    // +0x200: its attributes from export_descr_unit.txt, one bit each
    unsigned int m_attribute_7 : 1;     // bit 7 (not identified)
    unsigned int m_attributes_8 : 6;
    unsigned int m_command : 1;         // bit 14: command (an eagle or standard)
    unsigned int m_attributes_15 : 8;
    unsigned int m_is_peasant : 1;      // bit 23: is_peasant
    unsigned int m_general_unit : 1;    // bit 24: general_unit (general_unit_upgrade sets it too)
    unsigned int m_general_unit_upgrade : 1;  // bit 25: general_unit_upgrade
    unsigned int m_attributes_26 : 6;
    char m_pad204[0x20c - 0x204];

    int soldiers();                     // 0x00918878: m_c0 scaled by the unit size (a guess)

    bool owned_by(int faction) const { return (m_ownership & (1 << faction)) != 0; }
    bool is_animal() const { return m_2c == 5; }
};

// The unit types of export_descr_unit.txt (code\unit_db.cpp).
class UnitDB {
public:
    int count() const { return m_count; }
    bool has_general_upgrade(int faction);  // 0x0091B995
    int find(const String& name);       // 0x0091B93A: the index of the type of that name, or -1
    Unit* unk_0091AEAA(Region* region, int type, int faction, int, int);  // 0x0091AEAA: a new unit of the type (a guess)

    char m_pad0[4];
    UnitUnk50 m_types[500];             // +0x04
    int m_count;                        // +0x3ff74
};

extern "C" {
extern unsigned int lbl_015C95B4;    // how many unit types there are
extern UnitUnk50 lbl_01589644[];     // the unit types
extern UnitDB lbl_01589640;          // the same, with their count
}

#endif

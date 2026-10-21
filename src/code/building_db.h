// The building types of export_descr_buildings.txt (code\building_db.cpp).
#ifndef BUILDING_DB_H
#define BUILDING_DB_H

#include "unsorted.h"
#include "romans_utils.h"
#include "building.h"

// What lbl_014DA2B0's +0x2ac is (not identified).
class BuildingTypeList {
public:
    unsigned int unk_005BEA85();        // 0x005BEA85: how many
};

// The building types of export_descr_buildings.txt (a guess, from what it holds).
class BuildingDB {
public:
    BuildingType* unk_008D8695(unsigned int index);  // 0x008D8695
    int unk_008DB1B9(const String& name);   // 0x008DB1B9: the index of this hidden resource, or -1 (a guess)

    char m_pad0[0x2ac];
    BuildingTypeList m_types;           // +0x2ac
};

extern "C" {
extern BuildingDB lbl_014DA2B0;
}

#endif

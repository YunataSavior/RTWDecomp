// The factions' fog of war (code\fog_of_war.cpp).
#ifndef FOG_OF_WAR_H
#define FOG_OF_WAR_H

#include "unsorted.h"
#include "strategy_map.h"

// What FactionUnk180::unk_00493794 finds on a tile (not identified).
class FactionTileUnk {
public:
    virtual void unk_00();
    virtual void unk_04(Faction* faction);
    virtual int unk_08();
    virtual int unk_0c();
};

// What a faction's +0x180 is (not identified).
class FactionUnk180 {
public:
    unsigned char at(const RoadPoint& point) { return m_data[point.x + point.y * m_width]; }
    unsigned char at(const TilePosition& point) { return m_data[point.x + point.y * m_width]; }
    void unk_00492498(Character* character, bool);  // 0x00492498
    void unk_00492B0F(StrategyObject* object, bool);  // 0x00492B0F
    FactionTileUnk* unk_00493794(const RoadPoint& point);  // 0x00493794

    unsigned char* m_data;              // a byte per tile
    int m_width;                        // +0x04
};

#endif

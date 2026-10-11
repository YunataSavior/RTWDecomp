#ifndef ROAD_H
#define ROAD_H

#include "unsorted.h"

// A road between two regions' settlements (or from a settlement to its port). Its route is planned once, by the
// region that builds first: the planner's part is drawn, and the part in the other region waits, pending, until that
// region builds roads too.
class Road {
public:
    Road();                                                     // 0x004A1B51
    Road(int planner, int other);                               // 0x004A1B9C
    void set_path(Array<RoadPoint>& path, int border);          // 0x004A1BED
    void reveal();                                              // 0x004A1C6A
    void clear_drawn();                                         // 0x004A1CCF
    void set_road_level(int x, int y, int level);               // 0x004A1CF0
    void add_traffic(int amount);                               // 0x004A1D3B
    void reset_traffic();                                       // 0x004A1DC7
    bool unk_004A1DFD();                                        // 0x004A1DFD
    bool load(SaveStream& stream);                              // 0x004A1F63
    bool save(SaveStream& stream);                              // 0x004A20E2
    void unk_004A2229();                                        // 0x004A2229

    int planner() const { return m_planner; }
    int other() const { return m_other; }

    Array<RoadPoint> m_drawn;    // +0x00: the tiles shown on the map
    Array<RoadPoint> m_pending;  // +0x0c: the tiles in the other region, shown once it builds roads
    int m_planner;               // +0x18: the region that planned the road
    int m_other;                 // +0x1c: the region at its other end (the planner again for a port road)
    unsigned int m_traffic;      // +0x20
    bool m_split;                // +0x24: the route crosses into the other region, which has no roads yet
    int m_id;                    // +0x28: a number no other road has (the lowest free one when it was made)
    int m_display;               // +0x2c: the aerial map's handle for it, or -1
};

// The campaign's roads (Campaign +0xcd4c).
class RoadManager {
public:
    RoadManager();                                              // 0x004A2232
    void create(int planner, int other, int border);            // 0x004A224F
    void create_port_road(int region);                          // 0x004A2328
    void upgrade(Road* road);                                   // 0x004A2404
    void show();                                                // 0x004A249A
    void hide();                                                // 0x004A25F2
    void register_tiles(Road* road);                            // 0x004A2630
    int free_id();                                              // 0x004A273B
    void clear();                                               // 0x004A27B2
    void update_connected_regions(Region* region);              // 0x004A2801
    void reset_connected_traffic(Region* region);               // 0x004A295C
    void unk_004A2AC7();                                        // 0x004A2AC7
    bool load(SaveStream& stream);                              // 0x004A2AED
    bool save(SaveStream& stream);                              // 0x004A2BEE
    void unk_004A2C6D();                                        // 0x004A2C6D

    Array<Road*> m_roads;  // +0x00
    bool m_c;              // +0x0c
};

inline RoadManager& campaign_roads()
{
    return *(RoadManager*)lbl_018CD818->m_road_manager;
}

#endif

// The strategy map's grid and regions (code\world_map.cpp).
#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "unsorted.h"
#include "core_strings.h"
#include "romans_utils.h"
#include "strategy_map.h"

struct Port;

// A tile's road state in a saved game (0xc bytes).
struct TileRoadRecord {
    TileRoadRecord();                   // 0x0048ADD5

    Road* m_road;                       // the road's address when it was saved
    unsigned short m_road_level : 3;    // +0x04
    unsigned short m_flags15 : 2;
    unsigned short m_flags18 : 1;
    unsigned short m_flags21 : 1;
    unsigned short m_flags24 : 1;
    unsigned short m_flags27 : 1;
    unsigned short m_1c_30 : 1;
    unsigned short m_flags20 : 3;
    unsigned short m_20_3 : 1;
    unsigned short m_pad : 2;
    unsigned short m_x;                 // +0x06
    unsigned short m_y;                 // +0x08
};

// A land region's tile beside the sea (0xc bytes).
struct CoastTile {
    CoastTile(int tile) : m_tile(tile), m_4(-1.0f), m_8(999) {}
    bool operator==(const CoastTile& other) const
    {
        return m_tile == other.m_tile && m_4 == other.m_4 && m_8 == other.m_8;
    }

    int m_tile;                         // its index in the map's grid
    float m_4;
    int m_8;
};

// A region's entry for one of its neighbours (0x54 bytes).
struct RegionNeighbour {
    RegionNeighbour()
        : m_other(0), m_region(-1), m_8(-1.0f), m_c(false), m_road(0), m_14(0), m_18(0), m_24(0), m_30(0), m_3c(0),
          m_48(0)
    {
    }                                   // 0x0048B5D5
    RegionNeighbour(const RegionNeighbour& other)
        : m_other(other.m_other), m_region(other.m_region), m_8(other.m_8), m_c(other.m_c), m_road(other.m_road),
          m_14(other.m_14), m_18(other.m_18), m_24(other.m_24), m_30(other.m_30), m_3c(other.m_3c), m_48(other.m_48)
    {
        // NONMATCHING - the original reserves 4 bytes of stack, this 0x40
    }                                   // 0x0048DA4F
    void add_border_tile(int tile);     // 0x004863DB
    void unk_0048642E();                // 0x0048642E
    void unk_00486561(int region);      // 0x00486561
    void unk_00487915();                // 0x00487915

    Region* m_other;                    // +0x00
    int m_region;                       // +0x04
    float m_8;                          // +0x08
    bool m_c;                           // +0x0c
    Road* m_road;                       // +0x10: the road this region planned towards it
    int m_14;                           // +0x14
    Array<int> m_18;                    // +0x18
    Array<int> m_24;                    // +0x24
    Array<int> m_30;                    // +0x30
    Array<int> m_3c;                    // +0x3c
    Array<int> m_48;                    // +0x48
};

// One tile of the strategy map's grid (0x34 bytes).
struct Tile {
    bool is_sea() const { return m_10 >= 10 && m_10 <= 12; }
    bool is_bridge() const { return m_model >= 0x6c && m_model <= 0x6f; }
    bool is_river();                    // 0x0048A945: a river or a river's source
    int unk_0048A97F();                 // 0x0048A97F: 1 for ground type 8, 3 for type 7, else 0

    StrategyObject* m_objects;          // the first object on it
    Road* m_road;                       // +0x04: the road drawn on it
    char m_pad8[4];
    unsigned short m_model;             // +0x0c: 0x6c-0x6f for bridges, 0xffff for none
    char m_padE[2];
    int m_10;                           // +0x10: the ground type (10-12 sea)
    int m_region;                       // +0x14
    char m_pad18[4];
    unsigned int m_flags0 : 5;          // +0x1c
    unsigned int m_river_source : 1;
    unsigned int m_river : 1;
    unsigned int m_flags7 : 5;
    unsigned int m_road_level : 3;      // 4 for a settlement's tile (a guess)
    unsigned int m_flags15 : 2;
    unsigned int m_landmark : 1;        // a landmark stands on it
    unsigned int m_flags18 : 3;
    unsigned int m_flags21 : 3;         // doubles the cost of entering it
    unsigned int m_flags24 : 3;
    unsigned int m_flags27 : 3;
    unsigned int m_1c_30 : 1;
    unsigned int m_flags31 : 1;
    unsigned int m_flags20 : 3;         // +0x20
    unsigned int m_20_3 : 1;
    unsigned int m_flags20_4 : 4;
    unsigned int m_region_border : 1;   // beside a tile of another land region
    unsigned int m_coast : 1;           // a land tile beside a sea region's tile
    unsigned int m_sea_coast : 1;       // a sea tile beside a land region's tile
    unsigned int m_flags20_11 : 21;
    unsigned char m_24_0 : 1;           // +0x24
    unsigned char m_24_1 : 1;
    unsigned char m_24_2 : 1;           // a structure stands on it
    unsigned char m_24_3 : 1;
    unsigned char m_24_4 : 1;
    unsigned char m_24_5 : 1;
    unsigned char m_24_6 : 1;
    unsigned char m_24_7 : 1;
    unsigned char m_25;                 // the directions it can't be entered from (0xff: none)
    char m_pad26[2];
    unsigned int m_28;                  // +0x28
    int m_2c;                           // +0x2c: 0 when no character is on it (a guess)
    float m_movement_factor;            // +0x30
};

// A region's colour in map_regions.tga.
struct RegionColour {
    unsigned char m_rgb[3];
};

// A unit type and how many of it a revolt in a region raises (not identified).
struct RegionRevoltUnit {
    UnitUnk50* m_type;
    int m_experience;                   // +0x04
    char m_pad8[0x18 - 8];
    float m_count;                      // +0x18
};

// What a region's +0x88's +0x1c is (not identified).
class RegionRevoltUnitList {
public:
    unsigned int unk_004585E5();        // 0x004585E5: how many
};

// What a region's +0x88 is (not identified).
class RegionRevoltUnits {
public:
    RegionRevoltUnit* unk_005BE904(unsigned int index);  // 0x005BE904

    char m_pad0[0x1c];
    RegionRevoltUnitList m_1c;          // +0x1c
};

// What a region's +0x44 holds (not identified).
class RegionUnk44 {
public:
    int unk_38() const { return m_38; }
    void unk_004BF34E(bool, Faction* faction, int);  // 0x004BF34E: told of the settlement's new owner

    char m_pad0[0x38];
    int m_38;                           // +0x38
};

// A region of the strategy map (0x1d8 bytes).
class Region {
public:
    void unk_00483B67();                // 0x00483B67: at the start of each round
    int id() const { return m_id; }
    int unk_00485627(int resource, int);  // 0x00485627: how many of the resource it has (a guess)
    unsigned char road_level() const { return m_road_level; }
    Port* port() const { return m_port; }
    Settlement* settlement() const { return (Settlement*)m_port; }  // what StrategyMap::set_settlement keeps there
    int num_neighbours() const { return m_neighbours.size(); }
    void set_port_road(Road* road) { m_port_road = road; }
    int creator_faction() const { return m_creator_faction; }
    bool is_sea() const { return m_sea; }
    RegionNeighbour* neighbour(int region);  // 0x00483C05: its entry for a neighbouring region, added if missing
    void unk_00483CBF();                     // 0x00483CBF
    void unk_00486079(int tile);             // 0x00486079: adds the tile to m_a4
    void unk_00483D33();                     // 0x00483D33
    void set_road_level(unsigned char level);  // 0x00483E16: 2 at most
    void raise_road_level();                 // 0x00483E6B: 3 at most
    void set_farming_level(unsigned int level);  // 0x00483E8E: 5 at most
    void raise_farming_level();              // 0x00483EAF: 4 at most
    void set_famine_threat(int threat);      // 0x00483ECE: 4 at most
    void raise_famine_threat();              // 0x00483EF8: 4 at most
    int unk_00483F17();                      // 0x00483F17: the port's +0x184
    int unk_00483F36();                      // 0x00483F36: the port's +0x184
    void unk_00484763();                     // 0x00484763
    RebelType* rebel_type();                 // 0x00483BA6
    void unk_004841C6();                     // 0x004841C6
    bool has_rebel_type();                   // 0x00483BC5
    bool has_resource(unsigned int resource) const { return (m_resources[resource >> 5] & (1 << (resource & 0x1f))) != 0; }

    char m_pad0[8];
    String m_settlement_name;           // +0x08: its settlement's name in descr_regions.txt (a guess)
    char m_pad10[0x1c - 0x10];
    int m_id;                           // +0x1c
    unsigned char m_road_level;         // +0x20: 0 none, 1 roads, 2 paved roads, 3 highways
    char m_pad21[3];
    unsigned int m_farming_level;       // +0x24
    int m_famine_threat;                // +0x28
    char m_pad2c[0x44 - 0x2c];
    Array<RegionUnk44*> m_44;           // +0x44
    bool m_sea;                         // +0x50: a sea region
    char m_pad51[0x58 - 0x51];
    Road* m_port_road;                  // +0x58
    char m_pad5c[0x68 - 0x5c];
    int m_creator_faction;              // +0x68: the faction descr_regions.txt names for it (21 for none)
    char m_pad6c[0x88 - 0x6c];
    RegionRevoltUnits* m_88;            // +0x88: more units for a revolt there (not identified)
    char m_pad8c[0x98 - 0x8c];
    Array<CoastTile> m_coast;           // +0x98: its tiles beside the sea
    Array<int> m_a4;                    // +0xa4: tile indices
    RegionReligions m_religions;        // +0xb0 (BI)
    char m_padd8[0x148 - 0xd8];
    Port* m_port;                       // +0x148
    Array<int> m_tiles;                 // +0x14c: its tiles, by index into the map's grid
    char m_pad158[0x164 - 0x158];
    Array<RegionNeighbour> m_neighbours;  // +0x164: in the order the map's scan first finds them
    char m_pad170[0x188 - 0x170];
    unsigned int m_resources[3];        // +0x188: its hidden resources, a bit each
    RoadPoint m_settlement_position;    // +0x194
    RoadPoint m_port_position;          // +0x19c
    RegionColour m_colour;              // +0x1a4: its colour in map_regions.tga
    char m_pad1a7[1];
    Faction* m_owner;                   // +0x1a8: the faction that holds it, or 0
    bool m_1ac;                         // +0x1ac
    char m_pad1ad[3];
    int m_1b0;                          // +0x1b0: a revolt there is 1.5 times as strong while it's set (not identified)
    char m_pad1b4[0x1b8 - 0x1b4];
    UniString m_settlement_display_name;  // +0x1b8: its settlement's name as shown (a guess)
    char m_pad1bc[0x1c0 - 0x1bc];
    String m_rebel_type;                // +0x1c0: its rebel type's name
    char m_pad1c8[0x1d8 - 0x1c8];
};

// The strategy map (probably code\strategy_map.cpp).
class StrategyMap {
public:
    int num_regions() const { return m_region_list.size(); }
    unsigned int width() const { return m_width; }
    unsigned int height() const { return m_height; }
    bool in_bounds(int x, int y) { return x >= 0 && x < (int)width() && y >= 0 && y < (int)height(); }
    bool contains(const RoadPoint& point) { return point.x < width() && point.y < height(); }
    Tile* tile(int x, int y) { return &m_tiles[x + y * m_width]; }
    Tile* tile(int index) { return &m_tiles[index]; }
    Tile* tile(RoadPoint point) { return &m_tiles[point.x + point.y * m_width]; }
    Tile* tile(TilePosition position) { return &m_tiles[position.x + position.y * m_width]; }
    Region* region(int id) { return &m_regions[id]; }
    void add_road(int region, int other, Road* road);  // 0x004812B2
    bool can_cross_river(int from_x, int from_y, int to_x, int to_y);  // 0x0047C6B8: at a bridge, while planning roads
    void add_landmark(int region, Landmark* landmark); // 0x00481251
    void set_settlement(int region, Settlement* settlement);  // 0x004811B7: and the region's owner follows it
    void unk_0048204D(Army* army, int region);         // 0x0048204D
    void unk_00482CC0();                               // 0x00482CC0: at the end of each round in summer
    void unk_00479BE9();                               // 0x00479BE9: filter_coastlines
    unsigned char unk_00471495(RoadPoint position);    // 0x00471495: the tile's unk_0048A97F()
    void unk_00481FA4(int region);                     // 0x00481FA4: sets the region's m_1ac
    void unk_004826A1(TilePosition* position);         // 0x004826A1
    unsigned char unk_00481334(int tile, int* neighbours);  // 0x00481334: the tile's neighbouring tiles (8 at most)
    Region* unk_004829FD(TilePosition* position);      // 0x004829FD: the region of the tile at position
    Region* unk_00482A48(TilePosition* position);      // 0x00482A48: the same
    void unk_004823F9(int region, Faction* faction);   // 0x004823F9: a region's new owner
    void unk_0047FC44(int region);                     // 0x0047FC44
    bool is_sea_region(int region);                    // 0x00481149
    void build_neighbours();                           // 0x0047F003
    bool load_tile_road(SaveStream& stream);           // 0x0047D489
    void unk_0048257A(int region);                     // 0x0048257A
    void unk_00482B16(int region);                     // 0x00482B16

    char m_pad0[0x2c];
    int m_width;                        // +0x2c
    int m_height;                       // +0x30
    char m_pad34[0x48 - 0x34];
    Tile* m_tiles;                      // +0x48: m_width * m_height, by rows from the south
    char m_pad4c[0x70 - 0x4c];
    Region m_regions[199];              // +0x70 (how many is a guess)
    char m_pad16f58[0x17128 - 0x16f58];
    PointerList<Region> m_region_list;  // +0x17128 (a guess: only its count, +0x17130, is known)
};

extern "C" {
extern StrategyMap* lbl_018CD80C;
extern StrategyMap* lbl_018CD86C;  // the strategy map again (the one movement uses?)
extern StrategyMap* lbl_02921A94;  // the strategy map again (the aerial map's copy?)
extern StrategyMap* lbl_02921A34;   // the strategy map again
}

#endif

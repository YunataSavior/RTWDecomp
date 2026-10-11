// Declarations whose source file is not known yet. Move each to its own header once its home is found.
// Unnamed functions and variables keep the names tools/split.py gives them (fn_/lbl_ and their address), declared
// extern "C" so that they compile to those names.
#ifndef UNSORTED_H
#define UNSORTED_H

#include <windows.h>

#include "for_scope.h"

// A string of 16-bit characters (probably ca_libs\core\core_string.cpp).
class UniString {
public:
    UniString(const char* text);                // 0x00F338B0
    ~UniString();                               // 0x00F35440
    unsigned short length() const;              // 0x00F38200
    void truncate(unsigned short length);       // 0x00F37C00
    const unsigned short* chars() const;        // 0x00F37A20

private:
    void* m_data;
};

// A list of pointers (12 bytes: its items, its capacity and its count; the fields' order is a guess).
template <class T>
class PointerList {
public:
    unsigned int size() const { return m_count; }

    T** m_items;
    int m_capacity;
    int m_count;
};

// CA's growable array (probably ca_libs): its items, its capacity and its count. Its functions that are not inline
// are compiled once per item type, and the linker folded the copies whose code is the same.
template <class T>
class Array {
public:
    class iterator {
    public:
        iterator(T* p) : m_p(p) {}
        T& operator*() const { return *m_p; }
        T* operator->() const { return m_p; }
        iterator& operator++() { ++m_p; return *this; }
        bool operator==(const iterator& other) const { return m_p == other.m_p; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

        T* m_p;
    };

    Array();                                // 0x005F4004 (4-byte items), 0x00EB9407 (8-byte items)
    ~Array();                               // 0x00A48114

    unsigned int size() const { return m_size; }
    T& operator[](int index) { return m_data[index]; }
    T* data() { return m_data; }
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    iterator last() { return m_data + m_size - 1; }
    void clear() { while (m_size != 0) pop_back(); }
    void pop_back() { m_size--; }
    T& back() { return m_data[m_size - 1]; }

    T* push_back(const T& item);            // 0x006FAF63 (4-byte items), 0x009DDF80 (8-byte items)
    void erase(int index);                  // 0x00D1CD66 (8-byte items)
    iterator find(const T& item);           // 0x0097344A (4-byte items)
    T* add_unique(const T& item);           // 0x004A2C76 (Array<int>)

    T* m_data;
    unsigned int m_capacity;
    unsigned int m_size;

private:
    static T* allocate(unsigned int count)
    {
        T* data = (T*)::operator new((count * sizeof(T) + 3) / 4 * 4);
        return data;
    }

    static void deallocate(T* data)
    {
        void* block = data;
        ::operator delete(block);
    }
};

template <class T>
Array<T>::Array()
{
    m_capacity = 8;
    m_size = 0;
    m_data = allocate(m_capacity);
}

template <class T>
Array<T>::~Array()
{
    clear();
    deallocate(m_data);
}

template <class T>
void Array<T>::erase(int index)
{
    m_size--;
    for (unsigned int i = index; i < m_size; i++)
        m_data[i] = m_data[i + 1];
}

template <class T>
Array<T>::iterator Array<T>::find(const T& item)
{
    iterator last = end();
    iterator first = begin();
    for (iterator it = first; it != last; ++it)
        if (item == *it)
            return it;
    return last;
}

// (Not matching yet: the original builds find's result in place and has five more unused slots, as if VC6 had
// started to inline find.)
template <class T>
T* Array<T>::add_unique(const T& item)
{
    iterator found = find(item);
    iterator end_it = end();
    if (found == end_it)
        return push_back(item);
    return found.m_p;
}

// The game's allocator records each block's owner and the line that made it (probably ca_libs). Each source file
// declares THIS_FILE, its own path, for these.
void* operator new(unsigned int size, const char* owner, const char* file, int line, bool flag);  // 0x00F490F0
#define NEW new (THIS_FILE, __FILE__, __LINE__, true)

extern "C" {
void* fn_00F49080(void* p, const char* owner, const char* file, int line, int* count, int* size);  // find a block
void fn_00F49060(void* block);                                                                     // free a block
}

template <class T>
void debug_delete(T* p, const char* owner, const char* file, int line)
{
    int size;
    int count;
    void* allocation = fn_00F49080(p, owner, file, line, &count, &size);
    if (allocation) {
        for (int i = 0; i < count; i++)
            p[i].~T();
        fn_00F49060(allocation);
    }
}

#define CA_DELETE(p) debug_delete(p, THIS_FILE, __FILE__, __LINE__)

// Each part of a save game has a version, registered at startup (code\version_registry.cpp).
class VersionRegistry {
public:
    static VersionRegistry* instance();                     // 0x0042654A
    static void add(const char* name, int version);         // 0x004265D2
    int version(const char* name);                          // 0x00426599: -1 if not registered
};

inline int saved_version(const char* name)
{
    return VersionRegistry::instance()->version(name);
}

struct VersionRegistrar {
    VersionRegistrar(const char* name, int version) { VersionRegistry::add(name, version); }
};

class Landmark;
class Region;
class Road;
class SaveStream;

// An 8-bit string (probably ca_libs).
class String {
public:
    String() { m_chars = 0; m_4 = 0; }
    ~String();                                  // 0x00E2C38E
    const char* c_str() const { return m_chars; }
    void save(SaveStream& stream);              // 0x00E2C836
    void load(SaveStream& stream);              // 0x00E2C8CD

    char* m_chars;
    int m_4;
};

// A settlement (probably code\settlement.cpp).
class Settlement {
public:
    int faction() const { return m_faction; }

    char m_pad0[0xa0];
    int m_faction;                      // +0xa0
    char m_pada4[0x1d0 - 0xa4];
    int m_wonder_order;                 // +0x1d0: public order for capturing a wonder, 4 then down by 1 a turn
};

// An object's tile on the strategy map.
struct TilePosition {
    TilePosition() { x = 0; y = 0; }

    unsigned int x;
    unsigned int y;
};

struct Vector3 {
    Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}

    float m_x;
    float m_y;
    float m_z;
};

// A tile of the strategy map, by its coordinates (y from the south).
struct RoadPoint {
    RoadPoint() { x = 0; y = 0; }
    bool operator==(const RoadPoint& other) const
    {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }
    bool operator!=(const RoadPoint& other) const { return !(*this == other); }

    int x;
    int y;
};

// A region's entry for one of its neighbours (0x54 bytes).
struct RegionNeighbour {
    char m_pad0[4];
    int m_region;                       // +0x04
    char m_pad8[0x10 - 8];
    Road* m_road;                       // +0x10: the road this region planned towards it
    char m_pad14[0x54 - 0x14];
};

// A region's port (probably code\port.cpp).
struct PortUnkA98 {
    char m_pad0[0x78];
    int m_78;                           // +0x78
};

struct Port {
    int unk_184() const { return m_184; }
    PortUnkA98* unk_a98() const { return m_a98; }

    char m_pad0[0x184];
    int m_184;                          // +0x184
    char m_pad188[0xa98 - 0x188];
    PortUnkA98* m_a98;                  // +0xa98
};

// One tile of the strategy map's grid (0x34 bytes).
struct Tile {
    char m_pad0[4];
    Road* m_road;                       // +0x04: the road drawn on it
    char m_pad8[0x10 - 8];
    int m_10;                           // +0x10
    int m_region;                       // +0x14
    char m_pad18[4];
    unsigned int m_flags0 : 5;          // +0x1c
    unsigned int m_river_source : 1;
    unsigned int m_river : 1;
    unsigned int m_flags7 : 5;
    unsigned int m_road_level : 3;      // 4 for a settlement's tile (a guess)
    unsigned int m_flags15 : 2;
    unsigned int m_landmark : 1;        // a landmark stands on it
    unsigned int m_flags18 : 14;
    char m_pad20[4];
    unsigned int m_24 : 8;              // +0x24
    unsigned int m_25 : 8;
    unsigned int m_26 : 16;
    char m_pad28[0x34 - 0x28];
};

// A region of the strategy map (0x1d8 bytes).
class Region {
public:
    int id() const { return m_id; }
    unsigned char road_level() const { return m_road_level; }
    Port* port() const { return m_port; }
    int num_neighbours() const { return m_neighbours.size(); }
    void set_port_road(Road* road) { m_port_road = road; }
    void unk_00483E16(unsigned char level);  // 0x00483E16
    void unk_00483E6B();                     // 0x00483E6B
    void unk_00484763();                     // 0x00484763

    char m_pad0[0x1c];
    int m_id;                           // +0x1c
    unsigned char m_road_level;         // +0x20: 0 none, 1 roads, 2 paved roads, 3 highways
    char m_pad21[0x58 - 0x21];
    Road* m_port_road;                  // +0x58
    char m_pad5c[0x148 - 0x5c];
    Port* m_port;                       // +0x148
    Array<int> m_tiles;                 // +0x14c: its tiles, by index into the map's grid
    char m_pad158[0x164 - 0x158];
    Array<RegionNeighbour> m_neighbours;  // +0x164: in the order the map's scan first finds them
    char m_pad170[0x194 - 0x170];
    RoadPoint m_settlement_position;    // +0x194
    RoadPoint m_port_position;          // +0x19c
    char m_pad1a4[0x1a8 - 0x1a4];
    Settlement* m_settlement;           // +0x1a8
    char m_pad1ac[0x1d8 - 0x1ac];
};

// A set of factions, a bit each.
class FactionSet {
public:
    int bits() const { return m_bits; }

    int m_bits;
};

// A faction's entry in descr_sm_factions (probably code\faction_db.cpp).
class FactionDescription {
public:
    bool has_18() const { return m_18 < 0xffffffff; }

    char m_pad0[0x18];
    unsigned int m_18;                  // +0x18 (0xffffffff for most factions; not identified)
    char m_pad1c[0xa4 - 0x1c];
    bool m_custom_battle_availability;  // +0xa4
    char m_pada5[2];
    bool m_a7;                          // +0xa7
};

// A faction's campaign record (probably code\faction.cpp).
class Faction {
public:
    unsigned int num_regions() const { return m_regions.size(); }
    FactionSet neighbours() const { return m_neighbours; }
    bool no_cash_crisis() const { return m_4ec == 0; }  // (what +0x4ec means is not known)
    bool has_cash_crisis() const { return !no_cash_crisis(); }
    bool unk_4f4() const { return m_4f4; }

    bool unk_00445B2B();                // 0x00445B2B: description's +0xa4 set and +0xa7 clear
    bool unk_004584F1();                // 0x004584F1: has_cash_crisis() or +0x4f0 is a faction

    char m_pad0[0xb4];
    FactionDescription* m_description;  // +0xb4
    int m_control;                      // +0xb8 (1: human)
    char m_padbc[0x100 - 0xbc];
    PointerList<Region> m_regions;      // +0x100
    char m_pad10c[0x13c - 0x10c];
    FactionSet m_neighbours;            // +0x13c: the factions whose regions border this one's
    char m_pad140[0x4ec - 0x140];
    int m_4ec;                          // +0x4ec
    int m_4f0;                          // +0x4f0 (a faction index, 21 for none)
    bool m_4f4;                         // +0x4f4
    char m_pad4f5[0x504 - 0x4f5];
    int m_treasury;                     // +0x504
};

// The C runtime's rand(), kept per object (probably ca_libs).
class Random {
public:
    int percent();                      // 0x00F3AAD0: 0 to 99

    int m_seed;
};

// How one faction stands towards another.
class DiplomaticRelation {
public:
    char m_pad0[4];
    int m_stance;                       // +0x04: 0 allied, 600 at war
    char m_pad8[0x74 - 8];
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
    int stance(int faction, int other) const { return m_factions[faction].stance(other); }
    bool is_at_war(int faction);        // 0x005A11E9: at war with anyone

    FactionRelations m_factions[21];
};

// The campaign (probably code\campaign.cpp).
class Campaign {
public:
    Faction* faction(int index);        // 0x004548DD
    int difficulty(int faction);        // 0x004529E3: 0 easy to 3 very hard; always 1 for the AI
    bool is_faction_alive(int faction); // 0x0045491C
    int human_faction(int index);       // 0x00457158: the index'th human faction, or 21
    bool is_prologue() const { return m_prologue; }
    int unk_2e4() const { return m_2e4; }

    char m_pad0[0x2b5];
    bool m_prologue;                    // +0x2b5 (a guess: it lowers brigand spawns and changes taxes and garrisons)
    char m_pad2b6[0x2e4 - 0x2b6];
    int m_2e4;                          // +0x2e4 (compared with 100; perhaps the turn number)
    char m_pad2e8[0x528 - 0x2e8];
    Random m_random;                    // +0x528
    DiplomacyMatrix m_diplomacy;        // +0x52c
    char m_padcd00[0xcd4c - 0xcd00];
    char m_road_manager[0x10];          // +0xcd4c: a RoadManager (road.h's campaign_roads())
};

// The strategy map (probably code\strategy_map.cpp).
class StrategyMap {
public:
    int num_regions() const { return m_region_list.size(); }
    unsigned int width() const { return m_width; }
    unsigned int height() const { return m_height; }
    Tile* tile(int x, int y) { return &m_tiles[x + y * m_width]; }
    Tile* tile(int index) { return &m_tiles[index]; }
    Tile* tile(RoadPoint point) { return &m_tiles[point.x + point.y * m_width]; }
    Tile* tile(TilePosition position) { return &m_tiles[position.x + position.y * m_width]; }
    Region* region(int id) { return &m_regions[id]; }
    void add_road(int region, int other, Road* road);  // 0x004812B2
    void add_landmark(int region, Landmark* landmark); // 0x00481251
    void unk_0047FC44(int region);                     // 0x0047FC44
    bool unk_00481149(int region);                     // 0x00481149: the region's +0x50
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

// The campaign map's path planning (code\strategy_map.cpp).
class PathFinder {
public:
    int find_road_path(const RoadPoint& from, const RoadPoint& to);  // 0x004AE64B
    void plan_roads(int region, unsigned char level);               // 0x004B1B5B
    void build_roads(int region, unsigned char level);              // 0x004B21E2
    bool unk_004A605B(const RoadPoint& point);                      // 0x004A605B
    int unk_004A7EA9(Settlement* settlement, RoadPoint* point);     // 0x004A7EA9
    void unk_004ABED9(Landmark* landmark, TilePosition* position);  // 0x004ABED9
    unsigned char unk_004B3124(TilePosition position);              // 0x004B3124
    void unk_004AC914(const RoadPoint& point);                      // 0x004AC914
    void unk_004B64C3(int);                                         // 0x004B64C3

    char m_pad0[0x14];
    int m_status;                       // +0x14: of the last search (0 found, 1 bad goal, 2 no path)
    char m_pad18[0xbc - 0x18];
    Array<RoadPoint> m_path;            // +0xbc: the last path found, goal first
};

// A node of the A* search.
struct PathNode {
    char m_pad0[0x10];
    RoadPoint m_point;                  // +0x10
    PathNode* m_parent;                 // +0x18
};

// The campaign map's A* search (code\strategy_map_pathfinding.cpp).
class AStar {
public:
    PathNode* search(int mode, const RoadPoint& from, const RoadPoint& to, bool diagonal);  // 0x004B84E5
};

// What the AI knows about each faction (an AI unit's globals).
struct AIFactionSummary {
    int m_value;                        // positive when the faction is a threat to someone (not identified)
    char m_pad4[0x40 - 4];
};

struct AIFactionPair {
    bool m_strongest_neighbour;         // set in 0x004E18FA
    char m_pad1[0xc - 1];
};

// A save game being written or read (probably ca_libs).
class SaveStream {
public:
    virtual void unk_0();
    virtual void unk_4();
    virtual void fail();                     // +0x08 (a guess)
    virtual bool is_open();                  // +0x0c (a guess)

    bool unk_00F3D990();                     // 0x00F3D990 (checked before writing)
    void write(const void* data, int size);  // 0x00F3D8B0
    void read(int& value);                   // 0x00412D90
    void read(void* data, int size);         // 0x00412DD7
    void read(unsigned short& value);        // 0x004682A6

    template <class T>
    void write_value(const T& value)
    {
        write(&value, sizeof(T));
    }

    int read_int()
    {
        int value;
        read(value);
        return value;
    }

    unsigned short read_ushort()
    {
        unsigned short value;
        read(value);
        return value;
    }
};

// The rounding the game uses for float to int (a math header's inline assembly).
inline int float_to_int(float value)
{
    __asm {
        push eax
        fld value
        frndint
        fistp dword ptr [esp]
        pop eax
    }
}

extern "C" {
extern Campaign* lbl_018CD818;
extern StrategyMap* lbl_018CD80C;
extern int lbl_01181378;  // the rebel faction's index
extern AIFactionSummary lbl_012D0F78[21];
extern AIFactionPair lbl_012D1552[21 * 21];
extern const float lbl_00FE4970;  // 1.0

int fn_004E335E(int faction);

// Landmarks (code\landmark.cpp's callees)
int fn_00E3C9F6(const char* name);                     // a landmark type from its name
Landmark* fn_008F80F8(const String& name);             // creates a landmark (code\landmark_db.cpp)
void fn_00CB5468(Settlement* settlement, Landmark* landmark, TilePosition* position, int);
void fn_00CB54E7(Settlement* settlement, Landmark* landmark, TilePosition* position);
unsigned char fn_009CB2A7(unsigned int x, unsigned int y);
Vector3 fn_009C90D2(float x, float z, bool, bool, bool);
void* fn_0041374F(void* saved);                         // a saved pointer's new value

extern StrategyMap* lbl_018CD86C;  // the strategy map again (the one movement uses?)
extern StrategyMap* lbl_02921A94;  // the strategy map again (the aerial map's copy?)
extern PathFinder* lbl_029219E0;
extern AStar* lbl_012D07C0;
extern int lbl_012D0604;           // the movement rules in force (3 while planning roads)
extern void* lbl_015FC11C;         // the aerial map, when there is one

// Saved pointers (probably ca_libs)
void fn_0041369B(void* object, int saved_address);
void fn_0041391C(SaveStream* stream);
void fn_0041393B(SaveStream* stream, int);

// The aerial map's roads (probably code\display\aerial_map_roads.cpp)
int fn_009CA89F(int region, Road* road);
int fn_009CA94C(int region, Road* road);
void fn_009CAAA4(int handle, Road* road);
void fn_009CAB2E(int region, unsigned char level);
void fn_009CAB82(int handle, float alpha);
void fn_009CABA1(int handle, Array<RoadPoint>& points);

bool fn_00EFA483(int);
void fn_00425B4F();
void fn_00F32ED0(int);
void fn_00402805();
void fn_00F3A500(const char*);
bool fn_00408DCF();
int fn_00403412(HINSTANCE, HINSTANCE);
int fn_00425F8F(HINSTANCE, HINSTANCE);

extern char* lbl_012CA6B0;
extern bool lbl_012CB1F2;
extern char lbl_012CB1F4[];
extern int lbl_012CB0D8;
extern bool lbl_012CE880;
}

#include "common_constants.h"

#endif

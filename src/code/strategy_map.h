// The campaign map's path planning (code\strategy_map.cpp).
#ifndef STRATEGY_MAP_H
#define STRATEGY_MAP_H

#include "unsorted.h"
#include "env/tracked_object.h"
#include "extents_manager.h"

struct Tile;

// A tile of the strategy map, by its coordinates (y from the south).
struct RoadPoint {
    RoadPoint() { x = 0; y = 0; }
    RoadPoint(int index);               // 0x004A4309: the position of the map's tile index
    bool operator==(const RoadPoint& other) const
    {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }
    bool operator!=(const RoadPoint& other) const { return !(*this == other); }
    bool unk_004B61C2(const RoadPoint& other) const;  // 0x004B61C2

    int x;
    int y;
};

// What PathFinder::unk_004A76B7 finds on a tile (not identified).
class TileStructureOwner : public StrategyObject {
public:
    virtual void unk_74();
    virtual void unk_78();
    virtual void unk_7c();
    virtual void unk_80();
    virtual void unk_84();
    virtual void unk_88();
    virtual void unk_8c();
    virtual void unk_90();
    virtual void unk_94();
    virtual void unk_98();
    virtual void unk_9c();
    virtual Faction* unk_a0();          // +0xa0: its faction (a guess)
};

struct TileStructure {
    char m_pad0[0x30];
    TileStructureOwner* m_30;           // +0x30
};

// What PathFinder::unk_004A74BA returns (not identified).
struct TileUnk84 {
    char m_pad0[0x84];
    unsigned char m_84;                 // +0x84
};

// What a PathFinder's +0x6c is (not identified).
class PathFinderUnk6C {
public:
    void unk_0056CEC4(Character* character);  // 0x0056CEC4

    char m_pad0[4];
};

// The campaign map's path planning (code\strategy_map.cpp).
class PathFinder {
public:
    int find_road_path(const RoadPoint& from, const RoadPoint& to);  // 0x004AE64B
    void unk_004AFB1E(TilePosition* position);                      // 0x004AFB1E
    void plan_roads(int region, unsigned char level);               // 0x004B1B5B
    void build_roads(int region, unsigned char level);              // 0x004B21E2
    bool unk_004A605B(const RoadPoint& point);                      // 0x004A605B
    int unk_004A5231(RoadPoint* settlement, int);                   // 0x004A5231
    int unk_004A5879(RoadPoint* port);                              // 0x004A5879
    Army* unk_004AAC55(TilePosition* position);                     // 0x004AAC55
    int unk_004A7EA9(Faction* faction, RoadPoint* point);           // 0x004A7EA9
    void unk_004ABED9(StrategyObject* object, TilePosition* position);  // 0x004ABED9
    unsigned char unk_004B3124(TilePosition position);              // 0x004B3124
    void unk_004AC914(const RoadPoint& point);                      // 0x004AC914
    void set_character(Character* character);                       // 0x004B64C3: 0 for roads
    void unk_004A4E73();                                            // 0x004A4E73: before a load
    void unk_004B3011();                                            // 0x004B3011
    void unk_004B31E9();                                            // 0x004B31E9: at the start of each round
    void unk_004A434A();                                            // 0x004A434A
    void unk_004AFADF();                                            // 0x004AFADF
    bool is_blocked(const RoadPoint& from, const RoadPoint& to, Tile* tile);  // 0x004A6862: by the terrain
    int unk_004AB253(const RoadPoint& from, const RoadPoint& to);   // 0x004AB253
    bool unk_004AB325(Character* character);                        // 0x004AB325
    Army* unk_004AADAE(TilePosition* position);                     // 0x004AADAE: the army there, or 0
    void foreign_spies(Settlement* settlement, Array<Character*>* spies);  // 0x004A9D01
    void unk_004A9C59(Settlement* settlement, Array<Character*>* spies);   // 0x004A9C59: the same, by unk_00580C1E
    void unk_004A99D9(TilePosition* position, Array<Character*>* characters, Faction* faction, int type);  // 0x004A99D9:
                                                                    //   the faction's living characters of a type there
    void unk_004AB5CC(Settlement* settlement);                      // 0x004AB5CC
    void unk_004A4C7F();                                            // 0x004A4C7F: at the start of each faction's turn
    bool unk_004B13A2(int, int from, int to, Array<int>* path, float* cost, int);  // 0x004B13A2: a land route
                                                                    //   between two regions (a guess)
    void unk_004ACEB2(float* cost);                                 // 0x004ACEB2: a cost per turn (a guess)
    void unk_004AC808(Character* character);                        // 0x004AC808
    void unk_004AC1DB(Settlement* settlement);                      // 0x004AC1DB
    void unk_004AC371(Settlement* settlement);                      // 0x004AC371
    int unk_004A7D84(Faction* faction, const RoadPoint& point);     // 0x004A7D84
    int can_move(const RoadPoint& from, const RoadPoint& to, const RoadPoint& goal);  // 0x004B52BA: 1 when it can
    TileStructure* unk_004A76B7(const RoadPoint& point);           // 0x004A76B7: the tile's object of type 0x1d
    int unk_004A7CA9(Faction* faction, const RoadPoint& point);     // 0x004A7CA9
    TileUnk84* unk_004A74BA(const RoadPoint& point);                // 0x004A74BA
    int unk_004AB135(const RoadPoint& point, int);                  // 0x004AB135
    bool unk_004A6DA0(const RoadPoint& point, Character* character);  // 0x004A6DA0
    bool unk_004A7097(const RoadPoint& point, int, int);            // 0x004A7097
    Character* unk_004A95F3(Faction* faction, const RoadPoint& point, bool* flag);  // 0x004A95F3
    bool unk_004AC9E5(Faction* faction, Character* character);      // 0x004AC9E5
    bool unk_004AA85D(const RoadPoint& point);                      // 0x004AA85D: m_25 isn't 0xff, and bits 0, 1, 3, 5
                                                                    //   and 6 of +0x24 are clear
    void unk_004B68A4(Character* character, const RoadPoint& point, Array<int>* tiles, int);  // 0x004B68A4
    int unk_004A7F3C(Faction* faction, const RoadPoint& point);     // 0x004A7F3C
    float step_cost(const RoadPoint& from, const RoadPoint& to);    // 0x004B634D

    Character* character() const { return m_watched.m_character; }
    bool is_mode_3() const { return m_c == 3; }

    char m_pad0[4];
    WatchedCharacter m_watched;         // +0x04: the character whose path it is, or 0 (for roads)
    int m_c;                            // +0x0c: 3 for ships? (a guess)
    Faction* m_faction;                 // +0x10: the character's faction, or 0 (for roads)
    int m_status;                       // +0x14: of the last search (0 found, 1 bad goal, 2 no path)
    char m_pad18[4];
    int m_1c;                           // +0x1c: set to 3 at the start of each faction's turn
    char m_pad20[0x4c - 0x20];
    Character* m_4c;                    // +0x4c: the interface selects one of these three at the start of the
    Settlement* m_50;                   // +0x50:   player's turn (a guess)
    Fort* m_54;                         // +0x54
    char m_pad58[0x6c - 0x58];
    PathFinderUnk6C m_6c;               // +0x6c
    char m_pad70[0x84 - 0x70];
    ExtentsManager m_84;                // +0x84
    char m_pad88[0xbc - 0x88];
    Array<RoadPoint> m_path;            // +0xbc: the last path found, goal first
};

extern "C" {
extern PathFinder* lbl_029219E0;
extern PathFinder* lbl_02921A1C;    // the path finder again
inline PathFinder* path_finder()
{
    return lbl_02921A1C;
}
}

#endif

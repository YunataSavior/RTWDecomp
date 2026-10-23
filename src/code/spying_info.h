// What each faction knows of a map object (code\spying_info.cpp).
#ifndef SPYING_INFO_H
#define SPYING_INFO_H

#include "maths_random.h"
#include "bit_set.h"

class Army;
class Faction;

// What a faction may see of each kind of object at each level of information: a bit per item (filled in
// code\spying_info_db.cpp, 0x009101F9).
class SpyingInfoDB {
public:
    enum Kind {
        CHARACTER,
        AGENT,                          // a hidden spy or assassin (CharacterSpyingInfo::can_see)
        ARMY,
        UNIT,
        SETTLEMENT,
        FORT,
        BUILDING,
        NUM_KINDS
    };
    typedef BitSet<32> Levels[4];

    Levels& kind(int kind) { return m_info[kind]; }

    BitSet<32> m_info[NUM_KINDS][4];
};

extern "C" SpyingInfoDB lbl_01589168;

// A map object's spying information: each faction's level of information on it, from -1 (none) to 3 (the most),
// which decides what of it that faction may see (SpyingInfoDB). Each kind of object has its own subclass, with its
// own copies of the same methods.
class SpyingInfo {
public:
    SpyingInfo();                       // 0x004A3279
    virtual ~SpyingInfo() {}
    virtual bool can_see(int faction, unsigned int item) = 0;  // +0x04: whether FACTION may see ITEM of it

    char m_level[21];                   // +0x04: each faction's level
    Random m_random;                    // +0x1c: for what a faction sees of it (ArmySpyingInfo::estimate)
    int m_seed;                         // +0x20: m_random's seed, or -1 until one is drawn
};

// A character's (Character's base at +0x30).
class CharacterSpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A338C
    virtual Faction* faction();                     // +0x08
    virtual int type();                             // +0x0c: 0 spy, 1 assassin, 2 diplomat, 3 admiral, 4 general,
                                                    //   5 named character (a script command's names, 0x00B0EFF1)
    virtual Army* kind_army();                      // +0x10: the army (a guess)
    virtual int unk_14();                           // +0x14: an agent counts as hidden when this or +0x18 is set
    virtual int unk_18();                           // +0x18

    int seed();                         // 0x004A32D2
    int raise(int faction, int level);  // 0x004A332A: returns FACTION's level
    void reset(int faction);            // 0x004A336F: FACTION knows nothing of it
};

// A settlement's.
class SettlementSpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A3528
    virtual Faction* faction();                     // +0x08

    int seed();                         // 0x004A3471
    void raise(int faction, int level); // 0x004A34C9
    void reset(int faction);            // 0x004A350B
};

// A building's.
class BuildingSpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A3602
    virtual bool unk_08();                          // +0x08: every faction sees all of it when this or +0x0c is set
    virtual bool unk_0c();                          // +0x0c
    virtual Faction* faction();                     // +0x10

    void raise(int faction, int level); // 0x004A35C0
};

// A fort's.
class FortSpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A377C
    virtual Faction* faction();                     // +0x08

    int seed();                         // 0x004A36C5
    void raise(int faction, int level); // 0x004A371D
    void reset(int faction);            // 0x004A375F
};

// An army's (Army's base at +0xc).
class ArmySpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A39A1
    virtual Faction* faction();                     // +0x08

    int seed();                         // 0x004A3814
    void reseed();                      // 0x004A386C: m_random starts again from the seed
    int estimate(float accuracy, int value);    // 0x004A388E: VALUE as a faction with ACCURACY sees it
    void reset(int faction);            // 0x004A3984
};

// A unit's (Unit's base at +0xc).
class UnitSpyingInfo : public SpyingInfo {
public:
    bool can_see(int faction, unsigned int item);   // 0x004A3A9D
    virtual int faction_id();                       // +0x08
    virtual void unk_0c();                          // +0x0c
    virtual void unk_10();                          // +0x10
    virtual bool unk_14();                          // +0x14: a level of 0 counts as 1 when it's set

    int raise(int faction, int level);  // 0x004A3A39: returns FACTION's level
};

#endif

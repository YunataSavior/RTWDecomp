#ifndef LANDMARK_H
#define LANDMARK_H

#include "unsorted.h"

#include "core_linked_list.h"

// In .rdata after common_constants.h's, unreferenced (landmark_db.cpp has the last two as well).
const float k_2000 = 2000.0f;
const float k_0_00001 = 0.00001f;
const float k_0_007 = 0.007f;

static const ConstInt NUM_LANDMARK_TYPES(9);  // the wonders' type IDs are 1 to 9
static const float QUARTER_PI = PI * 0.25f;

class Landmark;
class FogOfWarEntry;

// A landmark's entry in descr_sm_landmarks (code\landmark_db.cpp).
struct LandmarkDescription {
    char m_pad0[8];
    int m_effect;                       // +0x08: what LandmarkManager::update plays at it
    char m_padc[0x14 - 0xc];
    String m_name;                      // +0x14: its type, as descr_landmarks_lookup names it
};

// Told when a landmark changes hands.
struct LandmarkEvent {
    LandmarkEvent(Settlement* settlement, Landmark* landmark) : m_settlement(settlement), m_landmark(landmark) {}

    Settlement* m_settlement;
    Landmark* m_landmark;
};

// A landmark on the strategy map: a wonder (0x44 bytes). Its constructor, destructor and vtable are
// landmark_db.cpp's, which creates them.
class Landmark {
public:
    virtual ~Landmark();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
    virtual void unk_20();
    virtual void unk_24();
    virtual void unk_28();
    virtual void unk_2c();
    virtual void unk_30();
    virtual void unk_34();
    virtual void unk_38();
    virtual void unk_3c();
    virtual void unk_40();
    virtual void unk_44();
    virtual void unk_48();
    virtual void unk_4c();
    virtual void unk_50();
    virtual void unk_54();
    virtual void unk_58();
    virtual void unk_5c();
    virtual void unk_60();
    virtual void unk_64();
    virtual void unk_68();
    virtual void unk_6c();
    virtual TilePosition* position();                                 // +0x70
    virtual FogOfWarEntry* unk_74();                                  // +0x74: 0x0049CE2D

    void set_settlement(Settlement* settlement, bool captured);      // 0x0049CD32
    Settlement* settlement() const { return m_settlement; }
    int region() const { return m_region; }

    char m_pad4[0x24 - 4];
    ObserverList<LandmarkEvent> m_observers;  // +0x24
    int m_type;                               // +0x34: 1 pyramids_and_sphinx to 9 gardens
    int m_region;                             // +0x38
    Settlement* m_settlement;                 // +0x3c: the settlement that holds it, or 0
    LandmarkDescription* m_description;       // +0x40
};

// The campaign's landmarks.
class LandmarkManager {
public:
    void add(Landmark* landmark);             // 0x0049CE73
    void remove(Landmark* landmark);          // 0x0049CF4A
    int num_held(int faction);                // 0x0049CFBA: for the Hold_all_wonders triumph
    Landmark* find(int type);                 // 0x0049D035
    void unk_0049D091();                      // 0x0049D091
    void clear();                             // 0x0049D09A
    void update();                            // 0x0049D0E9
    bool load(SaveStream& stream);            // 0x0049D302
    bool save(SaveStream& stream);            // 0x0049D455
    void resolve_settlements();               // 0x0049D535: after loading

    unsigned int size() const { return m_landmarks.size(); }

    Array<Landmark*> m_landmarks;
};

#endif

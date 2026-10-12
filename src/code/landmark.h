#ifndef LANDMARK_H
#define LANDMARK_H

#include "unsorted.h"

#include "core_linked_list.h"

// In .rdata after common_constants.h's, unreferenced (landmark_db.cpp has the last two as well).
const float k_2000 = 2000.0f;
const float k_0_00001 = 0.00001f;
const float k_0_007 = 0.007f;

class Landmark;
class FogOfWarEntry;
class StringTable;

// A model drawn on the strategy map (not identified).
class Model {
public:
    char m_pad0[0x11d];
    bool m_11d;                         // +0x11d
};

// How a model is loaded (not identified).
enum ModelFlags {
    MODEL_FLAG_4 = 4,
    MODEL_FLAG_18 = 0x18,
    MODEL_FLAG_100 = 0x100,
    MODEL_FLAG_800 = 0x800,
};

inline ModelFlags operator|(const ModelFlags& a, const ModelFlags& b)
{
    return ModelFlags(int(a) | int(b));
}

extern "C" Model* fn_00ADE246(const char* path, ModelFlags flags, int, bool, bool, bool);  // loads a model

// A wonder type's entry in descr_sm_landmarks.txt (0x54 bytes; code\landmark_db.cpp keeps the 10).
struct LandmarkDescription {
    LandmarkDescription() : m_model(0), m_title(0), m_short_description(0), m_long_description(0), m_effects(0) {}
    const String& name() const { return m_name; }
    Model* model() const { return m_model; }
    void load_model(const char* file, ModelFlags flags) { m_model = fn_00ADE246(file, flags, 2, true, true, true); }

    String m_type;                          // +0x00: its "type" line
    Model* m_model;                         // +0x08: loaded from m_item; what LandmarkManager::update draws
    String m_item;                          // +0x0c: the model's file
    String m_name;                          // +0x14: its type, as descr_landmarks_lookup names it
    String m_julii_rtm;                     // +0x1c
    String m_brutii_rtm;                    // +0x24
    String m_scipii_rtm;                    // +0x2c
    String m_movie;                         // +0x34
    String m_image;                         // +0x3c
    const UniString* m_title;               // +0x44: from landmarks.txt, by "<type>_title"
    const UniString* m_short_description;   // +0x48: "<type>_short_descr"
    const UniString* m_long_description;    // +0x4c: "<type>_long_descr"
    const UniString* m_effects;             // +0x50: "<type>_effects"
};

// Told when a landmark changes hands.
struct LandmarkEvent {
    LandmarkEvent(Settlement* settlement, Landmark* landmark) : m_settlement(settlement), m_landmark(landmark) {}

    Settlement* m_settlement;
    Landmark* m_landmark;
};

// A landmark on the strategy map: a wonder (0x44 bytes). code\landmark_db.cpp creates them.
class Landmark : public StrategyObject, public ObserverList<LandmarkEvent> {
public:
    Landmark() : m_type(10), m_settlement(0), m_description(0) {}
    virtual FogOfWarEntry* unk_74();                                  // +0x74: 0x0049CE2D

    void set_settlement(Settlement* settlement, bool captured);      // 0x0049CD32
    Settlement* settlement() const { return m_settlement; }
    int region() const { return m_region; }

    int m_type;                               // +0x34: 1 pyramids_and_sphinx to 9 gardens (10 for none)
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

// code\landmark_db.cpp
bool load_landmark_descriptions();            // 0x008F7A09: reads descr_sm_landmarks.txt
bool load_landmark_models();                  // 0x008F7FCD
void unload_landmark_descriptions();          // 0x008F80A3
Landmark* create_landmark(const String& type);  // 0x008F80F8: 0 for an unknown type
Model*& landmark_model(int type);             // 0x008F81AF

#endif

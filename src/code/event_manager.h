// The campaign's events and disasters (code\event_manager.cpp).
#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "unsorted.h"
#include "calender.h"

// The disasters of descr_events.txt, by their index in the exe's list of their names (earthquake, flood, ...).
enum {
    DISASTER_PLAGUE = 8,
};

// What a disaster did to a faction, for its turn report (0x84 bytes).
struct DisasterReport {
    DisasterReport(int disaster, int);  // 0x0043871F
    ~DisasterReport();                  // 0x00439F54
    int& soldiers_lost(int faction) { return m_soldiers_lost[faction]; }

    int m_disaster;
    int m_4;
    Array<Character*> m_characters;     // +0x08: the characters it struck
    int m_soldiers_lost[21];            // +0x14: by faction
    Array<Settlement*> m_settlements;   // +0x68: the settlements it struck
    int m_population_lost;              // +0x74
    Array<int> m_78;                    // +0x78
};

// Something about a place on the campaign map, at a date (not identified; 0x3c bytes).
class EventUnk437A36 {
public:
    EventUnk437A36(int type, TilePosition* position, Date date);  // 0x00437A36
    ~EventUnk437A36();                  // 0x00439CCA

    char m_pad0[0x3c];
};

// What lbl_012CED70 is (not identified): it keeps EventUnk437A36s.
class EventsUnk12CED70 {
public:
    void unk_0043900D();                // 0x0043900D: at the start of each round
    void unk_004392F9(const EventUnk437A36& event);  // 0x004392F9: adds it, unless the campaign's settings say not to
};

extern "C" {
extern EventsUnk12CED70 lbl_012CED70;
}

#endif

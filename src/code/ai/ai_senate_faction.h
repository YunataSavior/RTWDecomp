#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\ai\\ai_senate_faction.h"
// The senate's missions (code\ai\ai_senate_faction.h; the original's path, which its __FILE__ gives). Its line
// numbers are the original's.
#ifndef AI_SENATE_FACTION_H
#define AI_SENATE_FACTION_H

#include "unsorted.h"

// A mission the senate gives (not identified).
class SenateMission {
public:
    ~SenateMission();
};

// A character's senate mission, if it has one (16 bytes; not identified).
struct SenateMissionSlot {
    SenateMissionSlot() : m_0(-1), m_c(0) {}
    ~SenateMissionSlot()
    {
        if (m_c)
#line 123
            CA_DELETE(m_c);
    }

    int m_0;
    Date m_date;
    SenateMission* m_c;
};

// What a senate office restricts, and what it gives (from descr_senate.txt's Restrictions and Senate_benefits).
enum {
    OFFICE_NOT_CONSECUTIVELY = 0,   // can't be held again straight after
    OFFICE_QUAESTOR_TENURE = 1,     // must have been quaestor
    OFFICE_AEDILE_TENURE = 2,
    OFFICE_PRAETOR_TENURE = 3,
    OFFICE_CONSUL_TENURE = 4,
    OFFICE_UNKNOWN_RESTRICTION = 5,
};

enum {
    OFFICE_NO_CENSOR_SUICIDE = 0,   // a censor's investigation won't end in suicide
    OFFICE_CENSOR_IMMUNITY = 1,     // no investigation by the censor
};

// The offices, by name.
enum {
    OFFICE_QUAESTOR = 0,
    OFFICE_AEDILE = 1,
    OFFICE_PRAETOR = 2,
    OFFICE_CONSUL = 3,
    OFFICE_CENSOR = 4,
    OFFICE_PONTIFEX_MAXIMUS = 5,
    OFFICE_UNKNOWN = 6,
};

// A senate office (0x3c bytes). descr_senate.txt's offices make one for each of its Quantity.
class SenateOffice {
public:
    SenateOffice(int title, int text, int rank, int type, int duration, int sittings, const Array<int>& restrictions,
                 const Array<int>& benefits);   // 0x005283ED

    int m_title;                        // +0x00: its name's text (0x106 + 2 * type; 0x51b when unknown)
    int m_text;                         // +0x04: the text after that (0x107 + 2 * type)
    int m_rank;                         // +0x08
    int m_c;                            // +0x0c: 0 when made (not identified)
    Date m_date;                        // +0x10: the date it was made
    int m_type;                         // +0x18
    int m_duration;                     // +0x1c: in turns
    int m_sittings;                     // +0x20: how many times a man can hold it; 0 for no limit
    Array<int> m_restrictions;          // +0x24
    Array<int> m_benefits;              // +0x30
};

// A mission the senate gives (code\senate_mission.cpp; SENATE_MISSION_IMPL in a save), and the root of the builders
// that make them; see senate_mission.h.
class SenateMissionImpl {
public:
    virtual void unk_00();
    virtual void* title();              // +0x04: its name's text
    virtual void unk_08(int* a, int* b);
    virtual int unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
    virtual void unk_20();
    virtual void unk_24();
    virtual void unk_28();
    virtual int unk_2c();
    virtual Faction* faction();         // +0x30: whom it was given to
    virtual int result();               // +0x34: MISSION_FAILED or MISSION_SUCCEEDED when it's over
    virtual bool unk_38(int a);
    virtual bool save(SaveStream& stream);  // +0x3c
    virtual SenateMissionImpl* build(); // +0x40: a builder's mission (a builder deletes itself)
    virtual ~SenateMissionImpl();       // +0x44: 0x005470E0
};

enum {
    MISSION_FAILED = 1,
    MISSION_SUCCEEDED = 3,
};

// What the senate makes of a faction this turn (0x1c bytes). Faction 3 is the senate's own.
struct SenateView {
    void reset();                       // 0x00528F7E
    void update_state(int tolerance);   // 0x00528FE0: from the flags below

    bool m_at_war_with_senate;          // +0x00: (for the families) at war with the senate
    bool m_at_war;                      // +0x01: at war with a family or the senate
    bool m_hostile;                     // +0x02: stance 400 with a family or the senate
    bool m_allied;                      // +0x03: allied with a family or the senate
    bool m_4;                           // +0x04: a diplomacy flag (0x005A0F3E) with a family or the senate
    bool m_5;                           // +0x05
    bool m_targeted;                    // +0x06: a family's AI has it in view (not identified)
    int m_threat;                       // +0x08: what the families' AIs make of it, summed (not identified)
    int m_c;                            // +0x0c: summed the same way
    int m_10;                           // +0x10: summed the same way
    int m_state;                        // +0x14: SENATE_VIEW_*
    int m_18;                           // +0x18: 0x10 when reset
};

enum {
    SENATE_VIEW_ENEMY_OF_SENATE = 0,    // a family at war with the senate
    SENATE_VIEW_ENEMY = 1,              // at war with Rome, or one the senate always counts as an enemy
    SENATE_VIEW_HOSTILE = 2,
    SENATE_VIEW_NEUTRAL = 3,
    SENATE_VIEW_FRIEND = 4,
    SENATE_VIEW_5 = 5,
};

// One of the three Roman families as the senate sees it (0x20 bytes).
struct SenateFamily {
    SenateFamily();                     // 0x005290BD
    void update_popularity();           // 0x005290FB
    void check_gift();                  // 0x00529593: a legion's first cohort, now and then

    Faction* m_faction;                 // +0x00
    int m_enemy;                        // +0x04: the neighbour it should fight (21 for none)
    int m_appreciation;                 // +0x08: added to its senate standing, down by 2 a turn
    int m_displeasure;                  // +0x0c: taken from its senate standing, down by 2 a turn
    int m_senate_standing;              // +0x10: 0 to 99
    int m_popularity;                   // +0x14: its popular standing, 0 to 99
    unsigned int m_gift_turn;           // +0x18: when the senate last gave it a unit
    bool m_senate_rising;               // +0x1c: its senate standing went up at its last change
    bool m_popularity_rising;           // +0x1d: its popularity went up at its last change
};

// An int kept scrambled in memory (not identified; perhaps against memory editors).
class ScrambledInt {
public:
    ScrambledInt(unsigned int scrambled) : m_0(0), m_scrambled(scrambled) {}
    int value();                        // 0x004247FE

    int m_0;
    unsigned int m_scrambled;           // +0x04
};

// The senate: an AI faction that judges the Roman families and gives them missions and offices.
class AISenateFaction {
public:
    void update();                                  // 0x0052A3D4: its turn
    void mission_finished(SenateMissionImpl* mission);  // 0x0052B168
    void start_appreciation();                      // 0x0052B22C
    void start_displeasure();                       // 0x0052B24B
    void unk_0052E033();                            // 0x0052E033: runs what's due today from 0x014D5F70's list
    void update_families();                         // 0x0052E178
    void unk_0052E7D2(FactionAI* family_ai);        // 0x0052E7D2: adds a family's AI's view of each faction
    void unk_0052E911();                            // 0x0052E911: sets the views' diplomacy flags
    int choose_enemy(Faction* family);              // 0x0052EC0D
    void unk_0052EDA6();                            // 0x0052EDA6
    void roman_strength(int* armies, int* fleets);  // 0x0052F91E: the senate's and the families' at peace with it
    void update_senate_standing(SenateFamily* family);  // 0x0052FBAF
    void unk_0052FFC8();                            // 0x0052FFC8: the offices
    void unk_00530A0D();                            // 0x00530A0D: the missions
    bool at_war_with_senate(int family) const { return m_views[family].m_at_war_with_senate; }

    char m_pad0[0x48];                  // its base class's (an AI faction's) and a second vtable at +0x44
    SenateView m_views[21];             // +0x48: one per faction
    SenateFamily m_families[3];         // +0x294: the Julii, the Brutii and the Scipii
    Faction* m_senate;                  // +0x2f4
    SenateFamily* m_player_family;      // +0x2f8: the human's family, or 0
    Date m_appreciation_until;          // +0x2fc
    Date m_displeasure_until;           // +0x304
    char m_pad30c[2];
    bool m_mission_failed;              // +0x30e: set when a mission fails
    bool m_30f;                         // +0x30f
    unsigned int m_missions_succeeded;  // +0x310
    int m_missions_failed;              // +0x314
    char m_pad318[0x328 - 0x318];
    unsigned int m_advice_turn[10];     // +0x328: when the advisor last spoke of each standing change
};

#endif

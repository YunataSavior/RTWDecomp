// The campaign (code\world.cpp).
#ifndef WORLD_H
#define WORLD_H

#include "unsorted.h"
#include "maths_random.h"
#include "calender.h"
#include "env/tracked_object.h"
#include "fort.h"
#include "faction_diplomacy.h"

class AncillaryDescription;

// Something of the campaign's (not identified).
class CampaignUnkCD5C {
public:
    void unk_005F2A2F();                // 0x005F2A2F: at the end of a round
};

// The campaign (probably code\campaign.cpp).
// The campaign's settings, set up from the options when it starts (0x00415D33).
struct CampaignSettings {
    float m_unit_scale;                 // +0x00: the unit size option (20, 40, 80 or 160) / 40, kept to 0.33-4
    char m_pad4[0x20 - 4];
};

// Something of the campaign's that names the senate's faction (not identified; 8 bytes).
struct CampaignSenateSlot {
    int m_faction;
    int m_4;
};

// A human player of the campaign (not identified beyond its faction).
struct CampaignHuman {
    int m_0;
    int m_faction;                      // +0x04
};

class AISenateFaction;

class Campaign {
public:
    int unk_004529FB(int faction);      // 0x004529FB: m_114
    Faction* faction(int index);        // 0x004548DD
    bool senate_at_war_with(Faction* faction);  // 0x0045425F: AISenateFaction::at_war_with, or false without a senate
    int difficulty(int faction);        // 0x004529E3: 0 easy to 3 very hard; always 1 for the AI
    float unit_scale();                 // 0x00452964
    CampaignSettings* settings();       // 0x00B4DC5F: lbl_018DC384's +0x29f8
    bool unk_00452977();                // 0x00452977: +0x238
    bool is_faction_alive(int faction); // 0x0045491C
    bool unk_00454978();                // 0x00454978: lbl_02921AE0 and the battle's +0x46 are set
    int human_faction(int index);       // 0x00457158: the index'th human faction, or 21
    bool is_prologue() const { return m_prologue; }
    int unk_2e4() const { return m_date.m_year; }
    int unk_4() const { return m_4; }
    bool unk_00458257(AncillaryDescription* ancillary);  // 0x00458257: whether someone has it (a guess)
    int unk_b8() const { return m_b8; }
    unsigned int num_factions() const { return m_num_factions; }
    Faction* faction_at(unsigned int index) const { return m_factions[index]; }
    Faction* unk_22c() const { return m_22c; }
    AISenateFaction* senate() const { return m_senate; }
    Faction* senate_faction();                          // 0x00445AE7: the senate's faction, or 0
    void mission_finished(SenateMissionImpl* mission);  // 0x0045420B: tells the senate, if there is one
    void unk_004531EF(const TriggerEvent& event);       // 0x004531EF: told of each event before the triggers
    void unk_00452674();                                // 0x00452674: before a load
    bool unk_00454CE1(SaveStream& stream);              // 0x00454CE1: loads the campaign
    bool unk_00455C12(SaveStream& stream);              // 0x00455C12: saves the campaign
    void unk_004562B8();                                // 0x004562B8: after a load
    bool unk_0045460A();                                // 0x0045460A
    Faction* unk_00454960(int faction);                 // 0x00454960: by its index
    Settlement* unk_004543F1(const char* name);         // 0x004543F1: a settlement by its name (a guess)
    bool last_britain_region(Region* region);           // 0x00457A73
    void unk_004537F2();                                // 0x004537F2: the end of a round
    void unk_004544AA();                                // 0x004544AA
    Settlement* rome();                                 // 0x004576C4
    void set_rome(Settlement* settlement);              // 0x004576DD
    Settlement* constantinople();                       // 0x004576F9
    void set_constantinople(Settlement* settlement);    // 0x00457712
    Settlement* get_rome() const { return m_rome; }
    void unk_00457C3C();                                // 0x00457C3C
    void unk_00452A2D();                                // 0x00452A2D
    void unk_00452A62();                                // 0x00452A62
    void unk_0045361A();                                // 0x0045361A: the start of a round
    void unk_00453B9B();                                // 0x00453B9B: the start of a faction's turn
    void unk_0045732F();                                // 0x0045732F
    void unk_00453942();                                // 0x00453942

    bool m_0;                           // +0x00: set once the campaign has started (0x00452AD6); unk_00452A2D and unk_00452A62 clear it
    char m_pad1[3];
    int m_4;                            // +0x04
    char m_pad8[0x10 - 8];
    CampaignHuman m_humans[21];         // +0x10
    unsigned int m_b8;                  // +0xb8: how many human factions there are (a guess; 1: the advisor's triggers run)
    char m_padbc[0xc0 - 0xbc];
    int m_difficulty[21];               // +0xc0: each faction's difficulty
    int m_114[21];                      // +0x114: something for each faction
    Faction* m_factions[21];            // +0x168: in turn order
    Faction* m_factions_by_id[21];      // +0x1bc
    unsigned int m_num_factions;        // +0x210
    char m_pad214[0x218 - 0x214];
    AISenateFaction* m_senate;          // +0x218: or 0
    char m_pad21c[0x22c - 0x21c];
    Faction* m_22c;                     // +0x22c: the faction whose turn it is (a guess)
    unsigned int m_230;                 // +0x230: its index in m_factions (a guess)
    char m_pad234[0x24c - 0x234];
    unsigned int m_turn;                // +0x24c: the turn number (a guess: things are timed by it)

    unsigned int turn() { return m_turn; }

    char m_pad250[0x254 - 0x250];
    Watcher m_rome_watcher;             // +0x254: m_rome's (a WatchedCharacter's code sets it)
    Settlement* m_rome;                 // +0x258: the settlement named Rome (Settlement::after_load)
    Watcher m_constantinople_watcher;   // +0x25c
    Settlement* m_constantinople;       // +0x260: the settlement named Constantinople
    char m_pad264[0x2b4 - 0x264];
    bool m_2b4;                         // +0x2b4: the families' popularity is kept to 65 or less (not identified)
    bool m_prologue;                    // +0x2b5 (a guess: it lowers brigand spawns and changes taxes and garrisons)
    bool unk_2b8() const { return m_2b8; }

    bool marian_reforms() const { return m_marian_reforms; }
    bool m_marian_reforms;              // +0x2b6: set by descr_strat.txt's marian_reforms_activated, or by unk_004537F2
    bool m_marian_reforms_disabled;     // +0x2b7: descr_strat.txt's marian_reforms_disabled
    bool m_2b8;                         // +0x2b8: characters can go over in a civil war (not identified)
    bool m_2b9;                         // +0x2b9: every revolt is to rebels (not identified)
    char m_pad2ba[0x2c4 - 0x2ba];
    bool m_2c4;                         // +0x2c4
    char m_pad2c5[0x2c8 - 0x2c5];
    Array<CampaignSenateSlot> m_2c8;    // +0x2c8: its first item's m_faction is the senate's
    char m_pad2d4[0x2e4 - 0x2d4];
    Date m_date;                        // +0x2e4 (its year is compared with 100; perhaps the turn number)
    char m_pad2ec[0x528 - 0x2ec];
    Random m_random;                    // +0x528
    DiplomacyMatrix m_diplomacy;        // +0x52c
    char m_padcd00[0xcd28 - 0xcd00];
    CampaignForts m_forts;              // +0xcd28
    char m_padcd34[0xcd4c - 0xcd34];
    char m_road_manager[0x10];          // +0xcd4c: a RoadManager (road.h's campaign_roads())
    CampaignUnkCD5C m_cd5c;             // +0xcd5c
};

extern "C" {
extern Campaign* lbl_018CD818;
extern Campaign* lbl_02921A30;      // the campaign again
inline Campaign* campaign()
{
    Campaign* campaign = lbl_02921A30;
    return campaign;
}
}

#endif

// The senate's missions (code\senate_mission.cpp) and the classes each mission type (code\sm_*.cpp) builds on.
#ifndef SENATE_MISSION_H
#define SENATE_MISSION_H

#include "unsorted.h"
#include "ai/ai_senate_faction.h"

class SenateReward;

// A mission as a save keeps it (0x1c bytes), and what a builder holds.
struct SenateMissionState {
    SenateMissionState();               // 0x00546E4F

    int m_senate;                       // +0x00: 0 for the campaign's senate
    int m_faction;                      // +0x04: 21 for none
    int m_reward;                       // +0x08: an index into 0x00FE7DF0's rewards (21 for none)
    int m_c;
    Date m_date;                        // +0x10
    int m_18;                           // +0x18: 10 when made
};

// What the senate asks a mission type's builder for (0x14 bytes).
struct SenateMissionRequest {
    SenateMissionRequest(AISenateFaction* senate, Faction* faction, int level, SenateReward* reward,
                         int unk);      // 0x00547F0E

    AISenateFaction* m_senate;
    Faction* m_faction;                 // +0x04: whom it's for
    int m_level;                        // +0x08: the family's senate standing * 0.07, rounded down (0 to 6)
    SenateReward* m_reward;             // +0x0c: its reward or penalty, or 0 to look it up
    int m_10;
};

// A mission type's rewards and penalties from descr_senate.txt (0x58 bytes): one code per standing level (attitude -3
// to 3) and difficulty (E, M and D).
class SenateRewardTable {
public:
    SenateRewardTable();                // 0x0054A9E9

    bool m_0;
    int m_codes[7][3];                  // +0x04
};

// A mission's builder: what a save keeps of a mission type until it builds the mission. Each type's builder (0x20
// bytes) keeps a SenateMissionState at +0x04.
class SenateMissionBuilder : public SenateMissionImpl {
};

// A mission the senate gives (0x20 bytes).
class SenateMissionBase : public SenateMissionImpl {
public:
    SenateMissionBase(AISenateFaction* senate, Faction* faction, SenateReward* reward, int unk);  // 0x005468D5
    SenateMissionBase(const SenateMissionState& state);    // 0x00546818
    ~SenateMissionBase();               // 0x0054693A

    virtual bool save(SaveStream& stream);  // 0x00546AC1

    virtual void* text();               // +0x48: what the senate asks
    virtual void* target_name();        // +0x4c: whom it's about

    AISenateFaction* m_senate;          // +0x04
    Faction* m_faction;                 // +0x08
    SenateReward* m_reward;             // +0x0c
    int m_result;                       // +0x10: 0 while it's open
    Date m_date;                        // +0x14: the date it was given
    int m_1c;
};

// What the human answers to a mission the senate offers (its second vtable, at +0x20).
class SenateMissionAnswer {
public:
    virtual void accepted() = 0;
    virtual void declined() = 0;
    virtual void answer_08() = 0;
    virtual void answer_0c() = 0;
    virtual ~SenateMissionAnswer() {}
};

// A mission the human is offered and answers (0x24 bytes).
class SenateMissionOffer : public SenateMissionBase, public SenateMissionAnswer {
public:
    SenateMissionOffer(AISenateFaction* senate, Faction* faction, SenateReward* reward, int unk);  // 0x00546EA0
    SenateMissionOffer(const SenateMissionState& state);   // 0x00546EE8

    virtual void accepted();            // 0x0054702D: the faction takes it on
    virtual void declined();            // 0x00547053: the faction lets it go
    virtual void answer_08();           // 0x00548E1F
    virtual void answer_0c();           // 0x00548E33

    void report_accepted(void* name, void* text);   // 0x00546C92
    void report_declined(void* name, void* text);   // 0x00546D83
};

extern "C" {
// Reads a mission's state from a save.
void fn_00546B90(SaveStream& stream, SenateMissionState& state);
// The reward or penalty a descr_senate.txt code (0 to 7) gives the faction; &lbl_012D75B4 for none.
SenateReward* fn_0054610E(int code, Faction* faction);
extern SenateReward lbl_012D75B4;
}

// Whether a reward is the "none" one.
inline bool is_no_reward(SenateReward* reward)
{
    return reward == &lbl_012D75B4;
}

inline bool is_reward(SenateReward* reward)
{
    return !is_no_reward(reward);
}

#endif

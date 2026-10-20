// The senate's missions (code\senate_mission.cpp) and the classes each mission type (code\sm_*.cpp) builds on.
#ifndef SENATE_MISSION_H
#define SENATE_MISSION_H

#include "unsorted.h"
#include "ai/ai_senate_faction.h"

// Where a reward shows on the map, for its message: a reward that makes a unit sets it to the unit's place.
typedef TilePosition* RewardPlace;

// A reward or penalty the senate gives at a mission's end (descr_senate.txt's codes pick them; see choose_reward).
// Each is a single object, kept by its name in rewards(), and a save keeps its index in s_rewards.
class SenateReward {
public:
    SenateReward(const char* name);     // 0x005460BB

    virtual bool is_benefit() = 0;      // +0x00: a reward, not a penalty
    virtual void* title() = 0;          // +0x04: its name's text
    // +0x08: what a penalty does to the faction, and its message's text (0 when it does nothing).
    virtual UniString* punish(AISenateFaction* senate, Faction* faction, UniString& text);  // 0x005460F4
    // +0x0c: what a reward does for the faction, and its message's text (0 when it does nothing).
    virtual UniString* reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place);
                                        // 0x00546101
    virtual SaveStream& save(SaveStream& stream) = 0;   // +0x10: its index
};

// The rewards and penalties, by index (as a save keeps them).
enum {
    REWARD_OUTLAW = 0,
    REWARD_CENSOR_INVESTIGATES_FAMILY = 1,
    REWARD_CENSOR_INVESTIGATES_FACTION = 2,
    REWARD_QUAESTOR_CORRUPTION = 3,
    REWARD_QUAESTOR_FINANCIAL = 4,
    REWARD_QUAESTOR_ACCOUNTING = 5,
    REWARD_SENATE_DISPLEASURE = 6,
    REWARD_SECRET_INFORMATION_EXPOSURE = 7,
    REWARD_OFFICES_WITHHELD = 8,
    REWARD_MAJOR_EXOTIC_UNIT = 9,
    REWARD_TWO_MINOR_EXOTIC_UNITS = 10,
    REWARD_TWO_BEST_BUILDABLE_UNITS = 11,
    REWARD_ONE_BEST_BUILDABLE_UNIT = 12,
    REWARD_ONE_MINOR_EXOTIC_UNIT = 13,
    REWARD_ONE_BUILDABLE_UNIT = 14,
    REWARD_MAJOR_CASH = 15,
    REWARD_MODERATE_CASH = 16,
    REWARD_MINOR_CASH = 17,
    REWARD_PROMISE_OF_APPOINTMENT = 18,
    REWARD_SENATE_GRATITUDE = 19,
    REWARD_THROW_GAMES = 20,
    REWARD_NONE = 21,                   // null_payback
    REWARD_ONE_TRIARII = 22,
    REWARD_TWO_TRIARII = 23,
    NUM_REWARDS = 24,
};

extern SenateReward* const s_rewards[NUM_REWARDS];  // 0x00FE7DF0

// The reward or penalty a descr_senate.txt code (0 to 7) gives the faction.
SenateReward* choose_reward(int code, Faction* faction);  // 0x0054610E

class NoReward;
extern NoReward s_null_payback;         // 0x012D75B4: the "none" reward (s_rewards[REWARD_NONE])

// Whether a reward is the "none" one.
inline bool is_no_reward(SenateReward* reward)
{
    return (void*)reward == (void*)&s_null_payback;
}

inline bool is_reward(SenateReward* reward)
{
    return !is_no_reward(reward);
}

// A mission as a save keeps it (0x1c bytes), and what a builder holds.
struct SenateMissionState {
    SenateMissionState();               // 0x00546E4F

    int m_senate;                       // +0x00: 0 for the campaign's senate
    int m_faction;                      // +0x04: 21 for none
    int m_reward;                       // +0x08: an index into s_rewards (REWARD_NONE for none)
    int m_result;                       // +0x0c
    Date m_date;                        // +0x10
    int m_duration;                     // +0x18: 10 when made
};

// Reads a mission's state from a save.
SaveStream& load_mission_state(SaveStream& stream, SenateMissionState& state);  // 0x00546B90

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
    bool load(ScriptReader& reader);    // 0x00546541: a mission's lines, from its Duration to its End

    unsigned char m_duration;           // +0x00: Duration, in turns
    int m_codes[7][3];                  // +0x04
};

// A mission's builder: what a save keeps of a mission type until it builds the mission. Each type's builder (0x20
// bytes) keeps a SenateMissionState at +0x04.
class SenateMissionBuilder : public SenateMissionImpl {
};

// What a mission's message shows (fn_00CB639F).
struct SenateMissionInfo {
    SenateMissionInfo(void* title, int kind, void* target, void* text, int duration, void* payback_kind, void* payback,
                      void* answer)
        : m_title(title), m_kind(kind), m_target(target), m_text(text), m_duration(duration),
          m_payback_kind(payback_kind), m_payback(payback), m_answer(answer)
    {
    }

    void* m_title;                      // +0x00: the mission's name
    int m_kind;                         // +0x04: 0x3b
    void* m_target;                     // +0x08: whom it's about
    void* m_text;                       // +0x0c: what the senate asks
    int m_duration;                     // +0x10
    void* m_payback_kind;               // +0x14: "reward" or "penalty" (texts 0xf2 and 0xdf)
    void* m_payback;                    // +0x18: the reward's or penalty's name
    void* m_answer;                     // +0x1c: an offer's SenateMissionAnswer, or 0
};

// A mission the senate gives (0x20 bytes).
class SenateMissionBase : public SenateMissionImpl {
public:
    SenateMissionBase(AISenateFaction* senate, Faction* faction, SenateReward* reward, int duration);  // 0x005468D5
    SenateMissionBase(const SenateMissionState& state);    // 0x00546818
    ~SenateMissionBase();               // 0x0054693A

    virtual void unk_00();              // 0x0054696B: tells the faction of it
    virtual int turns_left();           // 0x00546A3C: its duration less the turns since it was given
    virtual bool reward_title(void** title);    // 0x00546A67
    virtual bool unk_18();              // 0x00546A98
    virtual void unk_24();              // 0x00546954: adds it to the faction's missions
    virtual void unk_28();              // 0x00546A33
    virtual Faction* faction();         // 0x00546AA3
    virtual int result();               // 0x00546AB2
    virtual bool save(SaveStream& stream);  // 0x00546AC1

    virtual void* text() = 0;           // +0x48: what the senate asks
    virtual void* target_name() = 0;    // +0x4c: whom it's about

    AISenateFaction* senate() { return m_senate; }      // 0x00546E22
    SenateReward* payback() { return m_reward; }        // 0x00546E31
    int duration() { return m_duration; }               // 0x00546E40
    void succeed() { m_result = MISSION_SUCCEEDED; }    // 0x00546E0F

    AISenateFaction* m_senate;          // +0x04
    Faction* m_faction;                 // +0x08
    SenateReward* m_reward;             // +0x0c
    int m_result;                       // +0x10: 0 while it's open
    Date m_date;                        // +0x14: the date it was given
    int m_duration;                     // +0x1c: in turns
};

// What the human answers to a mission the senate offers (its second vtable, at +0x20).
class SenateMissionAnswer {
public:
    virtual void accepted() = 0;
    virtual void declined() = 0;
    virtual void* accept_text() = 0;
    virtual void* decline_text() = 0;
    virtual ~SenateMissionAnswer() {}
};

// A mission the human is offered and answers (0x24 bytes).
class SenateMissionOffer : public SenateMissionBase, public SenateMissionAnswer {
public:
    SenateMissionOffer(AISenateFaction* senate, Faction* faction, SenateReward* reward, int duration);  // 0x00546EA0
    SenateMissionOffer(const SenateMissionState& state);   // 0x00546EE8

    virtual void unk_00();              // 0x00546F45: tells the faction of it, with the answer to give
    virtual void unk_24();              // 0x00546F27: adds it to the faction's offers
    virtual void accepted();            // 0x0054702D: the faction takes it on
    virtual void declined();            // 0x00547053: the faction lets it go
    virtual void* accept_text();        // 0x00548E1F
    virtual void* decline_text();       // 0x00548E33

    // It succeeded: the reward is given, and the message tells of it.
    void report_accepted(void* name, void* text);   // 0x00546C92
    // It's over with neither outcome (result 2).
    void report_cancelled(void* name, void* text);  // 0x00546D31
    // It failed: the penalty is given, and the message tells of it.
    void report_declined(void* name, void* text);   // 0x00546D83
};

#endif

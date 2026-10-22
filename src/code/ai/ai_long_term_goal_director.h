// A faction's AI's long-term goals: how it stands towards each faction, and which it should go to war with
// (code\ai\ai_long_term_goal_director.cpp; the classes' names are guesses from the unit's).
#ifndef AI_LONG_TERM_GOAL_DIRECTOR_H
#define AI_LONG_TERM_GOAL_DIRECTOR_H

#include "unsorted.h"
#include "ai_analyser.h"
#include "ai_faction_base.h"
#include "../faction.h"

// The AI's goal towards one faction (0x2c bytes).
struct AIFactionGoal {
    AIFactionGoal();                    // 0x00514B88

    int m_stance;                       // +0x00: update_stances' verdict, 0 to 5 (5 the worst enemy); 1 at first
    int m_stance_priority;              // +0x04: its priority, 300 to 900
    int m_war_goal;                     // +0x08: choose_war_targets' verdict, 0 to 4; 5 at first, for none
    int m_war_priority;                 // +0x0c: its priority, 100 to 900 (AISenateFaction adds it to m_threat)
    bool m_at_war;                      // +0x10: they're at war (set by update_stances)
    bool m_11;                          // +0x11: set for a war that isn't the other's shadowing faction's (not
                                        //   identified)
    bool m_12;                          // +0x12
    bool m_13;                          // +0x13
    bool m_14;                          // +0x14: set like m_11, for choose_war_targets' first case
    char m_pad15[3];
    int m_18;                           // +0x18: 1 to 10 with each verdict (not identified)
    int m_1c;                           // +0x1c: AISenateFaction adds it to its view's m_c
    int m_20;                           // +0x20
    int m_24;                           // +0x24
    int m_28;                           // +0x28: AISenateFaction adds it to its view's m_10
};

// One goal for each faction; its implicit constructor is 0x00518418.
struct AIFactionGoalArray {
    AIFactionGoal m_goals[21];
};

struct AIFactionGoals : public AIFactionGoalArray {
    AIFactionGoals() {}
};

// A region that a region can reach by sea, and how far it is (a region's +0x5c; 0x34 bytes).
struct AIRegionTarget {
    int m_region;
    char m_pad4[4];
    float m_distance;                   // +0x08: perhaps in turns (30 is near)
    char m_padc[0x34 - 0xc];
};

// A set of factions that starts empty.
struct AIFactionSet : public FactionSet {
    AIFactionSet() { m_bits = 0; }
};

// The long-term goals (vtable 0x00FE746C; 0x3cc bytes).
class AILongTermGoalDirectorBase {
public:
    AILongTermGoalDirectorBase(FactionAI* ai);  // 0x00514BFB

    virtual void unk_00();              // 0x00514E1C
    virtual void unk_04();              // 0x00514D34
    virtual void unk_08();              // 0x00514D48
    virtual void update_stances();      // 0x00515070
    virtual void choose_war_targets();  // 0x0051578F
    virtual void unk_14();              // 0x0051626F
    virtual void unk_18();              // 0x00514F43
    virtual void unk_1c() = 0;
    virtual void unk_20();              // 0x005183EF
    virtual void unk_24();              // 0x00514CA3
    virtual void unk_28() = 0;
    virtual void unk_2c();              // 0x00514CAC

    bool ready_for_war() const { return m_18; }
    int target_region() const { return m_1c; }

    bool stronger_armies(int faction);              // 0x00516568: its armies are stronger than the faction's
    bool stronger_than_its_enemies(int faction);    // 0x005165D2: than the faction's and its other enemies' together
    bool weaker_than_allies(int faction);           // 0x005166B2: the faction's armies are weaker than its and its
                                                    //   allies' that border the faction and aren't allied to it
    bool is_top_1c(int faction);                    // 0x005167E3: the faction's goal has the highest m_1c
    bool shares_superfaction(int faction);          // 0x005168BD
    bool is_fellow_family(int faction);             // 0x00516968: shares its superfaction, while neither has the
                                                    //   senate's unk_004542E9
    bool protected_by_family(int faction);          // 0x005169D8: the faction has bit 3 of the relation's flags
                                                    //   with its superfaction or one of that's other factions
    void choose_sea_target();                       // 0x00516ACE: sets m_1c and m_20
    int group_value(AIRegionGroup* group);          // 0x00516CE5: 100 for each region, or -1
    int target_score(const AIRegionTarget& target, int from);  // 0x00516D1D: -1 when it can't be a target
    bool borders_fellow_family(int region);         // 0x00517006: a neighbouring region's owner is_fellow_family
    bool borders_region(int region);                // 0x0051715B: the region or one of its neighbours borders its
                                                    //   own

    FactionAI* m_ai;                    // +0x04
    Faction* m_faction;                 // +0x08
    AIFactionSet m_c;                   // +0x0c: update_stances doesn't set m_11 for these factions
    int m_10;                           // +0x10
    float m_14;                         // +0x14
    bool m_18;                          // +0x18: free to start a war (choose_war_targets)
    char m_pad19[3];
    int m_1c;                           // +0x1c: the region it means to invade by sea, -1 for none
    int m_20;                           // +0x20: the region it would sail from, -1 at first
    int m_24;                           // +0x24
    AIFactionGoals m_goals;             // +0x28: one for each faction
    int m_3c4;                          // +0x3c4: 3 at first
    int m_3c8;                          // +0x3c8
};

// The faction AI's long-term goals (vtable 0x00FE749C).
class AILongTermGoalDirector : public AILongTermGoalDirectorBase {
public:
    AILongTermGoalDirector(FactionAI* ai);  // 0x00517247

    virtual void unk_00();              // 0x00517A9D
    virtual void unk_04();              // 0x00517269
    virtual void unk_08();              // 0x005179C3
    virtual void unk_14();              // 0x00517951
    virtual void unk_1c();              // 0x00517E6C
    virtual void unk_28();              // 0x00517CE2
};

#endif

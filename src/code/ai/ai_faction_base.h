// A faction's AI (code\ai\ai_faction_base.cpp).
#ifndef AI_FACTION_BASE_H
#define AI_FACTION_BASE_H

#include "unsorted.h"

class AILongTermGoalDirector;
class AIResourceAllocationManager;
class AncillaryDescription;

class FactionAIUnk34 {
public:
    void unk_0050625A(int, int);        // 0x0050625A
};

// A faction's AI's figures for one faction in its global strategy director (not identified; 0x20 bytes).
struct FactionAIUnk38Entry {
    char m_pad0[0x1e];
    bool m_1e;                          // +0x1e
    char m_pad1f[1];
};

// A faction's AI's global strategy director, probably (code\ai\ai_global_strategy_director.cpp; built at
// 0x0050AFCA; 0x370 bytes).
class FactionAIUnk38 {
public:
    bool unk_1e(int faction) const { return m_entries[faction].m_1e; }

    char m_pad0[0xc];
    FactionAIUnk38Entry m_entries[21];  // +0x0c: one per faction
    char m_pad2ac[0x370 - 0x2ac];
};

// What a faction AI's +0x0c is (not identified).
struct FactionAIUnkC {
    char m_pad0[8];
    int m_8;                            // +0x08: AILongTermGoalDirectorBase::choose_war_targets wants it non-zero
};

// A faction's AI (not identified; 0x44 bytes).
class FactionAI {
public:
    FactionAI(Faction* faction);        // 0x004FD904
    Faction* faction() const { return m_faction; }
    int faction_id() const { return m_faction_id; }
    FactionAIUnkC* unk_c() const { return m_c; }
    AILongTermGoalDirector* unk_20() const { return m_20; }
    AIResourceAllocationManager* resources() const { return m_resources; }
    FactionAIUnk34* unk_34() const { return m_34; }
    FactionAIUnk38* unk_38() const { return m_38; }

    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();

    Faction* m_faction;                 // +0x04
    int m_faction_id;                   // +0x08
    FactionAIUnkC* m_c;                 // +0x0c
    char m_pad10[0x20 - 0x10];
    AILongTermGoalDirector* m_20;       // +0x20: built at 0x00517247
    char m_pad24[0x2c - 0x24];
    AIResourceAllocationManager* m_resources;   // +0x2c
    char m_pad30[0x34 - 0x30];
    FactionAIUnk34* m_34;               // +0x34
    FactionAIUnk38* m_38;               // +0x38
    char m_pad3c[0x44 - 0x3c];
};

#endif

// A faction's AI (code\ai\ai_faction_base.cpp).
#ifndef AI_FACTION_BASE_H
#define AI_FACTION_BASE_H

#include "unsorted.h"

class AncillaryDescription;

class FactionAIUnk34 {
public:
    void unk_0050625A(int, int);        // 0x0050625A
};

// A faction's AI's figures for one faction (not identified; 0x2c bytes).
struct FactionAIUnk20Entry {
    char m_pad0[8];
    int m_8;                            // +0x08
    int m_c;                            // +0x0c: AISenateFaction adds it to its view's m_threat
    bool m_10;                          // +0x10
    char m_pad11[0x1c - 0x11];
    int m_1c;                           // +0x1c: and this to m_c
    char m_pad20[0x28 - 0x20];
    int m_28;                           // +0x28: and this to m_10
};

// A faction's AI's war targets, probably (not identified; built at 0x00517247, on 0x00514BFB; 0x3cc bytes).
class FactionAIUnk20 {
public:
    bool unk_18() const { return m_18; }
    int unk_1c() const { return m_1c; }

    char m_pad0[0x18];
    bool m_18;                          // +0x18: m_1c is set
    char m_pad19[3];
    int m_1c;                           // +0x1c: a region's index
    char m_pad20[0x28 - 0x20];
    FactionAIUnk20Entry m_entries[21];  // +0x28: one per faction
    char m_pad3c4[0x3cc - 0x3c4];
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

// A faction's AI (not identified; 0x44 bytes).
class FactionAI {
public:
    FactionAI(Faction* faction);        // 0x004FD904
    FactionAIUnk20* unk_20() const { return m_20; }
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

    char m_pad4[0x20 - 4];
    FactionAIUnk20* m_20;               // +0x20
    char m_pad24[0x34 - 0x24];
    FactionAIUnk34* m_34;               // +0x34
    FactionAIUnk38* m_38;               // +0x38
    char m_pad3c[0x44 - 0x3c];
};

#endif

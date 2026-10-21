// What settlements and forts have in common (code\settlement_base.cpp).
#ifndef SETTLEMENT_BASE_H
#define SETTLEMENT_BASE_H

#include "unsorted.h"
#include "env/tracked_object.h"
#include "siege.h"

// What settlements and forts have in common (not identified).
class FogOfWarEntry;

class SettlementBase : public StrategyObject {
public:
    int faction() const { return m_faction; }
    Army* garrison() const { return unk_40(); }
    Army* unk_40() const { return m_garrison.m_object; }
    bool besieged() const { return m_84; }
    Siege* siege(int index) const { return m_sieges[index].m_object; }
    bool has_plague() const { return m_plague; }

    void catch_plague();                                    // 0x0049ABD1: and so do the characters on its tile
    void plague_tick();                                     // 0x0049ACAA: the fourth tick ends the plague
    void unk_0049A464();                                    // 0x0049A464: does nothing
    unsigned int num_characters(bool governor);             // 0x0049BE82: the characters in it (the governor too, when asked)
    bool has_room(bool count);                              // 0x0049A49F: no garrison, or one with room for COUNT more units

    virtual FogOfWarEntry* unk_74();                        // +0x74: its fog of war entry, new
    virtual void unk_78();
    virtual void unk_7c();
    virtual void unk_80();
    virtual void unk_84(Army* army);                        // +0x84
    virtual void unk_88();
    virtual int region_id();                                // +0x8c: 0x005BE77E
    virtual void unk_90();
    virtual void unk_94();
    virtual void unk_98();
    virtual Faction* unk_9c();                              // +0x9c: the same function as owner()
    virtual Faction* owner();                               // +0xa0: 0x005BE753
    virtual void unk_a4();
    virtual void change_owner(CharacterRecord* captor, Faction* faction, int reason);  // +0xa8: 0x005B7F5E
    virtual void unk_ac(Character* leader);                 // +0xac: 0x005B9745, after a revolt
    virtual void unk_b0();
    virtual Region* unk_b4();

    char m_pad24[0x3c - 0x24];
    WatchedPointer<Army> m_garrison;    // +0x3c (a guess)
    WatchedPointer<Siege> m_sieges[8];  // +0x44 (a guess; 8 fill the space before m_84)
    unsigned char m_84;                 // +0x84: how many of m_sieges there are; no riots while it's non-zero
    char m_pad85[0x8c - 0x85];
    int m_8c;                           // +0x8c: public order slot 17 is twice this
    char m_pad90[0x98 - 0x90];
    int m_98;                           // +0x98: after a trade (reason 5), the previous owner; otherwise 21
    char m_pad9c[0xa0 - 0x9c];
    int m_faction;                      // +0xa0
    char m_pada4[0xac - 0xa4];
    unsigned int m_plague_ticks;        // +0xac
    bool m_plague;                      // +0xb0
    bool m_b1;                          // +0xb1: cleared by a change of owner
    char m_padb2[0xb4 - 0xb2];
};

#endif

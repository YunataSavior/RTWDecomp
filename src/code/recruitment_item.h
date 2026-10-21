// A settlement's recruitment (code\recruitment_item.cpp).
#ifndef RECRUITMENT_ITEM_H
#define RECRUITMENT_ITEM_H

#include "unsorted.h"

// A unit in a settlement's recruitment queue, or one it could recruit (code\recruitment_item.cpp; 0x20 bytes).
struct RecruitmentItem {
    RecruitmentItem();                  // 0x005AE26D
    void reset();                       // 0x005AE2D6
    void unk_005AE4F0(Settlement* settlement, UnitUnk50* type, unsigned char experience, unsigned char armour,
                      unsigned char weapon, int percent);  // 0x005AE4F0
    void unk_005AE6C3(Settlement* settlement, UnitUnk50* type, unsigned char experience, unsigned char armour,
                      unsigned char weapon, int percent);  // 0x005AE6C3: the same, with m_0d set (for a ship; a guess)

    UnitUnk50* m_type;                  // +0x00
    char m_pad4[4];
    Settlement* m_settlement;           // +0x08
    unsigned char m_0c;                 // +0x0c
    unsigned char m_0d;                 // +0x0d: set by unk_005AE6C3
    unsigned char m_experience;         // +0x0e
    unsigned char m_armour;             // +0x0f
    unsigned char m_weapon;             // +0x10
    unsigned char m_11;                 // +0x11
    unsigned char m_12;                 // +0x12
    unsigned char m_13;                 // +0x13
    unsigned char m_turns;              // +0x14: the turns it takes to train
    unsigned short m_cost;              // +0x16
    unsigned short m_soldiers;          // +0x18
    unsigned short m_turn;              // +0x1a: the turn it was made on
    unsigned char m_1c;                 // +0x1c
};

// The units a settlement can recruit (a revolt's too), up to 256 (not identified).
struct RecruitmentItems {
    RecruitmentItems()
    {
        m_data = (RecruitmentItem*)m_items;
        m_count = 0;
        m_2008 = 0;
    }
    ~RecruitmentItems()
    {
        while (m_count != 0)
            m_count--;
    }
    unsigned int count();               // 0x005AF300
    RecruitmentItem* add();             // 0x005AF319

    char m_items[256 * sizeof(RecruitmentItem)];  // made as they're added
    RecruitmentItem* m_data;            // +0x2000
    unsigned int m_count;               // +0x2004
    int m_2008;
};

#endif

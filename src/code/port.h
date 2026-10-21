// Ports (code\port.cpp).
#ifndef PORT_H
#define PORT_H

#include "unsorted.h"

// What a settlement's +0x184 is (not identified; its port?).
class SettlementUnk184 : public StrategyObject {
public:
    Army* unk_48() const { return m_48; }
    int unk_6c() const { return m_6c; }
    void unk_0049E140(Faction* faction, int);  // 0x0049E140: hands it to the settlement's new owner
    int unk_0049E906();                 // 0x0049E906: public order slot 18
    bool unk_0049E979();                // 0x0049E979
    SettlementUnk184* unk_0049EF76();   // 0x0049EF76: another one (its +0x78), or 0
    Faction* unk_0049E4B4();            // 0x0049E4B4: its settlement's owner
    Army* unk_0049E4D1(Unit* unit);     // 0x0049E4D1: puts the unit in its fleet (virtual +0x98), when it has one
    int unk_3c() const { return m_3c; }

    char m_pad24[0x3c - 0x24];
    int m_3c;                           // +0x3c: a port the senate can ask to blockade has it above 0
    char m_pad40[0x48 - 0x40];
    Army* m_48;                         // +0x48: an army in it, or 0
    char m_pad4c[0x6c - 0x4c];
    int m_6c;                           // +0x6c
};

// A region's port (probably code\port.cpp).
struct PortUnkA98 {
    int unk_78() const { return m_78; }

    char m_pad0[0x78];
    int m_78;                           // +0x78
};

struct Port {
    int unk_184() const { return m_184; }
    PortUnkA98* unk_a98() const { return m_a98; }

    char m_pad0[0x184];
    int m_184;                          // +0x184
    char m_pad188[0xa98 - 0x188];
    PortUnkA98* m_a98;                  // +0xa98
};

#endif

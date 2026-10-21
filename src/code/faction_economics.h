// A faction's money (code\faction_economics.cpp).
#ifndef FACTION_ECONOMICS_H
#define FACTION_ECONOMICS_H

#include "unsorted.h"

// A faction's finances (not identified beyond its update).
// A faction's figures for one turn in its finances (not identified; 0x58 bytes).
class FinanceRecord {
public:
    void unk_004D7EAA();                // 0x004D7EAA
    int total_income();                 // 0x004D7DA7: its ten incomes
    int total_costs();                  // 0x004D7DE1: its twelve costs
    int unk_004D7E1C();                 // 0x004D7E1C: its first four incomes
    int unk_004D7E74();                 // 0x004D7E74: its first two costs

    int m_income[10];                   // +0x00
    int m_costs[12];                    // +0x28
};

// A faction's last turns' finances (0x378 bytes).
struct FinanceHistory {
    FinanceRecord& current() { return m_records[m_current]; }

    FinanceRecord m_records[10];
    int m_370;                          // +0x370
    unsigned int m_current;             // +0x374
};

// A faction's money (0x38c bytes).
class FactionFinance {
public:
    void end_turn();                    // 0x004D8197: adds the turn's income, and the AI's cash
    void unk_004D8396(bool, bool);      // 0x004D8396: at the start of its turn
    void earn(int amount, int income);  // 0x004D8248: to the treasury, and to the turn's record under an income
    void spend(int amount, int cost);   // 0x004D829C: from the treasury, and to the turn's record under a cost
    // The turn's income less its costs.
    int net()
    {
        // NONMATCHING - its locals' stack slots are in another order
        FinanceRecord& record = m_history.current();
        int net = record.total_income() - record.total_costs();
        return net;
    }

    Faction* m_faction;                 // +0x00
    int m_treasury;                     // +0x04
    int m_income;                       // +0x08: the turn's
    FinanceHistory m_history;           // +0x0c
    char m_pad384[0x38c - 0x384];
};

#endif

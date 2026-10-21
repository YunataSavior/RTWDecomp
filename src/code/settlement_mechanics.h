// A settlement's growth and public order (code\settlement_mechanics.cpp).
#ifndef SETTLEMENT_MECHANICS_H
#define SETTLEMENT_MECHANICS_H

#include "unsorted.h"

class BuildingEffects;

// The growth factors (SettlementDD8::compute_growth); each point is 0.5% a turn.
enum {
    GROWTH_FERTILITY = 0,               // the region's, from descr_regions.txt
    GROWTH_FARMING = 1,                 // the buildings' farming_level and the governor's Farming, 0-25
    GROWTH_GRAIN = 2,                   // 3 for each of the region's grain resources, and then the governor's
                                        //   GrainTrading
    GROWTH_HEALTH = 3,                  // the buildings' population_health_bonus and the governor's Health, 0-25
    GROWTH_BONUS = 4,                   // the buildings' population_growth_bonus
    GROWTH_LOW_TAXES = 5,               // 1 at low taxes
    GROWTH_SLAVES = 7,                  // the region's slaves resources, and then the governor's SlaveTrading
    GROWTH_SQUALOR = 8,                 // 0-50
    GROWTH_PLAGUE = 9,                  // 20 while it has plague
    GROWTH_HIGH_TAXES = 10,             // 1 at high taxes, 2 at very high
};

// The resources of descr_sm_resources.txt, by their IDs (0x00E3CAFB gives them).
enum {
    RESOURCE_GRAIN = 4,
    RESOURCE_SLAVES = 9,
};

// A settlement's figures for a turn (0x1c0 bytes): its growth factors and its public order, each as the lines of the
// breakdown the player sees.

struct SettlementStats {
    int growth(int factor) const { return m_growth[factor]; }
    const int& growth_ref(int factor) const { return m_growth[factor]; }
    void set_growth(int factor, int value) { m_growth[factor] = value; }
    Settlement* settlement() const { return m_settlement; }
    int order(int slot) const { return m_order[slot]; }
    void apply_growth(int faction, int level);  // 0x005C7DED

    Settlement* m_settlement;           // +0x00
    int m_growth[16];                   // +0x04: factors 0-7 add to growth, 8-15 take from it (GROWTH_*)
    unsigned int m_population;          // +0x44
    int m_order[25];                    // +0x48: in units of 5%; slots 0-9 add to order, 10-24 take from it
    unsigned int m_order_percent;       // +0xac
    char m_padb0[0xf8 - 0xb0];
    RegionReligions m_religion;         // +0xf8: the region's (BI), copied in each time
    char m_pad120[0x1c0 - 0x120];
};

// What the religion slots of the public order come from (BI; 0x30 bytes).
struct SettlementReligion {
    SettlementReligion();               // 0x005CF3B4

    int m_values[10];
    int m_28;                           // +0x28
    int m_2c;                           // +0x2c
};

// What a settlement's +0xdd8 is: its turn's figures and how they're worked out (code\settlement_mechanics.cpp).
class SettlementDD8 {
public:
    void start_turn();                  // 0x005C865A
    unsigned int unk_005C8AD7();        // 0x005C8AD7: the population a riot can't take it below
    void unk_005C8CE1(int);             // 0x005C8CE1
    void unk_005C8EFF(unsigned int population);  // 0x005C8EFF: population lost (one of three causes; not identified)
    void unk_005C8F38(unsigned int population);  // 0x005C8F38: population lost to a riot or a revolt
    void unk_005C8F71(unsigned int population);  // 0x005C8F71: population lost (not identified)
    void update(bool mark_owner);       // 0x005C8729: works out m_current
    void set_faction(Faction* faction); // 0x005C8A72
    void compute_growth(SettlementStats* stats, BuildingEffects* effects);  // 0x005CC583
    void compute_order(SettlementStats* stats, BuildingEffects* effects, int* religion, int* religion_effects);  // 0x005CC9D7
    void unk_005CDB6A(SettlementStats* stats, BuildingEffects* effects);  // 0x005CDB6A: fills +0xb0 (income? a guess)
    void unk_005CE3E8(SettlementStats* stats, BuildingEffects* effects);  // 0x005CE3E8: BI's religion
    LandmarkManager* landmarks();       // 0x00B4E286

    Faction* m_faction;                 // the owner
    Settlement* m_settlement;           // +0x04
    int m_8;                            // +0x08: from the region's descr_regions.txt entry (not identified)
    int m_rebel_chance;                 // +0x0c: the chance of the region's rebel type in descr_rebel_factions.txt
    char m_pad10[0x54 - 0x10];
    SettlementStats m_last;             // +0x54: last turn's
    SettlementStats m_current;          // +0x214
    bool m_3d4;                         // +0x3d4: cleared by each update
};

#endif

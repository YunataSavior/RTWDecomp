// A settlement's building effects (code\capabilities.cpp).
#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include "unsorted.h"

// Building effects, by the index of their names in export_descr_buildings.txt's list (0xFEC790).
enum {
    EFFECT_POPULATION_GROWTH_BONUS = 0,
    EFFECT_POPULATION_HEALTH_BONUS = 2,
    EFFECT_FARMING_LEVEL = 8,
    EFFECT_ARMOUR = 14,
    EFFECT_STAGE_GAMES = 15,
    EFFECT_STAGE_RACES = 16,
    EFFECT_WEAPON_SIMPLE = 18,          // then missile, bladed, siege and other
    EFFECT_RECRUITS_EXP_BONUS = 25,
    EFFECT_HAPPINESS_BONUS = 26,
    EFFECT_LAW_BONUS = 27,
    NUM_EFFECTS = 36,
};

// One building effect's value: a level, and a bonus that counts only when the level isn't negative (a guess).
struct BuildingEffect {
    BuildingEffect() {}
    short total() const;                // 0x005D4BE6

    short m_level;
    short m_bonus;                      // +0x02
};

// A unit type a settlement's buildings can train, and the experience they give it.
struct BuildingRecruit {
    int m_unit;                         // its index in export_descr_unit.txt
    BuildingEffect m_experience;        // +0x04
};

// A settlement's building effects (0x308 bytes).
class BuildingEffects {
public:
    BuildingEffects();                  // 0x005D451D
    short unk_005D48AE(int effect);     // 0x005D48AE
    BuildingEffect effect(int effect);  // 0x005D4973: none (0, 0) for an ID from 36 up
    unsigned char unk_005D48F2(int effect);  // 0x005D48F2: the effect's total, from 0 to 255
    unsigned int num_recruits();        // 0x005D4ADF
    BuildingRecruit* recruit(BuildingRecruit* out, unsigned int index);  // 0x005D4AF1: none (unit count, -1) past the end

    char m_pad0[0xb4];
    BuildingRecruit m_recruits[0x40];   // +0xb4: the units its buildings can train
    unsigned int m_num_recruits;        // +0x2b4
    char m_pad2b8[0x308 - 0x2b8];
};

#endif

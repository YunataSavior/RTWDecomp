#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\settlement_mechanics.cpp"
// (The original's path, as its neighbours' __FILE__s give it; this file has none of its own.)
static char THIS_FILE[] = __FILE__;

// A settlement's figures for each turn: its growth and its public order.
#include "unsorted.h"
#include "army.h"
#include "capabilities.h"
#include "faction.h"
#include "rebel_faction_db.h"
#include "romans_utils.h"
#include "settlement.h"
#include "settlement_mechanics.h"
#include "strategy_map.h"
#include "world.h"
#include "world_map.h"

#include "ai/ai_cheat_manager.h"
#include "character.h"
#include "character_record.h"
#include "landmark.h"

// The wonders the public order reads (landmark types).
enum {
    LANDMARK_PYRAMIDS = 1,
    LANDMARK_STATUE_OF_ZEUS = 2,
};

inline int min_int(const int& a, const int& b)
{
    return a < b ? a : b;
}

inline int max_int(const int& a, const int& b)
{
    return a < b ? b : a;
}

inline int clamp_int(const int& value, const int& low, const int& high)
{
    return min_int(max_int(value, low), high);
}

inline unsigned int min_uint(const unsigned int& a, const unsigned int& b)
{
    return a < b ? a : b;
}

inline unsigned int max_uint(const unsigned int& a, const unsigned int& b)
{
    return a < b ? b : a;
}

inline void clamp_uint(unsigned int& value, const unsigned int& low, const unsigned int& high)
{
    value = min_uint(max_uint(value, low), high);
}

// Growth is factors 0-5 less factors 8-10, kept to -200..25 points, and then the AI's bonus for the difficulty; a point
// is 0.5% of the population. Factors 6 and 7 (slaves) aren't counted. The plague's share (factor 9) is kept as the
// settlement's m_plague_deaths, and the population is kept between the level's m_8 and three times its m_c.
void SettlementStats::apply_growth(int faction, int level)
{
    int growth = 0;
    growth += growth_ref(GROWTH_FERTILITY);
    growth += growth_ref(GROWTH_FARMING);
    growth += growth_ref(GROWTH_GRAIN);
    growth += growth_ref(GROWTH_HEALTH);
    growth += growth_ref(GROWTH_BONUS);
    growth += growth_ref(GROWTH_LOW_TAXES);
    growth -= growth_ref(GROWTH_SQUALOR);
    growth -= growth_ref(GROWTH_PLAGUE);
    growth -= growth_ref(GROWTH_HIGH_TAXES);
    growth = clamp_int(growth, -200, 25);
    growth += AICheatManager::ai_population_growth(faction);
    settlement()->m_plague_deaths = float_to_int(m_population * (0.005f * growth_ref(GROWTH_PLAGUE)));
    m_population += float_to_int(m_population * (0.005f * growth));
    SettlementLevel* limits = &lbl_012DAFC8[level];
    clamp_uint(m_population, (unsigned int&)limits->m_8, limits->m_c * 3);
}

// At the start of the owner's turn: reads the region's rebel chance, works the figures out again, and keeps them as
// last turn's (which the riot and revolt checks read).
void SettlementDD8::start_turn()
{
    m_8 = fn_008F99BC(&m_settlement->region()->m_colour);
    Region* region = m_settlement->region();
    RebelType* type = fn_00903C7E(region->m_rebel_type);
    m_rebel_chance = type->m_chance;
    if (m_rebel_chance < 0)
        m_rebel_chance = 0;
    update(true);
    m_last = m_current;
}

// Works the settlement's figures out again into m_current: its growth, its public order and the rest, from its
// buildings' effects (and in BI its religions).
void SettlementDD8::update(bool mark_owner)
{
    // NONMATCHING - stack slot order differs, and the original reserves 8 more bytes of stack
    Campaign* campaign = lbl_02921A30;
    int unk_4 = campaign->unk_4();
    if (unk_4 == 0) {
        BuildingEffects effects;
        m_settlement->unk_005BCA50(&effects);
        int religion[10];
        int zero = 0;
        unsigned int i;
        for (i = 0; i < 10; i++)
            religion[i] = zero;
        int religion_effects[10];
        int zero2 = 0;
        unsigned int j;
        for (j = 0; j < 10; j++)
            religion_effects[j] = zero2;
        m_settlement->unk_005B5849(religion, religion_effects);
        m_3d4 = false;
        if (religions_enabled()) {
            Region* region = m_settlement->region();
            m_current.m_religion = region->m_religions;
            unk_005CE3E8(&m_current, &effects);
        }
        compute_growth(&m_current, &effects);
        compute_order(&m_current, &effects, m_current.m_religion.m_values, religion_effects);
        unk_005CDB6A(&m_current, &effects);
        if (mark_owner) {
            Faction* owner = m_settlement->owner();
            owner->m_4e4 = true;
        }
        Unk02921A38* unk = lbl_02921A38;
        unk->unk_00C098DC(m_settlement);
    }
}

// Works out the growth factors (GROWTH_*). Squalor is the governor's Squalor, plus a point for each 1500 people and
// for each 1500 above twice the level's first population, plus a point for each 350 above its last.
void SettlementDD8::compute_growth(SettlementStats* stats, BuildingEffects* effects)
{
    // NONMATCHING - the original keeps the governor and some attribute values in more temporaries, and its stack slot
    // order differs
    int grain = 0;
    int slaves = 0;
    int health = 0;
    int squalor = 0;
    int farming = 0;
    if (m_settlement->region_id() != -1) {
        Region* region = m_settlement->region();
        grain = region->unk_00485627(RESOURCE_GRAIN, 0) * 3;
        slaves = region->unk_00485627(RESOURCE_SLAVES, 0);
    }
    Character* governor = m_settlement->governor();
    if (governor) {
        CharacterRecord* record = governor->m_record;
        if (grain > 0)
            grain += record->attribute_value(ATTR_GRAIN_TRADING);
        if (slaves > 0)
            slaves += record->attribute_value(ATTR_SLAVE_TRADING);
        health += record->attribute_value(ATTR_HEALTH);
        squalor += record->attribute_value(ATTR_SQUALOR);
        farming += record->attribute_value(ATTR_FARMING);
    }
    stats->set_growth(GROWTH_FERTILITY, m_8);
    stats->set_growth(GROWTH_FARMING, clamp_int(effects->unk_005D48AE(EFFECT_FARMING_LEVEL) + farming, 0, 25));
    stats->set_growth(GROWTH_GRAIN, grain);
    stats->set_growth(GROWTH_HEALTH, clamp_int(effects->unk_005D48AE(EFFECT_POPULATION_HEALTH_BONUS) + health, 0, 25));
    stats->set_growth(GROWTH_BONUS, effects->unk_005D48AE(EFFECT_POPULATION_GROWTH_BONUS));
    stats->set_growth(GROWTH_LOW_TAXES, m_settlement->tax_level() == 0);
    stats->set_growth(GROWTH_SLAVES, slaves);
    int crowding = stats->m_population - lbl_012DAFC8[m_settlement->level()].m_0 * 2;
    int overcrowding = stats->m_population - lbl_012DAFC8[m_settlement->level()].m_c;
    stats->set_growth(GROWTH_SQUALOR,
                      clamp_int(squalor + (stats->m_population + (crowding > 0 ? crowding : 0)) / 1500 +
                                    (overcrowding > 0 ? overcrowding : 0) / 350,
                                0, 50));
    stats->set_growth(GROWTH_PLAGUE, m_settlement->has_plague() ? 20 : 0);
    stats->set_growth(GROWTH_HIGH_TAXES, m_settlement->tax_level() >= 2 ? m_settlement->tax_level() - 1 : 0);
}

// Works out the settlement's public order: the 25 lines of its breakdown (in units of 5%), and the percentage.
// Lines 0-9 add to it and lines 10-24 take from it.
void SettlementDD8::compute_order(SettlementStats* stats, BuildingEffects* effects, int* religion, int* religion_effects)
{
    // NONMATCHING - the original reads the order slots and the governor's attributes through inline accessors that
    // keep their index and the object in temporaries, and its stack slot order differs
    int faction = m_faction ? m_faction->id() : lbl_01181378;
    int growth = 0;
    int squalor_population = lbl_012DAFC8[m_settlement->m_level].m_0;
    unsigned int i;
    for (i = 0; i < 8; i++)
        growth += stats->m_growth[i];
    for (; i < 16; i++)
        growth -= stats->m_growth[i];

    // The garrison, against the population.
    int garrison = 0;
    if (m_settlement->m_garrison.m_object) {
        if (lbl_02921A30->m_prologue) {
            Army* army = m_settlement->m_garrison.m_object;
            int population = float_to_int(lbl_02921A30->unit_scale() * stats->m_population);
            garrison = army->garrison_soldiers() * 1600 / population * 2;
        } else {
            Army* army = m_settlement->m_garrison.m_object;
            int population = float_to_int(lbl_02921A30->unit_scale() * stats->m_population);
            garrison = army->garrison_soldiers() * 700 / population * 2;
        }
        garrison /= 5;
        garrison = clamp_int(garrison, 0, 16);
    } else {
        garrison = 0;
    }

    // The governor's attributes.
    int influence = 0;
    int health = 0;
    int law = 0;
    Character* governor = m_settlement->m_governor.m_character;
    if (governor) {
        CharacterRecord* record = governor->m_record;
        influence = record->influence();
        influence += record->attribute(ATTR_LOCAL_POPULARITY);
        law = record->attribute(ATTR_LAW);
        health = record->attribute(ATTR_HEALTH);
        influence = clamp_int(influence, 0, 8);
    } else {
        influence = 0;
        law = 0;
        health = 0;
    }

    int boom = 0;
    if (growth > AICheatManager::boom_threshold(faction))
        boom = growth - AICheatManager::boom_threshold(faction);
    else
        boom = 0;

    int wonder = 0;
    if (landmarks()->find(LANDMARK_STATUE_OF_ZEUS)) {
        if (landmarks()->find(LANDMARK_STATUE_OF_ZEUS)->faction() == m_faction)
            wonder = 2;
    }

    stats->m_order[0] = garrison;
    stats->m_order[1] = clamp_int(law + effects->unk_005D48AE(EFFECT_LAW_BONUS), 0, 25);
    stats->m_order[2] = clamp_int(effects->unk_005D48AE(EFFECT_HAPPINESS_BONUS), 0, 25);
    stats->m_order[3] = influence;
    stats->m_order[4] = m_settlement->m_tax_level ? 0 : 6;
    stats->m_order[5] = m_faction->m_triumph_order;
    stats->m_order[6] = wonder + m_faction->m_wonder_order;
    stats->m_order[7] = boom;
    stats->m_order[8] = m_settlement->unk_005BC80B() / 5;
    stats->m_order[9] = clamp_int(effects->unk_005D48AE(EFFECT_POPULATION_HEALTH_BONUS) + health, 0, 25);

    // Squalor, from the population.
    int over_twice = stats->m_population - squalor_population * 2;
    int over = stats->m_population - lbl_012DAFC8[m_settlement->m_level].m_c;
    int squalor_extra = (over > 0 ? over : 0) / 350;
    int squalor = (stats->m_population + (over_twice > 0 ? over_twice : 0)) / 1500 + squalor_extra;
    if (governor)
        squalor += governor->m_record->attribute(ATTR_SQUALOR);

    int distance = m_settlement->unk_005BCDC4() - 10;

    // The share of the population of the owner's culture (and, with the Pyramids, of the Egyptian culture).
    int share = 0;
    int culture = m_faction->culture();
    int other_culture = 7;
    if (landmarks()->find(LANDMARK_PYRAMIDS)) {
        if (landmarks()->find(LANDMARK_PYRAMIDS)->faction() == m_faction) {
            int* egyptian = fn_008E695D(String("egyptian"));
            if (egyptian)
                other_culture = *egyptian;
        }
    }
    for (i = 0; i < lbl_02921A30->num_factions(); i++) {
        Faction* other = lbl_02921A30->m_factions[i];
        if (other) {
            int other_faction_culture = other->culture();
            if (other_faction_culture == culture || other_faction_culture == other_culture)
                share += m_settlement->m_population_share[other->id()];
        }
    }
    if (m_settlement->m_creator == faction)
        share = 100;
    else if (lbl_02921A30->m_prologue)
        share = (share + 100) >> 1;

    // Unrest: the countdown after a change of owner, foreign spies, the governor, and the region's rebels.
    int unrest = m_settlement->m_owner_turns;
    if (unrest >= 6)
        unrest = 0;
    else
        unrest = 6 - unrest;
    static Array<Character*> s_spies;
    s_spies.clear();
    lbl_02921A1C->foreign_spies(m_settlement, &s_spies);
    int spies = 0;
    for (i = 0; i < s_spies.size(); i++) {
        int subterfuge = s_spies[i]->m_record->subterfuge();
        if (subterfuge > 0)
            spies += subterfuge;
    }
    if (spies > 10)
        spies = 10;
    unrest += spies;
    if (governor)
        unrest += governor->m_record->attribute(ATTR_UNREST);
    if (m_settlement->region_id() != -1 && m_faction) {
        Region* region = m_settlement->region();
        if (m_faction->id() != region->creator_faction())
            unrest += m_rebel_chance;
    }

    int unused_16 = 0;
    int unused_19 = 0;
    SettlementReligion religion_order;
    if (lbl_02921A30->unk_4() == 0 && religions_enabled()) {
        fn_005F4FEB(religion, religion_effects, m_settlement->unk_005B5975(), effects, m_settlement->unk_005B5AEB(),
                    m_settlement->unk_005B5B56(&religion_order));
    }

    squalor = clamp_int(squalor, 0, 20);
    stats->m_order[10] = squalor;
    distance = clamp_int(distance, 0, 80);
    stats->m_order[11] = distance / 5;
    stats->m_order[12] = (100 - share) / 10;
    stats->m_order[13] = m_settlement->m_garrison.m_object ? 0 : 3;
    stats->m_order[14] = (m_settlement->m_tax_level >= 2 ? m_settlement->m_tax_level - 1 : 0) * 4;
    unrest = clamp_int(unrest, 0, 16);
    stats->m_order[15] = unrest;
    stats->m_order[16] = unused_16;
    stats->m_order[17] = m_settlement->m_8c * 2;
    stats->m_order[18] = m_settlement->m_184 && m_settlement->m_184->unk_0049E979()
        ? m_settlement->m_184->unk_0049E906()
        : 0;
    unused_19 = clamp_int(unused_19, 0, 16);
    stats->m_order[19] = unused_19;
    if (religions_enabled()) {
        stats->m_order[22] = religion_order.m_values[0] / 5;
        stats->m_order[23] = religion_order.m_values[1] / 5;
        stats->m_order[24] = religion_order.m_values[2] / 5;
        stats->m_order[20] = religion_order.m_28 / 5;
        stats->m_order[21] = religion_order.m_2c / 5;
    }

    int order = 0;
    unsigned int slot;
    for (slot = 0; slot < 10; slot++)
        order += stats->m_order[slot];
    for (; slot < 25; slot++)
        order -= stats->m_order[slot];
    order += AICheatManager::ai_public_order(m_settlement->owner_id());
    order = clamp_int(order, -20, 80);
    stats->m_order_percent = order * 5 + 100;
    if (m_settlement->m_level == 0 && stats->m_order_percent <= 50)
        stats->m_order_percent = 51;
}

SettlementReligion::SettlementReligion()
{
    int zero = 0;
    for (unsigned int i = 0; i < 10; i++)
        m_values[i] = zero;
    m_28 = 0;
    m_2c = 0;
}

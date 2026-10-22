#include "ai_cheat_manager.h"

#include "unsorted.h"
#include "ai_analyser.h"
#include "../faction.h"
#include "../world.h"
#include "../world_map.h"
#include "map_constants.h"

// Each faction's standing towards the first human faction, rebuilt by update().
struct FactionCheatFlags {
    FactionCheatFlags()
    {
        human = false;
        at_war_and_bordering = false;
        allied = false;
        bordering = false;
        strongest_neighbour = false;
        small_ai = false;
    }

    bool human;
    bool at_war_and_bordering;
    bool allied;
    bool bordering;
    bool strongest_neighbour;
    bool small_ai;
};

struct FactionCheatFlagTable {
    FactionCheatFlags flags[21];
};

static FactionCheatFlagTable s_flags;        // 0x012D2E70
static int s_player_war_turns;               // 0x012D2F94: turns the player has been at war in a row

static int s_ai_population_growth;           // 0x012D2F7C: -1 / 0 / +2 / +4
static int s_ai_public_order;                // 0x012D2F80: -2 / 0 / +4 / +6
static int s_ai_extra_count;                 // 0x012D2F84: 0 / 0 / 1 / 2
static int s_ai_value_against;               // 0x012D2F88: 0 / 0 / 10 / 30
static int s_ai_turn_cash;                   // 0x012D2F8C: 0 / 200 / 400 / 700
static bool s_war_filters;                   // 0x012D2F90: 0 / 1 / 1 / 1
static bool s_target_player_bonus;           // 0x012D2F91: 0 / 0 / 0 / 1
static bool s_unknown_12d2f92;               // 0x012D2F92: 0 / 1 / 1 / 1
bool AICheatManager::s_player_objective_bonus;  // 0x012D2F98: 0 / 0 / 1 / 1

static int s_boom_threshold = 4;             // 0x010DA994: 4 / 6 / 8 / 10
static bool s_unknown_10da998 = true;        // 0x010DA998: 0 / 0 / 1 / 1
static float s_unused_scale = 1.0f;          // 0x010DA99C: 1.0 / 1.0 / 1.2 / 1.4
static float s_ai_brigand_scale = 1.0f;      // 0x010DA9A0: 1.0 / 1.0 / 0.88 / 0.8
static float s_player_brigand_scale = 1.0f;  // 0x010DA9A4: 1.0 / 1.0 / 1.12 / 1.2
static int s_war_turns = 10;                 // 0x010DA9A8: 20 / 10 / 4 / 2
static float s_player_tax_scale = 1.0f;      // 0x010DA9AC: 1.2 / 1.0 / 0.92 / 0.85
static float s_player_farm_scale = 1.0f;     // 0x010DA9B0: 1.2 / 1.0 / 0.92 / 0.85
static float s_ai_income_scale = 1.0f;       // 0x010DA9B4: 1.0, or the catch-up factor on medium and hard
static float s_player_target_value = 1.0f;   // 0x010DA9B8: 1.0 / 1.8 / 2.2 / 3.0

inline int max_int(const int& a, const int& b)
{
    return a < b ? b : a;
}

inline float max_float(const float& a, const float& b)
{
    return a < b ? b : a;
}

inline float min_float(const float& a, const float& b)
{
    return a < b ? a : b;
}

inline float clamp_float(const float& value, const float& low, const float& high)
{
    return min_float(max_float(value, low), high);
}

inline int ai_summary_value(int faction)
{
    return lbl_012D0F78[faction].m_value;
}

inline bool is_strongest_neighbour(int faction, int other)
{
    return lbl_012D1552[faction * 21 + other].m_strongest_neighbour;
}

void AICheatManager::reset_war_turns()
{
    s_player_war_turns = 0;
}

void AICheatManager::update_war_turns()
{
    int player = lbl_018CD818->human_faction(0);
    if (lbl_018CD818->is_faction_alive(player)) {
        if (lbl_018CD818->m_diplomacy.is_at_war(player))
            s_player_war_turns++;
        else
            s_player_war_turns = 0;
    }
}

void AICheatManager::update()
{
    int human = lbl_018CD818->human_faction(0);
    int neighbour_mask = 0;
    Faction* player_faction = 0;
    int level = 1;
    unsigned int num_regions = 7;
    bool unknown = false;
    if (lbl_018CD818->is_faction_alive(human)) {
        player_faction = lbl_018CD818->faction(human);
        neighbour_mask = player_faction->neighbours().bits();
        level = lbl_018CD818->difficulty(human);
        num_regions = player_faction->num_regions();
        if (fn_004E335E(human))
            unknown = true;
    }

    switch (level) {
    case 0:
        s_ai_population_growth = -1;
        s_ai_public_order = -2;
        s_ai_extra_count = 0;
        s_ai_value_against = 0;
        s_ai_turn_cash = 0;
        s_boom_threshold = 4;
        s_war_filters = false;
        s_target_player_bonus = false;
        s_unknown_10da998 = false;
        s_unknown_12d2f92 = false;
        s_unused_scale = 1.0f;
        s_ai_brigand_scale = 1.0f;
        s_player_brigand_scale = 1.0f;
        s_player_tax_scale = 1.2f;
        s_player_farm_scale = 1.2f;
        s_war_turns = 20;
        s_ai_income_scale = 1.0f;
        s_player_target_value = 1.0f;
        s_player_objective_bonus = false;
        break;
    case 2: {
        s_ai_population_growth = 2;
        s_ai_public_order = 4;
        s_ai_extra_count = 1;
        s_ai_value_against = 10;
        s_ai_turn_cash = 400;
        s_boom_threshold = 8;
        s_war_filters = true;
        s_target_player_bonus = false;
        s_unknown_10da998 = true;
        s_unknown_12d2f92 = true;
        s_unused_scale = 1.2f;
        s_ai_brigand_scale = 0.88f;
        s_player_brigand_scale = 1.12f;
        s_player_tax_scale = 0.92f;
        s_player_farm_scale = 0.92f;
        s_war_turns = 4;
        int map_regions = lbl_018CD80C->num_regions();
        if (map_regions > 30) {
            if (num_regions > 10)
                s_ai_income_scale = 1.0f + clamp_float(0.1f * (float)(num_regions - 10) / 20.0f, 0.0f, 0.2f);
            else
                s_ai_income_scale = 1.0f;
        } else {
            s_ai_income_scale = 1.0f;
        }
        if (human != 21 && ai_summary_value(human) > 0)
            s_player_target_value = 2.2f;
        else
            s_player_target_value = 1.0f;
        s_player_objective_bonus = true;
        break;
    }
    case 3:
        s_ai_population_growth = 4;
        s_ai_public_order = 6;
        s_ai_extra_count = 2;
        s_ai_value_against = 30;
        s_ai_turn_cash = 700;
        s_boom_threshold = 10;
        s_war_filters = true;
        s_target_player_bonus = true;
        s_unknown_10da998 = true;
        s_unknown_12d2f92 = true;
        s_unused_scale = 1.4f;
        s_ai_brigand_scale = 0.8f;
        s_player_brigand_scale = 1.2f;
        s_player_tax_scale = 0.85f;
        s_player_farm_scale = 0.85f;
        s_war_turns = 2;
        s_ai_income_scale = 1.0f;
        if (human != 21 && ai_summary_value(human) > 0)
            s_player_target_value = 3.0f;
        else
            s_player_target_value = 1.5f;
        s_player_objective_bonus = true;
        break;
    default: {
        s_ai_population_growth = 0;
        s_ai_public_order = 0;
        s_ai_extra_count = 0;
        s_ai_value_against = 0;
        s_ai_turn_cash = 200;
        s_boom_threshold = 6;
        s_war_filters = true;
        s_target_player_bonus = false;
        s_unknown_10da998 = false;
        s_unknown_12d2f92 = true;
        s_unused_scale = 1.0f;
        s_ai_brigand_scale = 1.0f;
        s_player_brigand_scale = 1.0f;
        s_player_tax_scale = 1.0f;
        s_player_farm_scale = 1.0f;
        s_war_turns = 10;
        int map_regions = lbl_018CD80C->num_regions();
        if (map_regions > 30) {
            if (num_regions > 10)
                s_ai_income_scale = 1.0f + clamp_float(0.1f * (float)(num_regions - 10) / 20.0f, 0.0f, 0.2f);
            else
                s_ai_income_scale = 1.0f;
        } else {
            s_ai_income_scale = 1.0f;
        }
        if (human != 21 && ai_summary_value(human) > 0)
            s_player_target_value = 1.8f;
        else
            s_player_target_value = 1.0f;
        s_player_objective_bonus = false;
        break;
    }
    }

    if (human != 21) {
        for (unsigned int i = 0; i < 21; ++i) {
            int faction = i;
            if (lbl_018CD818->is_faction_alive(faction)) {
                Faction* record = lbl_018CD818->faction(faction);
                int stance = lbl_018CD818->m_diplomacy.stance(faction, human);
                s_flags.flags[i].human = record->m_control == 1;
                s_flags.flags[i].at_war_and_bordering = stance == 600 && (bool)(neighbour_mask & (1 << faction));
                s_flags.flags[i].allied = stance == 0;
                s_flags.flags[i].bordering = (neighbour_mask & (1 << faction)) != 0;
                if (!unknown && human != faction && !record->unk_004584F1() && is_strongest_neighbour(human, faction))
                    s_flags.flags[i].strongest_neighbour = true;
                else
                    s_flags.flags[i].strongest_neighbour = false;
                if (!s_flags.flags[i].human && lbl_018CD818->unk_2e4() < 100 && record->unk_00445B2B()
                    && !record->m_description->has_shadowing() && record->num_regions() <= num_regions / 2)
                    s_flags.flags[i].small_ai = true;
                else
                    s_flags.flags[i].small_ai = false;
            }
        }
    } else {
        for (unsigned int i = 0; i < 21; ++i) {
            int faction = i;
            if (lbl_018CD818->is_faction_alive(faction)) {
                s_flags.flags[i].human = false;
                s_flags.flags[i].at_war_and_bordering = false;
                s_flags.flags[i].allied = false;
                s_flags.flags[i].bordering = false;
                s_flags.flags[i].strongest_neighbour = false;
                s_flags.flags[i].small_ai = false;
            }
        }
    }
}

int AICheatManager::ai_population_growth(int faction)
{
    if (!s_flags.flags[faction].human && faction != lbl_01181378)
        return s_ai_population_growth;
    return 0;
}

int AICheatManager::ai_public_order(int faction)
{
    if (!s_flags.flags[faction].human && faction != lbl_01181378)
        return s_ai_public_order;
    return 0;
}

int AICheatManager::ai_extra_count(int faction)
{
    if (!s_flags.flags[faction].human)
        return s_ai_extra_count;
    return 0;
}

int AICheatManager::ai_value_against(int faction, int other)
{
    if (!s_flags.flags[faction].human && !s_flags.flags[other].human)
        return s_ai_value_against;
    return 0;
}

int AICheatManager::ai_turn_cash(int faction)
{
    if (!s_flags.flags[faction].human) {
        Faction* record = lbl_018CD818->faction(faction);
        float scale = 0.0f;
        int num_regions = record->num_regions();
        if (num_regions > 5)
            scale = 0.1f * (float)max_int(5 - (num_regions - 5), 0) + 0.5f;
        else
            scale = 1.0f;
        if (record->has_subfactions()) {
            int treasury = record->m_finance.m_treasury;
            if (treasury < 0)
                return s_ai_turn_cash + 3000 - treasury;
            return s_ai_turn_cash + 3000;
        }
        if (s_flags.flags[faction].small_ai)
            return s_ai_turn_cash + 1000;
        if (faction != lbl_01181378)
            return float_to_int((float)s_ai_turn_cash * scale);
    }
    return 0;
}

int AICheatManager::boom_threshold(int faction)
{
    if (s_flags.flags[faction].human)
        return s_boom_threshold;
    return 4;
}

float AICheatManager::unused_scale()
{
    return s_unused_scale;
}

float AICheatManager::brigand_spawn_scale(int faction)
{
    if (lbl_018CD818->is_prologue())
        return 0.33f;
    if (s_flags.flags[faction].human)
        return s_player_brigand_scale;
    return s_ai_brigand_scale;
}

float AICheatManager::tax_income_scale(int faction)
{
    if (s_flags.flags[faction].human)
        return s_player_tax_scale;
    return 1.1f * s_ai_income_scale;
}

float AICheatManager::farm_income_scale(int faction)
{
    if (s_flags.flags[faction].human)
        return s_player_farm_scale;
    return 1.1f * s_ai_income_scale;
}

bool AICheatManager::may_target(int faction, int target)
{
    if (target == lbl_01181378)
        return true;
    if (s_war_filters && !s_flags.flags[faction].allied && s_flags.flags[target].at_war_and_bordering)
        return false;
    if (s_flags.flags[target].at_war_and_bordering && ai_summary_value(target) <= 0)
        return false;
    if (s_flags.flags[target].strongest_neighbour && !s_flags.flags[faction].allied)
        return false;
    if (s_flags.flags[faction].strongest_neighbour && !s_flags.flags[target].human)
        return false;
    if (lbl_018CD818->faction(target)->unk_004584F1() && !s_flags.flags[faction].human
        && !s_flags.flags[target].human) {
        if (lbl_018CD818->faction(faction)->num_regions() < 7 || s_flags.flags[target].small_ai)
            return false;
    }
    return true;
}

bool AICheatManager::punish_long_war(int faction, int player)
{
    if (s_flags.flags[player].human && is_strongest_neighbour(player, faction) && s_player_war_turns > s_war_turns)
        return true;
    return false;
}

bool AICheatManager::flag_war(int faction, int enemy)
{
    if (s_war_filters && !s_flags.flags[faction].allied && s_flags.flags[enemy].at_war_and_bordering)
        return true;
    if (s_flags.flags[enemy].at_war_and_bordering && ai_summary_value(enemy) <= 0)
        return false;
    return false;
}

bool AICheatManager::target_player_bonus(int faction, int target)
{
    if (s_target_player_bonus && !s_flags.flags[faction].allied && s_flags.flags[target].human)
        return true;
    return false;
}

bool AICheatManager::unknown_10da998(int faction)
{
    if (lbl_018CD818->faction(faction)->unk_4f4())
        return true;
    return s_unknown_10da998;
}

bool AICheatManager::unknown_12d2f92()
{
    return s_unknown_12d2f92;
}

bool AICheatManager::unknown_4fcdb8()
{
    return true;
}

bool AICheatManager::small_ai_chance(int faction, int other)
{
    if (lbl_018CD818->faction(faction)->unk_004584F1() && !s_flags.flags[faction].human
        && !s_flags.flags[other].human && s_flags.flags[faction].small_ai && lbl_018CD818->m_random.percent() < 60)
        return true;
    return false;
}

bool AICheatManager::not_small_ai(int faction)
{
    return !s_flags.flags[faction].small_ai;
}

float AICheatManager::player_target_value(int faction)
{
    if (s_flags.flags[faction].human)
        return s_player_target_value;
    return lbl_00FE4970;
}

bool AICheatManager::save(SaveStream& stream)
{
    stream.write(&s_player_war_turns, 4);
    return true;
}

bool AICheatManager::load(SaveStream& stream)
{
    int war_turns;
    stream.read(war_turns);
    s_player_war_turns = war_turns;
    return true;
}

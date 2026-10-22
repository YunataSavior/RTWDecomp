#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\settlement.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A settlement of the campaign map.
#include "unsorted.h"
#include "ai/ai_analyser.h"
#include "army.h"
#include "battle.h"
#include "building.h"
#include "building_db.h"
#include "calender.h"
#include "campaign_triggers.h"
#include "capabilities.h"
#include "event_manager.h"
#include "faction.h"
#include "faction_db.h"
#include "fog_of_war.h"
#include "horde.h"
#include "port.h"
#include "rebel_faction_db.h"
#include "recruitment_item.h"
#include "romans_utils.h"
#include "settlement.h"
#include "settlement_mechanics.h"
#include "settlement_triggers.h"
#include "siege.h"
#include "strategy_map.h"
#include "unit.h"
#include "unit_db.h"
#include "version_registry.h"
#include "world.h"
#include "world_map.h"

#include "ai/ai_cheat_manager.h"
#include "character.h"
#include "character_record.h"
#include "spy_actions.h"

// The kinds of revolt (what revolt_type gives).
enum {
    REVOLT_REEMERGE = 0,       // to a dead faction that most of the population belongs to
    REVOLT_REBELS = 1,
    REVOLT_SHADOW = 2,         // to the owner's shadowed_by or shadowing faction (BI's civil wars)
    REVOLT_SPAWN = 3,          // to the owner's spawns_on_revolt faction
    REVOLT_BRITAIN = 4,        // in a region with the britain hidden resource (BI)
    REVOLT_GLADIATORS_1 = 5,   // gladiators, by the level of the settlement's amphitheatre
    REVOLT_GLADIATORS_2 = 6,
    REVOLT_GLADIATORS_3 = 7,
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

inline short min_short(const short& a, const short& b)
{
    return a < b ? a : b;
}

inline short max_short(const short& a, const short& b)
{
    return a < b ? b : a;
}

inline short clamp_short(const short& value, const short& low, const short& high)
{
    return min_short(max_short(value, low), high);
}

// A settlement's fog of war entry (not identified; 0x30 bytes).
class SettlementFogEntry {
public:
    SettlementFogEntry(Settlement* settlement);  // 0x004946FB

    char m_pad0[0x30];
};

extern "C" int fn_008F6174(int faction);  // the faction description's +0x40's first value (not identified)

FogOfWarEntry* Settlement::unk_74()
{
#line 223
    return (FogOfWarEntry*)NEW SettlementFogEntry(this);
}

int Settlement::unk_005B1486()
{
    return fn_008F6174(m_owner->id());
}

int Settlement::owner_id()
{
    int id = m_owner->id();
    return id;
}

// What a riot or a revolt does: kills some of the population (but never below a floor), some of the garrison and some
// of the characters in the settlement, and passes the rest on to the buildings. The three flags say which of the
// population's losses it is (the third also takes the most of the range instead of rolling).
SettlementDamage Settlement::riot_damage(SettlementDamageParams* params, bool cause_1, bool cause_2, bool cause_3,
                                         bool skip_outside)
{
    // NONMATCHING - stack slot order differs, the original keeps the campaign and each range bound in temporaries, and
    // it reads an unset temporary before the third flag's case
    int amount = lbl_02921A30->m_random.range(params->m_0, params->m_4);
    if (amount > 0)
        m_dd8.unk_005C8CE1(amount);
    else
        amount = 0;
    int percent;
    if (cause_3)
        percent = params->m_population_max;
    else
        percent = lbl_02921A30->m_random.range(params->m_population_min, params->m_population_max);
    unsigned int killed = m_dd8.m_current.m_population * percent / 100;
    unsigned int most = m_dd8.m_current.m_population - m_dd8.unk_005C8AD7();
    if (killed > most)
        killed = most;
    if (cause_1)
        m_dd8.unk_005C8EFF(killed);
    else if (cause_2)
        m_dd8.unk_005C8F38(killed);
    else if (cause_3)
        m_dd8.unk_005C8F71(killed);

    int losses = 0;
    if (m_garrison.m_object) {
        Army* army = m_garrison.m_object;
        losses = army->kill_soldiers(lbl_02921A30->m_random.range(params->m_garrison_min, params->m_garrison_max));
    }
    SettlementDamage damage(this, amount, killed, losses);
    m_buildings.damage(params, &damage, skip_outside);

    // Each character in it (the governor too) is killed with the same chance.
    unsigned int count = num_characters(true);
    unsigned int chance = lbl_02921A30->m_random.range(params->m_character_min, params->m_character_max);
    unsigned int unused = 0;
    for (unsigned int i = 0; i < count; i++) {
        unsigned int roll = lbl_02921A30->m_random.percent();
        if (roll < chance) {
            Character* victim = character(i, true);
            if (victim) {
                victim->set_death_cause(2);
                victim->kill();
                if (victim->type() == 5)
                    victim->unk_00580C1E()->m_898.set_14(21);
                damage.add_character(victim);
            }
        }
    }
    if (damage.m_num_buildings > 0)
        m_owner->unk_004D3E6F(0);
    return damage;
}

// At the start of the settlement's turn: at 65% public order or less it riots, or, when it has been unhappy for long
// enough, it may revolt instead. It reads the order worked out last (and last turn's), not a new one.
void Settlement::check_riots(BuildingEffects* effects)
{
    // NONMATCHING - stack slot order differs, the original compares the garrison with a 0 kept on the stack, and this
    // copies riot_damage's result into damage where the original has it built there
    bool rioted = m_rioted;
    m_rioted = false;
    int owner = m_owner->id();
    if (m_owner->is_unk_a7() || !m_owner->no_subfactions())
        return;
    if (!AICheatManager::not_small_ai(owner))
        return;
    if (m_84 > 0)
        return;
    unsigned int order = m_dd8.m_current.m_order_percent;
    if (order > 65)
        return;

    // The chance of a revolt.
    int chance = 0;
    int type = revolt_type();
    if (m_turns_since_revolt < 3 || m_dd8.m_last.m_order_percent > 70 || m_owner_turns <= 1) {
        chance = 0;
    } else if (rioted) {
        Region* region = this->region();
        int neighbours = 0;
        int unused = 0;
        for (unsigned int i = 0; i < region->m_neighbours.size(); i++) {
            StrategyMap* map = lbl_02921A34;
            Region* other = map->region(region->m_neighbours[i].m_region);
            Faction* faction = other->m_owner;
            if (faction) {
                if (faction->is_unk_a7())
                    neighbours++;
                if (type == REVOLT_SHADOW) {
                    if (faction->description()->has_shadowed_by()
                        && m_owner->id() == faction->description()->shadowed_by()) {
                        neighbours++;
                    } else if (faction->description()->has_shadowing()
                               && m_owner->id() == faction->description()->shadowing()) {
                        neighbours++;
                    }
                }
            }
        }
        int base = 70 - order + neighbours * 5 + 10;
        if (order < 50 || m_garrison.m_object == 0) {
            chance = base + 30;
        } else {
            Army* army = m_garrison.m_object;
            unsigned int population = m_dd8.m_current.m_population;
            if (army->unk_00722430() > (int)(population >> 1))
                chance = 0;
            else
                chance = base;
        }
    }
    chance = clamp_int(chance, 0, 90);

    Faction* faction = m_owner;
    if (m_force_revolt || (chance != 0 && lbl_02921A30->m_random.percent() <= chance)) {
        // A revolt.
        m_turns_since_revolt = 0;
        m_force_revolt = false;
        Army* army = revolt(type, 0, 2, 21);
        if (faction->unk_004584F1())
            m_a7e++;
        fn_008B4820(this, faction, m_governor.m_character);
        lbl_012CED70.unk_004392F9(EventUnk437A36(9, position(), Date(-1000, 2)));
        SettlementDamageParams params;
        params.m_population_min = 0;
        params.m_population_max = 10;
        params.m_building_chance = 5;
        params.m_building_min = 10;
        params.m_building_max = 30;
        params.m_heavy_chance = 0;
        params.m_heavy_min = 0;
        params.m_heavy_max = 0;
        SettlementDamage damage = riot_damage(&params, false, true, false, true);
        if (army) {
            Character* leader = army->unk_007216B9();
            if (leader)
                fn_00CB42BB(&damage, faction);
            unk_ac(leader);
        }
    } else {
        // A riot, of a severity from 0 to 30.
        int severity = lbl_02921A30->m_random.range(0, 30);
        int severity2 = severity;
        int over_25 = max_int(severity - 25, 0);
        lbl_012CED70.unk_004392F9(EventUnk437A36(9, position(), Date(-1000, 2)));
        SettlementDamageParams params;
        int low = severity * 10 / 30;
        int high = severity * 20 / 30;
        params.m_population_min = low;
        params.m_population_max = high;
        params.m_garrison_min = low;
        params.m_garrison_max = high;
        params.m_character_min = low;
        params.m_character_max = high;
        params.m_building_chance = severity2;
        params.m_building_min = 10;
        params.m_building_max = 60;
        params.m_heavy_chance = over_25;
        params.m_heavy_min = 30;
        params.m_heavy_max = 100;
        SettlementDamage damage = riot_damage(&params, false, true, false, true);
        fn_00CB4122(&damage);
        if (m_governor.m_character) {
            Character* governor = m_governor.m_character;
            fire_triggers(GovernorCityRiotsEvent(governor->m_record, this, faction));
        }
        fire_triggers(CityRiotsEvent(this, faction));
        m_rioted = true;
    }
}

// The dead faction (other than the owner, when not_owner) with the largest share of the population, if its share is
// above *share.
void Settlement::largest_share(int* faction, unsigned char* share, bool not_owner)
{
    // NONMATCHING - stack slot order differs
    int unused = 0;
    for (unsigned int i = 0; i < 21; i++) {
        int id = i;
        if (lbl_02921A30->unk_00454960(id)) {
            if (!not_owner || id != m_owner->id()) {
                if (lbl_02921A30->unk_00454960(id)->unk_178() == 0 && m_population_share[i] > *share) {
                    *share = m_population_share[i];
                    *faction = id;
                }
            }
        }
    }
}

// What kind of revolt the settlement would have, in this order: in a britain region; to the owner's shadow faction
// when it's dead or waiting (state 3); to its spawns_on_revolt faction when that's waiting; to a dead faction that has
// more than 60% of the population; to the rebels when the campaign's +0x2b9 is set; gladiators, when the owner is a
// playable faction (a guess) and the settlement has an amphitheatre; otherwise to the rebels.
int Settlement::revolt_type()
{
    // NONMATCHING - stack slot order differs, the original tests the two states of a shadow faction in one condition,
    // and it reads the description fields through inline accessors that keep it in a temporary
    int type = REVOLT_REEMERGE;
    Campaign* campaign = lbl_02921A30;
    if (campaign->last_britain_region(unk_b4()))
        return REVOLT_BRITAIN;

    if (m_owner->description()->has_shadowed_by()) {
        Faction* owner = m_owner;
        Faction* shadow = lbl_02921A30->unk_00454960(owner->description()->shadowed_by());
        if (shadow->unk_178() == 0)
            return REVOLT_SHADOW;
        owner = m_owner;
        shadow = lbl_02921A30->unk_00454960(owner->description()->shadowed_by());
        if (shadow->unk_178() == 3)
            return REVOLT_SHADOW;
    }
    if (m_owner->description()->has_shadowing()) {
        Faction* owner = m_owner;
        Faction* shadow = lbl_02921A30->unk_00454960(owner->description()->shadowing());
        if (shadow->unk_178() == 0)
            return REVOLT_SHADOW;
        owner = m_owner;
        shadow = lbl_02921A30->unk_00454960(owner->description()->shadowing());
        if (shadow->unk_178() == 3)
            return REVOLT_SHADOW;
    }
    if (m_owner->description()->has_spawns_on_revolt()) {
        Faction* owner = m_owner;
        Faction* spawn = lbl_02921A30->unk_00454960(owner->description()->spawns_on_revolt());
        if (spawn->unk_178() == 3)
            return REVOLT_SPAWN;
    }

    int faction = 21;
    unsigned char share = 0;
    largest_share(&faction, &share, true);
    if (faction != 21 && lbl_02921A30->unk_00454960(faction)->unk_178() == 0
        && !lbl_02921A30->unk_00454960(faction)->unk_4f4() && share > 60) {
        type = REVOLT_REEMERGE;
    } else if (lbl_02921A30->m_2b9) {
        type = REVOLT_REBELS;
    } else {
        bool found = false;
        if (m_owner->unk_00445B2B()) {
            String region_name("amphitheatres");
            unsigned int unused = 0;
            for (unsigned int i = 0; i < lbl_014DA2B0.m_types.unk_005BEA85(); i++) {
                BuildingType* building_type = lbl_014DA2B0.unk_008D8695(i);
                if (building_type->m_name == region_name) {
                    unsigned int unused2 = 0;
                    for (unsigned int j = 0; j < num_buildings(); j++) {
                        if (m_buildings.unk_005C2B81(j)->m_type == building_type) {
                            found = true;
                            switch (m_buildings.unk_005C2B81(j)->m_level) {
                            case 0:
                                type = REVOLT_GLADIATORS_1;
                                break;
                            case 1:
                                type = REVOLT_GLADIATORS_2;
                                break;
                            default:
                                type = REVOLT_GLADIATORS_3;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!found)
            type = REVOLT_REBELS;
    }
    return type;
}

inline float min_float(const float& a, const float& b)
{
    return a < b ? a : b;
}

inline float max_float(const float& a, const float& b)
{
    return a < b ? b : a;
}

// The settlement revolts: it goes to the faction that the kind of revolt gives (or to `faction`, when that isn't 21),
// which gets an army there led by a new general (or, in a civil war, by the best commander of those who go over).
// Returns the army, or 0 when a revolt that raises its units from the settlement finds none to raise.
Army* Settlement::revolt(int type, int unk, int reason, int faction)
{
    // NONMATCHING - stack slot order differs, the original keeps many fields and call results in temporaries, and this
    // instantiates its own Array<Character*>::erase where the original calls a folded copy
    Faction* owner = m_owner;
    String rebel_name;
    RebelType* rebel = 0;
    int unit_faction = 21;  // the faction whose units and names the army gets
    if (faction == 21) {
        if (type == REVOLT_REEMERGE) {
            unsigned char share = 0;
            largest_share(&faction, &share, true);
            unit_faction = faction;
        } else if (type == REVOLT_SHADOW) {
            if (m_owner->description()->has_shadowed_by())
                unit_faction = m_owner->description()->shadowed_by();
            else
                unit_faction = m_owner->description()->shadowing();
            faction = unit_faction;
        } else if (type == REVOLT_SPAWN) {
            unit_faction = m_owner->description()->spawns_on_revolt();
            faction = unit_faction;
        } else if (type == REVOLT_BRITAIN) {
            String region_name("romano_british");
            unit_faction = fn_008F60D5(region_name)->m_id;
            faction = unit_faction;
        } else if (lbl_02921A30->m_2b9) {
            type = REVOLT_REBELS;
            faction = lbl_01181378;
            unit_faction = region()->m_creator_faction;
            rebel = region()->rebel_type();
        } else {
            // Gladiators, when it has an amphitheatre; the type is worked out again here, as the level plus 4 (where
            // revolt_type gives the level plus 5).
            bool found = false;
            if (owner->unk_00445B2B()) {
                String region_name("amphitheatres");
                unsigned int unused = 0;
                for (unsigned int i = 0; i < lbl_014DA2B0.m_types.unk_005BEA85(); i++) {
                    BuildingType* building_type = lbl_014DA2B0.unk_008D8695(i);
                    if (building_type->m_name == region_name) {
                        unsigned int unused2 = 0;
                        for (unsigned int j = 0; j < num_buildings(); j++) {
                            if (m_buildings.unk_005C2B69(j)->m_type == building_type) {
                                found = true;
                                type = m_buildings.unk_005C2B69(j)->m_level + 4;
                            }
                        }
                    }
                }
            }
            if (found) {
                faction = lbl_01181378;
                unit_faction = region()->m_creator_faction;
                rebel_name = "gladiator_uprising";
                rebel = fn_00903C7E(rebel_name);
            } else {
                type = REVOLT_REBELS;
                faction = lbl_01181378;
                unit_faction = region()->m_creator_faction;
                rebel = region()->rebel_type();
            }
        }
    } else {
        unit_faction = faction;
    }
    Faction* new_owner = lbl_02921A30->faction(faction);

    // The units of every kind but the rebels' come from the settlement (the Romano-British's from every infantry and
    // cavalry unit they own), and from the region's list.
    RecruitmentItems units;
    BuildingEffects effects;
    unk_005C58A1(unit_faction, &effects);
    if (type == REVOLT_REEMERGE || type == REVOLT_SPAWN || type == REVOLT_BRITAIN || type == REVOLT_SHADOW) {
        if (type == REVOLT_BRITAIN) {
            unsigned int unused = 0;
            for (unsigned int i = 0; i < lbl_015C95B4; i++) {
                UnitUnk50* unit_type = &lbl_01589644[i];
                if ((unit_type->m_2c == 0 || unit_type->m_2c == 1) && (unit_type->m_ownership & (1 << faction)) != 0)
                    units.add()->unk_005AE4F0(this, &lbl_01589644[i], 0, 0, 0, 100);
            }
        } else {
            recruitable_units(&units, &effects, false, false);
        }
        RegionRevoltUnits* extra = region()->m_88;
        if (extra) {
            unsigned int unused = 0;
            for (unsigned int i = 0; i < extra->m_1c.unk_004585E5(); i++) {
                RegionRevoltUnit* entry = extra->unk_005BE904(i);
                for (int count = float_to_int_chop(entry->m_count); count > 0; count--)
                    units.add()->unk_005AE4F0(this, entry->m_type, extra->unk_005BE904(i)->m_experience, 0, 0, 100);
            }
        }
        if (units.count() == 0) {
            change_owner(0, new_owner, reason);
            return 0;
        }
    }

    String kind;
    if (type == REVOLT_SHADOW && (new_owner->m_178 == 0 || new_owner->m_178 == 3))
        kind = "region_name character";
    else if ((type == REVOLT_SPAWN || type == REVOLT_BRITAIN) && new_owner->m_178 == 3)
        kind = "region_name character";
    else
        kind = "general";
    Character* leader = 0;
    Army* army = 0;

    // In a civil war, characters go over; the first of them, or the last found with more command than the first, leads.
    static Array<Character*> defectors;
    defectors.clear();
    if (type == REVOLT_SHADOW) {
        find_defectors(&defectors, new_owner);
        int command = 0;
        if (defectors.size() != 0) {
            command = defectors[0]->m_record->command(0);
            leader = defectors[0];
        }
        unsigned int unused = 0;
        for (unsigned int i = 0; i < defectors.size(); i++) {
            Character* character = defectors[i];
            if (character->m_record->command(0) > command && leader != character) {
                defectors.erase(i);
                defectors.insert(character, 0);
                leader = character;
            }
        }
    }
    if (leader) {
        leader->unk_005877ED(0);
        leader->unk_00581119();
        army = leader->army();
        if (!army)
            army = leader->unk_00581089(leader->unk_00581119());
        if (army)
            leader->army();
        unsigned int unused = 0;
        for (unsigned int i = 1; i < defectors.size(); i++) {
            Character* character = defectors[i];
            character->unk_00588F38(army);
            int a = 0;
            int b = 0;
            character->unk_00581119()->unk_007B64D2(army, a, b);
            army->unk_007216B9();
        }
        // The garrison's units without a general join the army.
        if (m_garrison.m_object) {
            int unused2 = 0;
            for (int j = 0; j < m_garrison.m_object->num_units(); j++) {
                Unit* unit = m_garrison.m_object->unit_at(j);
                if (unit->unk_278() == 0) {
                    int a = 0;
                    int b = 0;
                    unit->unk_007B64D2(army, a, b);
                    j--;
                    if (!m_garrison.m_object)
                        break;
                }
            }
        }
        leader = army->unk_007216B9();
        leader->unk_00581E49(new_owner);
        unsigned int unused3 = 0;
        for (unsigned int k = 0; k < defectors.size(); k++) {
            Character* character = defectors[k];
            fn_00CBB9AE(character, owner, new_owner);
            character->unk_0057FAE2(new_owner, 1);
        }
    } else {
        // A new general, aged 20 to 50; a region_name character is adopted by a man of the faction with fewer than 4
        // children, a living one if there is one.
        int first_name = 0;
        int surname = 0;
        fn_008FF5CB(&lbl_02921A30->m_random, unit_faction, 1, &first_name, &surname);
        leader = fn_008E1FA5(&kind, new_owner, unit_faction, lbl_02921A30->m_random.range(20, 50), first_name, surname);
        lbl_02921A1C->unk_004ABED9(leader, position());
        army = new_owner->unk_004C9515(leader, 0);
        if (leader->type() == 5) {
            CharacterRecord* father = 0;
            unsigned int unused = 0;
            for (unsigned int i = 0; i < new_owner->m_records.size(); i++) {
                CharacterRecord* record = new_owner->m_records.m_items[i];
                if (record != leader->m_record && record->m_male && record->m_flag16 && !record->m_flag2
                    && record->m_num_children < 4) {
                    if (father && !father->m_alive && record->m_alive)
                        father = record;
                    if (!father)
                        father = record;
                    if (father->m_alive)
                        break;
                }
            }
            if (father)
                leader->m_record->adopt(father);
            fire_triggers(UnkMessage8A4586(leader->m_record));
            army->unk_0071DADD(leader, 0, 0, 0, 0);
        }
        new_owner->unk_180()->unk_00492498(leader, false);
    }
    if (type == REVOLT_SHADOW || type == REVOLT_SPAWN || type == REVOLT_BRITAIN) {
        if (type == REVOLT_BRITAIN)
            lbl_02921A30->m_2c4 = true;
        new_owner->unk_004CDFCF(leader);
    }
    if (rebel && army)
        army->unk_007201B3(rebel, unit_faction);

    // How strong an army it raises.
    float strength = 0.0f;
    if (type == REVOLT_SHADOW)
        strength = (revolt_strength() - army->strength()) * 0.5f;
    else if (type == REVOLT_BRITAIN)
        strength = revolt_strength() * 1.3f;
    else if (reason == 5)
        strength = (revolt_strength() - army->strength()) * 0.25f;
    else
        strength = revolt_strength();
    if (reason == 9) {
        float high = 100.0f;
        float low = 20.0f;
        strength = min_float(max_float(strength, low), high);
    }

    if (leader)
        unk_0049B15C(leader, unk, reason, reason != 5);
    else
        unk_0049B043(new_owner, reason, reason != 5);
    unk_0049A894(true);
    if (type == REVOLT_REEMERGE || type == REVOLT_SPAWN || type == REVOLT_BRITAIN || type == REVOLT_SHADOW)
        army->unk_00720D26(type, strength, lbl_00FEC4E8[m_level].m_min, lbl_00FEC4E8[m_level].m_max, &units);
    else
        army->unk_007201D6(type, strength, lbl_00FEC4E8[m_level].m_min, lbl_00FEC4E8[m_level].m_max, rebel);

    // A spawned faction and the Romano-British also get a diplomat, aged 20 to 40.
    if (type == REVOLT_SPAWN || type == REVOLT_BRITAIN) {
        int first_name = 0;
        int surname = 0;
        fn_008FF5CB(&lbl_02921A30->m_random, m_owner->id(), 1, &first_name, &surname);
        Character* diplomat = fn_008E212F(2, m_owner, lbl_02921A30->m_random.range(20, 40),
                                          fn_008FF4A3(0, m_owner->id(), first_name),
                                          fn_008FF4A3(2, m_owner->id(), surname), m_owner->id());
        diplomat->unk_00581619(diplomat->unk_005815D7());
        lbl_02921A1C->unk_004ABED9(diplomat, position());
        m_owner->unk_180()->unk_00492498(diplomat, false);
        unk_0049BC60(diplomat);
        diplomat->unk_004B7AE0();
    }
    if (leader)
        leader->unk_00587657();
    return army;
}

// How strong an army a revolt raises: the average strength of the owner's armies in this region and the regions next
// to it, averaged with that of all its armies, times 2 less the owner's share of the population (so twice as strong when
// none of the population is the owner's), times 1.5 while the region's +0x1b0 is set; with a garrison, from 2 to 3
// times the garrison's strength.
float Settlement::revolt_strength()
{
    // NONMATCHING - stack slot order differs, and this zeroes one more slot
    ArmyStrengths strengths;
    strengths.clear();
    fn_004E4403(m_owner->id(), m_region_id, &strengths);
    int total = strengths.m_own;
    int count = strengths.m_own_count;
    unsigned int unused = 0;
    for (unsigned int i = 0; i < region()->m_neighbours.size(); i++) {
        RegionNeighbour* neighbour = &region()->m_neighbours[i];
        fn_004E4403(m_owner->id(), neighbour->m_region, &strengths);
        total += strengths.m_own;
        count += strengths.m_own_count;
    }
    if (count == 0)
        count++;
    float nearby = (float)total / (float)count;
    int armies = 0;
    int all = fn_004E282B(m_owner, &armies);
    if (armies == 0)
        armies++;
    float overall = (float)all / (float)armies;
    float strength = (nearby + overall) / 2.0f;
    float factor = 1.0f - m_population_share[m_owner->id()] / 100.0f + 1.0f;
    strength *= factor;
    if (region()->m_1b0)
        strength *= 1.5f;
    if (unk_40()) {
        Army* garrison = unk_40();
        float most = (float)garrison->strength() * 3.0f;
        float least = 2.0f;
        strength = min_float(max_float(strength, least), most);
    }
    return strength;
}

// Orders characters by influence, the most first.
struct MoreInfluence {
    bool operator()(Character* a, Character* b) const
    {
        int first = a->m_record->influence();
        int second = b->m_record->influence();
        return first > second;
    }
};

template <class T>
inline void swap_items(T& a, T& b)
{
    T swap = a;
    a = b;
    b = swap;
}

// A selection sort of first to last (last included): each place gets the item that pred puts before the rest, the
// later one of a tie.
template <class Iterator, class Pred>
void selection_sort(Iterator first, Iterator last, Pred pred)
{
    Iterator end = last;
    ++end;
    for (Iterator i = first; i != last; ++i) {
        Iterator best = i;
        Iterator j = i;
        ++j;
        while (j != end) {
            if (!pred(*best, *j))
                best = j;
            ++j;
        }
        if (best != i)
            swap_items(*i, *best);
    }
}

// Which of the settlement's characters go over to the faction in a civil war (into defectors). Only its region_name
// characters that aren't the faction's already can, as many as the faction's men have room to adopt; the most
// influential go first. Each one goes with a chance of 100 less 10 times its loyalty, plus 10 for each region next to
// this one held by a faction in a civil war with the owner, plus a total that grows by 10 for each army of one besieging
// it with each character checked (it's never reset), from 1% to 90%. A quarter of the influence of those that have gone
// counts against the loyalty of the rest.
void Settlement::find_defectors(Array<Character*>* defectors, Faction* faction)
{
    // NONMATCHING - stack slot order differs, and the original reads each faction description again for its shadow
    // checks where this keeps the result in a temporary
    Campaign* campaign = lbl_02921A30;
    if (!campaign->unk_2b8())
        return;
    unsigned int room = faction->m_88c.unk_004D3219();
    if (room == 0)
        return;

    Array<Character*> candidates;
    unsigned int i;
    for (i = 0; i < num_characters(true); i++) {
        Character* character = this->character(i, true);
        if (room != 0 && character->type() == 5 && character->unk_00580C1E() != faction
            && !character->m_record->m_leader) {
            room--;
            candidates.push_back(character);
        }
    }
    if (candidates.empty())
        return;
    selection_sort(candidates.begin(), candidates.last(), MoreInfluence());

    Region* region = this->region();
    int nearby = 0;
    for (i = 0; i < region->m_neighbours.size(); i++) {
        StrategyMap* map = lbl_02921A34;
        Region* neighbour = map->region(region->m_neighbours[i].m_region);
        Faction* owner = neighbour->m_owner;
        if (owner) {
            if (owner->m_description->has_shadowed_by() && m_owner->id() == owner->m_description->shadowed_by())
                nearby += 10;
            else if (owner->m_description->has_shadowing() && m_owner->id() == owner->m_description->shadowing())
                nearby += 10;
        }
    }

    float influence = 0.0f;
    int besiegers = 0;
    for (i = 0; i < candidates.size(); i++) {
        Character* character = candidates[i];
        CharacterRecord* record = character->m_record;
        int character_influence = record->influence();
        Character* same = character;
        int character_loyalty = same->m_record->loyalty();
        int loyalty = float_to_int_chop(((float)character_loyalty - influence) * 10.0f);
        if (besieged()) {
            for (unsigned int j = 0; j < m_84; j++) {
                if (siege(j)) {
                    Faction* besieger = siege(j)->unk_00717C38()->unk_00580C1E();
                    if ((besieger->m_description->has_shadowed_by()
                         && m_owner->id() == besieger->m_description->shadowed_by())
                        || (besieger->m_description->has_shadowing()
                            && m_owner->id() == besieger->m_description->shadowing())) {
                        besiegers += 10;
                    }
                }
            }
        }
        int chance = 100 - loyalty + nearby + besiegers;
        int high = 90;
        int low = 1;
        chance = min_int(max_int(chance, low), high);
        Campaign* now = lbl_02921A30;
        if (now->m_random.percent() < chance) {
            defectors->push_back(character);
            influence += character_influence / 4.0f;
        }
    }
}

// At the start of the owner's turn: one more turn since the owner changed, its building effects are worked out again,
// it switches from games to races when it can stage only races, and the SettlementTurnStart triggers fire.
void Settlement::start_turn()
{
    m_owner_turns++;
    unk_0049A464();
    BuildingEffects effects;
    unk_005BCA50(&effects);
    unk_005B6C52(&effects);
    bool games = false;
    if (can_stage_games())
        games = true;
    if (can_stage_races() && !games && entertainment() < 3)
        set_entertainment(3);
    unk_005B6BDB(&effects);
    check_spies();
    fire_triggers(SettlementTurnStartEvent(this));
}

// At the end of the owner's turn: a settlement without plague may catch it from its squalor, a chance of
// (squalor - 3 - 2 * health) / 2 in 1001. The SettlementTurnEnd triggers fire at the end.
void Settlement::end_turn()
{
    // NONMATCHING - the original keeps the health factor out of a temporary, and the stack slots of stats and squalor
    // are swapped
    if (unk_005BC86B())
        unk_005BC88A(0);
    if (!m_plague) {
        SettlementStats* stats = &m_dd8.m_current;
        int squalor = stats->growth(GROWTH_SQUALOR);
        if (squalor > 3) {
            int chance = (squalor - 3 - 2 * stats->growth(GROWTH_HEALTH)) >> 1;
            Campaign* campaign = lbl_02921A30;
            if (campaign->m_random.range(0, 1000) < chance) {
                m_plague = true;
                m_plague_deaths = 0;
                m_plague_ticks = 0;
            }
        }
    }
    BuildingEffects effects;
    unk_005BCA50(&effects);
    unk_005B68EA();
    check_riots(&effects);
    unk_0049A46D();
    fire_triggers(SettlementTurnEndEvent(this));
}

// Whether the character would make a better governor: a region_name character, when there's no governor or his Management
// is higher than the governor's.
bool Settlement::better_governor(Character* character)
{
    if (character->type() == 5) {
        Character* current = governor();
        if (!current)
            return true;
        if (character->record()->management() > governor()->record()->management())
            return true;
    }
    return false;
}

// Each turn, every foreign spy inside may be caught (Faction::catch_spy). The owner's strength against them is the
// combined Subterfuge of its own spies and region_name characters on the settlement's tile, plus 1, plus the governor's
// PublicSecurity.
void Settlement::check_spies()
{
    // NONMATCHING - the original keeps the third path finder in a temporary
    static Array<Character*> s_watchers;
    s_watchers.clear();
    path_finder()->unk_004A99D9(position(), &s_watchers, m_owner, 0);
    path_finder()->unk_004A99D9(position(), &s_watchers, m_owner, 5);
    float strength = combined_subterfuge(s_watchers) + 1.0f;
    if (governor())
        strength += governor()->record()->attribute_at(ATTR_PUBLIC_SECURITY);
    static Array<Character*> s_spies;
    s_spies.clear();
    path_finder()->unk_004A9C59(this, &s_spies);
    for (unsigned int i = 0; i < s_spies.size(); i++)
        m_owner->catch_spy(s_spies[i], strength, this);
}

Region* Settlement::region()
{
    StrategyMap* map = lbl_02921A34;
    return map->region(m_region_id);
}

// Hands the settlement to a new owner. The reason is 1 when an army takes it, 2 when it revolts, 5 when it's traded in
// a deal, 6 when all of a faction's settlements go at once, 7 when it's bribed (9 is not identified); captor is the
// character who took it, or 0.
void Settlement::change_owner(CharacterRecord* captor, Faction* faction, int reason)
{
    // NONMATCHING - stack slot order differs, the original keeps two more call results in temporaries, and it passes
    // reason != 1 without zeroing eax first
    Faction* old_owner = owner();
    PathFinder* paths = lbl_02921A1C;
    paths->unk_004AC371(this);

    // The countdown of unrest after a change of owner: 6 turns, or only 1 after a trade.
    if (reason == 5)
        m_owner_turns = 5;
    else
        m_owner_turns = 0;
    m_98 = reason == 5 ? old_owner->id() : 21;

    old_owner->unk_180()->unk_00492B0F(this, true);
    if (m_184 && m_184->unk_6c() == 0)
        old_owner->unk_180()->unk_00492B0F(m_184, true);
    FactionTileUnk* tile = old_owner->unk_180()->unk_00493794(*(RoadPoint*)position());
    if (tile && tile->unk_08())
        tile->unk_04(faction);
    if (m_184 && m_184->unk_6c() == 0) {
        tile = old_owner->unk_180()->unk_00493794(*(RoadPoint*)m_184->position());
        if (tile && tile->unk_0c())
            tile->unk_04(faction);
    }

    unk_0049B680(faction, reason);
    if (old_owner->is_unk_a7())
        set_rebels(0, 21);
    fn_008B4D04(captor, this, faction, old_owner, reason);
    SettlementOwnerChange change;
    change.m_settlement = this;
    change.m_faction = faction;
    m_e0.unk_005BF3A1(&change);
    unk_005B6F77(faction);

    // Who rebels in it: the captor's army's rebels, or the region's when the rebels take it.
    int rebel_faction = 21;
    if (captor) {
        Army* army = captor->m_character->army();
        rebel_faction = army->m_rebel_faction;
        set_rebels(army->m_rebel_type, army->m_rebel_faction);
    } else if (m_owner->is_unk_a7() && m_rebel_faction == 21) {
        set_rebels(region()->rebel_type(), region()->creator_faction());
    }

    if (old_owner->unk_178() == 0 && old_owner->capital() == 0)
        old_owner->unk_004CF14F();
    Campaign* campaign = lbl_02921A30;
    if (!campaign->unk_0045460A() || faction->unk_1d4())
        unk_005BD77B(unk_005BD9DA(), true);
    else
        unk_005BD74B();
    m_1e0.unk_005C2497();
    m_318.unk_005C5559();
    m_tax_level = 1;

    // Games, or races when it can stage only races.
    bool games = false;
    if (can_stage_games()) {
        games = true;
        set_entertainment(0);
    }
    if (!games && can_stage_races())
        set_entertainment(3);

    if (m_a68 != *position())
        unk_005BAE31(position());

    unsigned int i;
    for (i = 0; i < region()->m_44.size(); i++) {
        RegionUnk44* unk = region()->m_44[i];
        if (unk->unk_38() == 0)
            unk->unk_004BF34E(reason != 1, faction, 0);
    }
    StrategyMap* map = lbl_02921A34;
    map->unk_004823F9(m_region_id, faction);
    m_owner->unk_180()->unk_00492B0F(this, false);
    paths = lbl_02921A1C;
    paths->unk_004AB5CC(this);

    if (m_184) {
        bool kept = false;
        Army* army = m_184->unk_48();
        if (army && army->unk_00721587()->stance(faction) != 600)
            kept = army->unk_0072309A();
        if (m_184->unk_0049EF76()) {
            army = m_184->unk_0049EF76()->unk_48();
            if (army && army->unk_00721587()->stance(faction) != 600)
                kept = army->unk_0072309A();
        }
        if (!kept) {
            if (m_184->unk_6c() == 0)
                m_owner->unk_180()->unk_00492B0F(m_184, false);
            m_184->unk_0049E140(faction, 0);
        }
        if (m_184->unk_0049EF76()) {
            SettlementUnk184* other = m_184->unk_0049EF76();
            paths = lbl_02921A1C;
            Army* there = paths->unk_004AADAE(other->position());
            if (there && there->unk_00721587() != m_owner) {
                Character* leader = there->unk_007216B9();
                if (leader->unk_00583B60() && leader->m_154.unk_0056BE44()) {
                    paths = lbl_02921A1C;
                    paths->m_6c.unk_0056CEC4(leader);
                } else {
                    there->unk_00724043(1, 1, 3, 1);
                }
            }
        }
    }

    m_owner->unk_004D3E6F(0);
    old_owner->unk_004D3E6F(0);
    SettlementOwnerChange change2;
    change2.m_settlement = this;
    change2.m_faction = faction;
    m_120.unk_005BF675(&change2);
    fn_008B529F(this, faction);
    m_dd5 = !unk_005B7AEA();
    m_b1 = false;
    m_a8a = false;
    m_a8b = false;
    map = lbl_02921A34;
    map->unk_00482A48(position())->unk_004841C6();
    paths = lbl_02921A1C;
    paths->unk_004AC1DB(this);
    campaign = lbl_02921A30;
    if (campaign->last_britain_region(region())) {
        campaign = lbl_02921A30;
        campaign->m_2c4 = true;
        campaign = lbl_02921A30;
        campaign->unk_00457C3C();
    }
}

// After a revolt, the new army's leader and the characters with him come into the settlement, and each one who would
// make a better governor (better_governor) becomes the governor.
void Settlement::unk_ac(Character* leader)
{
    // NONMATCHING - the original's second read of the battle's +0x08 is a temporary, not a local
    if (lbl_018BF5E0->unk_8() != 4)
        int state = lbl_018BF5E0->unk_8();
    Army* army = leader->army();
    if (army)
        unk_84(army);
    if (better_governor(leader))
        set_governor(leader);
    else if (leader->type() == 4)
        leader->unk_005813AC();
    else
        unk_0049BC60(leader);
    while (army->m_characters.size()) {
        Character* character = army->m_characters[0];
        army->unk_0071B214(character, 0);
        if (better_governor(character))
            set_governor(character);
        else
            unk_0049BC60(character);
    }
}

// The cost of training a unit here, in percent: 5% less for each point of the governor's TrainingUnits, from -10 to 10.
int Settlement::unit_cost(BuildingEffects* effects)
{
    int percent = 100;
    if (m_governor.valid()) {
        int training = governor()->record()->attribute_at(ATTR_TRAINING_UNITS);
        training = clamp_int(training, -10, 10);
        percent -= training * 5;
    }
    return percent;
}

extern "C" int fn_005D44D5(int weapon);  // a weapon kind's building effect, or NUM_EFFECTS for none

// Adds the units the buildings can train, the ships or the others. An animal unit costs 5% less for each point of the
// governor's TrainingAnimalUnits. A unit whose weapon can be upgraded gets the level of its weapon's effect, from -1 to
// 3, and isn't listed at -1. With owned_only, only the units the owner can have are listed.
void Settlement::recruitable_units(RecruitmentItems* units, BuildingEffects* effects, bool ships, bool owned_only)
{
    // NONMATCHING - stack slot order only: the original has the cost, loop index and count, and the per-unit locals,
    // in other slots
    int cost = unit_cost(effects);
    unsigned int i;
    unsigned int count = effects->num_recruits();
    for (i = 0; i < count; i++) {
        BuildingRecruit recruit;
        effects->recruit(&recruit, i);
        UnitUnk50* type = &lbl_01589644[recruit.m_unit];
        int unit_cost = cost;
        if (ships == (type->m_2c == 4)) {
            if (type->is_animal() && governor())
                unit_cost = float_to_int(unit_cost * (1.0f - 0.05f * governor()->record()->attribute_at(ATTR_TRAINING_ANIMAL_UNITS)));
            short weapon = -1;
            if (type->m_d0 & 1) {
                int effect = fn_005D44D5(type->m_weapon);
                if (effect == NUM_EFFECTS) {
                    weapon = 0;
                } else {
                    weapon = effects->effect(effect).total();
                    weapon = clamp_short(weapon, -1, 3);
                }
            } else {
                weapon = 0;
            }
            bool listed = true;
            if (owned_only) {
                if (!type->owned_by(owner_id())) {
                    listed = false;
                    bool minor = m_owner->is_unk_a7();
                }
            }
            if (listed) {
                if (!(type->m_d0 & 1) || weapon >= 0) {
                    RecruitmentItem* unit = units->add();
                    if (ships)
                        unit->unk_005AE6C3(this, type, recruit.m_experience.total(), effects->unk_005D48F2(EFFECT_ARMOUR),
                                           weapon, (unsigned char)unit_cost);
                    else
                        unit->unk_005AE4F0(this, type,
                                           recruit.m_experience.total() + effects->unk_005D48AE(EFFECT_RECRUITS_EXP_BONUS),
                                           effects->unk_005D48F2(EFFECT_ARMOUR), weapon, (unsigned char)unit_cost);
                }
            }
        }
    }
}

bool Settlement::can_stage_games()
{
    BuildingEffects effects;
    unk_005BCA50(&effects);
    return effects.effect(EFFECT_STAGE_GAMES).total() > 0;
}

bool Settlement::can_stage_races()
{
    BuildingEffects effects;
    unk_005BCA50(&effects);
    return effects.effect(EFFECT_STAGE_RACES).total() > 0;
}

void Settlement::set_entertainment(int entertainment)
{
    if (m_entertainment != entertainment) {
        m_entertainment = entertainment;
        m_dd8.update(true);
    }
}

int Settlement::entertainment()
{
    return m_entertainment;
}

int Settlement::region_id()
{
    return m_region_id;
}

extern "C" {
void* fn_0041374F(void* saved);
void* fn_00413785(void* saved);
void* fn_004137F3(void* saved);
void* fn_0041380E(void* saved);
void* fn_0041385F(void* saved);
void* fn_00413895(void* saved);
}

inline StrategyMap* strategy_map()
{
    StrategyMap* map = lbl_02921A34;
    return map;
}

inline PathFinder* route_finder()
{
    PathFinder* paths = lbl_02921A1C;
    return paths;
}

// After a load: its pointers become the loaded objects' new addresses, and it's put back on the map. The campaign's
// Rome and Constantinople are the settlements with those names in descr_regions.txt, whoever holds them. Saves older
// than version 8 get the shown name from the region.
void Settlement::after_load()
{
    unsigned short i = 0;
    unsigned short num_sieges = m_84;
    unsigned short count = m_abc.size();
    m_owner = (Faction*)fn_0041374F(m_owner);
    m_184 = (SettlementUnk184*)fn_00413785(m_184);
    m_governor.restore((Character*)fn_004137F3(m_governor.get()));
    m_a9c.restore((Character*)fn_004137F3(m_a9c.get()));
    m_garrison.restore((Army*)fn_0041380E(m_garrison.get()));
    for (i = 0; i < num_sieges; i++)
        m_sieges[i].restore((Siege*)fn_00413895(m_sieges[i].get()));
    m_318.unk_005C56E9();
    m_1e0.unk_005C26DB();
    for (i = 0; i < count; i++)
        m_abc[i] = fn_0041385F(m_abc[i]);
    if (saved_version("SETTLEMENT") < 8u)
        m_name = strategy_map()->region(m_region_id)->m_settlement_display_name;
    const String& region_name = region()->m_settlement_name;
    if (region_name == String("Rome")) {
        campaign()->rome();  // read and dropped
        campaign()->set_rome(this);
    }
    if (region_name == String("Constantinople")) {
        campaign()->constantinople();
        campaign()->set_constantinople(this);
    }
    m_dd8.set_faction(m_owner);
    strategy_map()->set_settlement(m_region_id, this);
    unk_005BACCE(&m_a68);
    m_dd8.update(true);
    if (m_owner->is_unk_a7() && m_rebel_faction == 21)
        m_rebel_faction = region()->creator_faction();
    route_finder()->unk_004AC1DB(this);
}

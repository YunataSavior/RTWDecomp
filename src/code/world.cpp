#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\world.cpp"
// The campaign: its factions and its settings. (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"
#include "character.h"
#include "ai/ai_cheat_manager.h"
#include "ai/ai_senate_faction.h"

// How much bigger or smaller units are than at the normal unit size, which the garrison's public order weighs the
// population by.
float Campaign::unit_scale()
{
    return settings()->m_unit_scale;
}

// The faction's difficulty, 0 easy to 3 very hard.
int Campaign::difficulty(int faction)
{
    return m_difficulty[faction];
}

int Campaign::unk_004529FB(int faction)
{
    return m_114[faction];
}

extern "C" {
void fn_00CEA1BD(int);
}

void Campaign::unk_00452A2D()
{
    fn_00CEA1BD(1);
    m_0 = true;
    m_turn = 0;
    m_diplomacy.unk_005A2C6A();
    m_0 = false;
}

// Each human faction's unk_004CA308(true), then the AI's cheats are worked out again (AICheatManager::update).
void Campaign::unk_00452A62()
{
    m_0 = true;
    for (unsigned int i = 0; i < m_b8; i++)
        m_factions_by_id[m_humans[i].m_faction]->unk_004CA308(true);
    m_22c = m_factions[m_230];
    m_0 = false;
    AICheatManager::update();
}

class Unk02921A58 {
public:
    void unk_004A1030();                // 0x004A1030
};

class Unk02921A10 {
public:
    void unk_005970F3();                // 0x005970F3
};

extern "C" {
extern bool lbl_012CA767;
extern Unk02921A58* lbl_02921A58;
extern Unk02921A10* lbl_02921A10;
void fn_005214ED();
void fn_005F50EF();                     // (when religions are enabled)
void fn_00CB4D9D(Faction* faction, int);
}

static inline Unk02921A58* unk_02921A58()
{
    Unk02921A58* unk = lbl_02921A58;
    return unk;
}

static inline StrategyMap* strategy_map()
{
    StrategyMap* map = lbl_02921A34;
    return map;
}

static inline PathFinder* route_finder()
{
    PathFinder* finder = lbl_02921A1C;
    return finder;
}

// The start of a round: the turn number goes up, every faction's turn update (Faction::unk_004CAC88) and neighbours,
// and the senate's turn while it's alive.
void Campaign::unk_0045361A()
{
    m_turn++;
    if (!lbl_012CA767) {
        for (unsigned int i = 0; i < m_num_factions; i++) {
            FactionUnkD4* unk = m_factions[i]->m_d4;
            if (unk)
                unk->unk_10();
        }
    }
    m_22c = m_factions[m_230];
    unk_0045732F();
    m_date.unk_0043C59F();
    unk_02921A58()->unk_004A1030();
    unsigned int regions = strategy_map()->m_region_list.size();
    for (unsigned int j = 0; j < regions; j++)
        strategy_map()->m_regions[j].unk_00483B67();
    for (unsigned int k = 0; k < m_num_factions; k++) {
        m_factions[k]->unk_004CAC88();
        m_factions[k]->unk_004CB391();
    }
    lbl_02921A10->unk_005970F3();
    if (!m_prologue) {
        route_finder()->unk_004B31E9();
        fn_005214ED();
    }
    lbl_012CED70.unk_0043900D();
    if (m_senate && m_senate->faction()->unk_178() == 0)
        m_senate->update();
    unk_00453942();
    if (religions_enabled())
        fn_005F50EF();
}

// The start of the turn of the faction at m_230: the AI's cheats are worked out again (AICheatManager::update), its
// Faction::start_turn when unk_178() is 0, every character, settlement and fort is told whose turn it is, and while m_0
// is set its forts with no garrison and nobody inside are taken off the map.
void Campaign::unk_00453B9B()
{
    m_22c = m_factions[m_230];
    route_finder()->m_1c = 3;
    route_finder()->unk_004A434A();
    AICheatManager::update();
    if (m_22c->unk_178() == 0)
        m_22c->start_turn();
    for (unsigned int i = 0; i < m_num_factions; i++) {
        unsigned int j;
        Faction* other_faction = m_factions[i];
        for (j = 0; j < other_faction->m_characters.size(); j++)
            other_faction->m_characters[j]->unk_005805DC(m_22c->id());
        for (j = 0; j < other_faction->m_settlements.size(); j++)
            other_faction->m_settlements[j]->unk_005BD3CE(m_22c->id());
        for (j = 0; j < other_faction->m_forts.size(); j++)
            other_faction->m_forts[j]->unk_00498B82(m_22c->id());
        if (m_0) {
            for (int k = 0; k < (int)other_faction->m_forts.size(); k++) {
                Fort* fort = other_faction->m_forts[k];
                if (fort->unk_40() == 0 && fort->num_characters(true) == 0 && fort->owner() == unk_22c()) {
                    unk_02921A38()->unk_00C0A689(fort);
                    m_forts.unk_0049954D(fort);
                    k--;
                }
            }
        }
    }
    fn_00CB4D9D(m_22c, 0);
}

extern "C" {
extern Array<int> lbl_012D6FA0;             // (not identified)
void fn_0097A192();                         // clears 0x015CD164
void* fn_00D93A0C(int text);                // a text of this number (a guess)
void fn_00CB691A(void* title, void* text, const char* image);  // shows a message (a guess)
}

// The end of a round, after every faction's turn. When a major faction holds a huge city in Italy (see
// Faction::unk_004CAEDB), the Marian reforms come for every faction, unless they already have or descr_strat.txt
// disables them.
void Campaign::unk_004537F2()
{
    StrategyMap* map;
    fn_0097A192();
    m_diplomacy.unk_005A2EEE();
    if (m_date.season() == 2) {
        map = lbl_02921A34;
        map->unk_00482CC0();
    }
    unk_004544AA();
    bool reformed = false;
    unsigned int i;
    for (i = 0; i < m_num_factions; i++) {
        if (m_factions[i]->unk_004CAEDB() && !m_marian_reforms && !m_marian_reforms_disabled) {
            reformed = true;
            fn_00CB691A(fn_00D93A0C(0x2fb), fn_00D93A0C(0x31e), "marian_reform");
        }
    }
    if (reformed) {
        m_marian_reforms = true;
        for (i = 0; i < m_num_factions; i++)
            m_factions[i]->marian_reforms();
    }
    m_cd5c.unk_005F2A2F();
    AICheatManager::update_war_turns();
    lbl_012D6FA0.clear();
    PathFinder* route_finder = lbl_02921A1C;
    route_finder->m_84.unk_00490EE4();
}

bool Campaign::unk_00454978()
{
    return lbl_02921AE0 && lbl_018BF5E0->unk_46();
}

// The index'th human faction, or 21 when there isn't one. An index equal to the count passes the check.
int Campaign::human_faction(int index)
{
    if (index <= m_b8)
        return m_humans[index].m_faction;
    return 21;
}

Settlement* Campaign::rome()
{
    return get_rome();
}

// Whether the region is the last one with the britain hidden resource that a major faction holds (other than this
// one, none is held by a faction with custom_battle_availability and without description+0xa7), while it isn't the
// Romano-British's and they haven't been spawned yet (m_2c4). A revolt there goes to the Romano-British (BI).
bool Campaign::last_britain_region(Region* region)
{
    // NONMATCHING - stack slot order differs
    if (!m_2c4) {
        int resource = lbl_014DA2B0.unk_008DB1B9(String("britain"));
        if (resource >= 0 && region->has_resource(resource)) {
            FactionDescription* description = region->m_owner->m_description;
            bool not_romano_british = description != fn_008F60D5(String("romano_british"));
            if (not_romano_british) {
                unsigned int unused = 0;
                unsigned int i;
                for (i = 0; i < lbl_02921A34->num_regions(); i++) {
                    Region* other = lbl_02921A34->region(i);
                    if (!(other->id() == region->id()) && other->m_owner && other->m_owner->unk_00445B2B()
                        && other->has_resource(resource))
                        break;
                }
                if (i == lbl_02921A34->num_regions())
                    return true;
            }
        }
    }
    return false;
}

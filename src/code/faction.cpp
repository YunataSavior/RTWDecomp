#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\faction.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A faction's campaign record: its turn update.
#include "unsorted.h"
#include "horde.h"
#include "ai/ai_faction_base.h"
#include "army.h"
#include "building_db.h"
#include "campaign_triggers.h"
#include "capabilities.h"
#include "event_manager.h"
#include "faction.h"
#include "faction_db.h"
#include "fort.h"
#include "port.h"
#include "romans_utils.h"
#include "settlement.h"
#include "strategy_map.h"
#include "unit.h"
#include "unit_db.h"
#include "world.h"
#include "world_map.h"

#include "character.h"
#include "character_record.h"
#include "senate_mission.h"

// Runs at the end of the faction's turn: its characters age and act, its settlements are updated, and the public
// order bonuses for a triumph and for capturing a wonder each wind down by 1.
void Faction::unk_004CAC88()
{
    if (m_178 == 0) {
        unsigned int i;
        int j;
        for (i = 0; i < m_records.size(); i++)
            m_records.m_items[i]->update_age();
        for (i = 0; i < m_armies.size(); i++)
            m_armies.m_items[i]->unk_00719E61();
        for (i = 0; i < m_characters.size(); i++)
            m_characters.m_items[i]->unk_0057DD3B();
        for (j = 0; j < (int)m_characters.size(); j++) {
            if (m_characters.m_items[j]->unk_a0()) {
                if (unk_004C5093(m_characters.m_items[j], true, false))
                    j--;
            }
        }
        for (i = 0; i < m_characters.size(); i++)
            m_characters.m_items[i]->start_turn();
        for (i = 0; i < m_settlements.size(); i++)
            m_settlements.m_items[i]->unk_005B5D57();
        if (m_control == 1)
            m_88c.make_offers();
        for (i = 0; i < m_characters.size(); i++)
            m_characters.m_items[i]->unk_00587731();
        if (m_triumph_order > 0)
            m_triumph_order--;
        if (m_wonder_order > 0)
            m_wonder_order--;
    }
    unk_004CB56C();
}

// The faction's part of the end of a round: its finances, its armies' tiles and its settlements, then its plague tick.
// Returns whether, as a major faction, it holds a huge city (level 5) other than Rome in a region with the italy hidden
// resource; that brings the Marian reforms (Campaign::unk_004537F2).
bool Faction::unk_004CAEDB()
{
    // NONMATCHING - the original works out the hidden resource's word and bit before it calls region()
    bool marian = false;
    unsigned int i;
    m_finance.end_turn();
    for (i = 0; i < m_armies.size(); i++)
        lbl_029219E0->unk_004AFB1E(m_armies.m_items[i]->unk_00719BB2());
    for (i = 0; i < m_settlements.size(); i++) {
        Settlement* settlement = m_settlements.m_items[i];
        settlement->unk_005B5F04();
        if (unk_00445B2B() && settlement->level() == 5 && settlement != lbl_018CD818->rome()) {
            int resource = lbl_014DA2B0.unk_008DB1B9(String("italy"));
            if (resource >= 0 && settlement->region()->has_resource(resource))
                marian = true;
        }
    }
    plague_tick();
    return marian;
}

extern "C" {
void fn_00CB3AAB(Faction* faction, DisasterReport* report);  // the turn's message about it (a guess)
}

// One plague tick of each of the faction's settlements, forts and characters (Settlement::plague_tick and
// Character::plague_tick). The garrisons of those still sick each lose 4-10% of their soldiers.
void Faction::plague_tick()
{
    // NONMATCHING - the stack frame is laid out differently
    DisasterReport report(DISASTER_PLAGUE, 0);
    unsigned int i;
    for (i = 0; i < m_settlements.size(); i++) {
        Settlement* settlement = m_settlements[i];
        settlement->plague_tick();
        if (settlement->has_plague()) {
            report.m_population_lost += settlement->plague_deaths();
            report.m_settlements.push_back(settlement);
            if (settlement->garrison())
                report.soldiers_lost(m_id) +=
                    settlement->garrison()->kill_soldiers(lbl_018CD818->m_random.range(4, 10));
        }
    }
    for (i = 0; i < m_forts.size(); i++) {
        Fort* fort = m_forts.m_items[i];
        fort->plague_tick();
        if (fort->has_plague()) {
            if (fort->garrison())
                report.soldiers_lost(m_id) +=
                    fort->garrison()->kill_soldiers(lbl_018CD818->m_random.range(4, 10));
        }
    }
    // The total starts again at each character without plague, so the report counts only the armies' losses after
    // the last of them.
    int soldiers_lost = 0;
    for (i = 0; i < m_characters.size(); i++) {
        Character* character = m_characters[i];
        character->plague_tick(&soldiers_lost);
        if (character->has_plague())
            report.m_characters.push_back(character);
        else
            soldiers_lost = 0;
    }
    report.soldiers_lost(m_id) += soldiers_lost;
    fn_00CB3AAB(this, &report);
}

extern "C" {
void fn_008B3EE1(CharacterRecord* leader);  // after a new leader is chosen (a message, when some flag allows)
void fn_008B41DA(CharacterRecord* heir);    // after a new heir is chosen (the same)
void fn_008B1C3F(CharacterRecord* spy, Settlement* settlement, int outcome);  // the message (a guess)
}

enum {
    SPY_EXECUTED = 0,
    SPY_EXPELLED = 3,
};

// Works out the factions whose regions border its own, and the foreign regions on its borders. m_14c is set instead when
// one of its own regions is a sea region; the check reads its own region, not the neighbour.
void Faction::unk_004CB391()
{
    // NONMATCHING - the original's two temporaries for FactionSet::add are the other way round
    if (m_178 == 0) {
        m_neighbours.m_bits = 0;
        m_bordering_regions.clear();
        m_14c = false;
        for (unsigned int i = 0; i < m_regions.size(); i++) {
            Region* region = lbl_018CD80C->region(m_regions[i]);
            for (unsigned int j = 0; j < region->m_neighbours.size(); j++) {
                if (region->m_neighbours[j].m_c)
                    continue;
                int other = region->m_neighbours[j].m_region;
                if (region->is_sea()) {
                    m_14c = true;
                    continue;
                }
                Faction* region_owner = lbl_018CD80C->region(other)->m_owner;
                if (!region_owner)
                    continue;
                if (region_owner != this) {
                    unsigned int k;
                    for (k = 0; k < m_bordering_regions.size(); k++) {
                        if (m_bordering_regions[k] == other)
                            break;
                    }
                    if (k == m_bordering_regions.size()) {
                        m_bordering_regions.push_back(other);
                        m_neighbours.add(region_owner->m_id);
                    }
                }
            }
        }
    }
}

// A foreign spy in one of the faction's settlements may be caught: the chance is 18% times the owner's strength over
// the spy's Subterfuge. A caught spy is always expelled; the test for executing him instead can never pass.
void Faction::catch_spy(Character* spy, float strength, Settlement* settlement)
{
    // NONMATCHING - the stack slots of chance and roll, and of the ratio and the odds, are swapped
    float ratio = strength / spy->record()->subterfuge();
    int chance = ratio * 0.18f * 100.0f;
    int roll = lbl_018CD818->m_random.percent();
    int outcome = 0;
    if (roll < chance) {
        float odds = (100 - chance) * 0.2f;
        if (roll >= 100.0f - odds)
            outcome = SPY_EXECUTED;
        else
            outcome = SPY_EXPELLED;
    } else {
        return;
    }
    switch (outcome) {
    case SPY_EXPELLED:
        spy->unk_005840D0(settlement, 1, 1, 1);
        unk_004C9616(spy);
        break;
    case SPY_EXECUTED:
        spy->set_death_cause(5);
        spy->kill();
        break;
    }
    int spy_faction = spy->unk_00580C1E()->id();
    lbl_018CD818->m_diplomacy.unk_005A1417(spy_faction, m_id, 0x1e);
    if (settlement)
        fn_008B1C3F(spy->record(), settlement, outcome);
}

extern "C" {
extern UnkC0A37E* lbl_029219DC;
extern bool lbl_010D2324;            // set by 0x0044B9ED
float fn_004100B6();                 // a global float (0x012CB9E0)
void fn_0054558A(FactionAI* ai);
}

// The start of the faction's turn: its characters' and settlements' turn starts, its neighbours worked out again, the
// AI's turn start, and the FactionTurnStart event.
void Faction::start_turn()
{
    lbl_029219DC->unk_00C0A6AF(this);
    m_170 = fn_004100B6();
    unsigned int i;
    for (i = 0; i < m_characters.size(); i++)
        m_characters[i]->unk_0057E5A1();
    unk_004D3E6F(0);
    for (i = 0; i < m_settlements.size(); i++)
        m_settlements[i]->start_turn();
    for (i = 0; i < m_forts.size(); i++)
        m_forts[i]->unk_0049A464();
    lbl_029219E0->unk_004A4C7F();
    m_finance.unk_004D8396(true, true);
    unk_004CB391();
    if (lbl_010D2324 && m_178 < 2 && m_ai) {
        m_ai->unk_10();
        fn_0054558A(m_ai);
    }
    m_4f8->unk_004DBCBD();
    fire_triggers(FactionTurnStartEvent(this));
}

extern "C" {
void fn_00CBB55B(Faction* faction);  // a message (a guess)
void fn_00CBB5FC(Faction* faction);  // a message (a guess)
}

// A dead faction (m_178 3) comes back with the leader, for example after a revolt: it gets a new AI, and the leader
// becomes its leader. romano_british gets m_4f4 when the leader isn't on a tile with an occupant.
void Faction::unk_004CDFCF(Character* leader)
{
    // NONMATCHING - the stack slots of the new AI's temporary and not_slavs differ
    if (m_178 == 3) {
        m_178 = 0;
        if (leader && !lbl_029219E0->unk_004A74BA(*(RoadPoint*)leader->position())) {
            int romano_british = fn_008F60D5(String("romano_british"))->m_id;
            if (romano_british == m_id)
                m_4f4 = true;
        }
#line 6070
        m_ai = new (THIS_FILE, __FILE__, __LINE__, true) FactionAI(this);
        m_ai->unk_1c();
        m_ai->unk_34()->unk_0050625A(m_c0, m_c4);
        if (leader) {
            CharacterRecord* current = unk_004CE1E1();
            if (!current) {
                leader->record()->m_flag17 = true;
                leader->record()->m_flag16 = true;
            }
            set_leader(leader->record());
        }
        if (m_description->has_shadowed_by()) {
            fn_00CBB55B(this);
        } else if (m_description->has_shadowing()) {
            fn_00CBB55B(this);
        } else {
            bool not_slavs = m_id != fn_008F60D5(String("slavs"))->m_id;
            if (not_slavs)
                fn_00CBB5FC(this);
        }
    }
}

extern "C" {
extern Campaign* lbl_02921AC0;
void fn_00CBB8EF(Faction* faction);  // a message: its shadow takes its place (a guess)
void fn_00CBB966(Faction* faction);  // a message: a shadow faction is gone (a guess)
void fn_00CBB889();                  // a message (type 0x9c) to every faction: one empire holds Rome and Constantinople
}

// What its listeners are told (not identified; nothing in it is set).
struct FactionGoneEvent {
    FactionGoneEvent() {}

    char m_pad0[4];
};

// It's gone (m_178 becomes 1). An AI faction whose shadowed_by faction is alive hands over to it instead, and keeps
// its m_178. A shadow faction (shadowing) hands what it has to the faction it shadows when that one's m_178 is 3; any
// other hands it to the slave faction, and with m_4f4 (romano_british) its armies first lose their units with attribute
// bit 7. Then its leader, or else its heir, loses the leader's traits, except that the heir's case takes them from the
// leader, who is null: a bug. Last, when it's one of the two empires and either of them holds both Rome and
// Constantinople, every faction is told.
void Faction::destroy()
{

    if (m_description->has_shadowed_by() && m_control == 0
        && lbl_02921AC0->unk_00454960(shadowed_by())->unk_178() == 0) {
        Faction* shadow = lbl_018CD818->faction(shadowed_by());
        shadow->m_4fc = true;
        shadow->m_listeners.unk_004D71CF(FactionGoneEvent());
        shadow->unk_004CE844(m_id, 10);
        unk_004CE839(shadowed_by());
        choose_leader();
        fn_00CBB8EF(this);
    } else {
        if (m_description->has_shadowing()) {
            if (!unk_4fc())
                fn_00CBB966(this);
            else
                m_4fc = false;
        }
        if (m_description->has_shadowing()
            && lbl_02921AC0->unk_00454960(shadowing())->unk_178() == 3) {
            m_178 = 1;
            m_listeners.unk_004D71CF(FactionGoneEvent());
            unk_004CE943(shadowing(), 1);
            unk_004CEA1C(shadowing(), 2, 6);
            unk_004CEAC8(shadowing(), 6);
        } else {
            if (m_4f4) {
                for (int i = m_armies.size() - 1; i >= 0; i--) {
                    Army* army = m_armies[i];
                    if (!army->unk_ac() && !army->unk_bc()) {
                        for (int j = army->num_units() - 1; j >= 0; j--) {
                            Unit* unit = army->m_units[j];
                            if (unit->unk_50()->m_attribute_7) {
                                if (army->num_units() == 1) {
                                    army->unk_0071E3F1(unit);
                                    i = m_armies.size();  // the army went with its last unit: start again
                                    break;
                                }
                                army->unk_0071E3F1(unit);
                            }
                        }
                    }
                }
            }
            m_178 = 1;
            m_listeners.unk_004D71CF(FactionGoneEvent());
            unk_004CE943(lbl_01181378, 1);
            unk_004CEA1C(lbl_01181378, 1, 6);
            unk_004CEAC8(lbl_01181378, 6);
        }
        set_capital(0);
        if (m_leader) {
            m_leader->remove_leader_traits();
            m_leader->m_leader = false;
            m_leader = 0;
        } else if (m_heir) {
            m_leader->remove_leader_traits();
            m_heir->m_heir = false;
            m_heir = 0;
        }
    }
    if (lbl_018CD818->constantinople()) {
        int west = fn_008F6278(String("empire_west"));
        int east = fn_008F6278(String("empire_east"));
        if (m_id == west || m_id == east) {
            if (lbl_018CD818->rome()->owner_id() == west && lbl_018CD818->constantinople()->owner_id() == west
                || lbl_018CD818->rome()->owner_id() == east && lbl_018CD818->constantinople()->owner_id() == east)
                fn_00CBB889();
        }
    }
}

void Faction::unk_004CE839(int faction)
{
}

// Only a record with m_flag16 can lead (a null one clears the leader). The old leader's CeasedFactionLeader comes first;
// the old record keeps its m_leader.
void Faction::set_leader(CharacterRecord* record)
{
    if (record && !record->m_flag16)
        return;
    if (m_leader)
        fire_triggers(CeasedFactionLeaderEvent(m_leader));
    m_leader = record;
    if (m_leader) {
        m_leader->m_leader = true;
        fire_triggers(BecomesFactionLeaderEvent(m_leader));
    }
}

// Makes the record the heir, or leaves the faction without one; a record outside the family tree is refused.
void Faction::set_heir(CharacterRecord* record)
{
    if (record && !record->m_flag16)
        return;
    if (m_heir)
        fire_triggers(CeasedFactionHeirEvent(m_heir));
    m_heir = record;
    if (m_heir) {
        m_heir->m_heir = true;
        fire_triggers(BecomesFactionHeirEvent(m_heir));
    }
}

void Faction::kill_leader()
{
    m_leader->m_character->kill();
}

inline int Character::influence()
{
    return m_record->influence();
}

// A new leader, after the old one is gone: the heir, who is replaced as heir, or else the family member on the map with
// the most Influence whose character isn't dead (the first of them on a tie, but the last while the best has 0).
void Faction::choose_leader()
{
    // NONMATCHING - the original gives the loop's size temporary a slot above the record's
    set_leader(0);
    if (m_heir) {
        set_leader(m_heir);
        choose_heir(0, false);
    } else {
        int best = 0;
        CharacterRecord* chosen = 0;
        for (unsigned int i = 0; i < m_characters.size(); i++) {
            CharacterRecord* record = m_characters.m_items[i]->m_record;
            if (record->m_flag16) {
                if (best == 0 || m_characters.m_items[i]->influence() > best) {
                    if (!m_characters.m_items[i]->unk_a0()) {
                        chosen = m_characters.m_items[i]->m_record;
                        best = m_characters.m_items[i]->influence();
                    }
                }
            }
        }
        if (chosen)
            set_leader(chosen);
        if (m_leader)
            choose_heir(0, false);
    }
    if (m_leader)
        fn_008B3EE1(m_leader);
}

// A new heir: the record, or else the family member on the map other than the leader with the most Influence whose
// character isn't dead (chosen as for the leader). With announce, the new heir is announced.
void Faction::choose_heir(CharacterRecord* record, bool announce)
{
    // NONMATCHING - the original gives the loop's size temporary a slot above the record's
    if (m_heir)
        m_heir->m_heir = false;
    set_heir(0);
    if (record) {
        set_heir(record);
    } else {
        int best = 0;
        CharacterRecord* chosen = 0;
        for (unsigned int i = 0; i < m_characters.size(); i++) {
            CharacterRecord* candidate = m_characters.m_items[i]->m_record;
            if (candidate->m_flag16 && !m_characters.m_items[i]->m_record->is_leader()) {
                if (best == 0 || m_characters.m_items[i]->influence() > best) {
                    if (!m_characters.m_items[i]->unk_a0()) {
                        chosen = m_characters.m_items[i]->m_record;
                        best = m_characters.m_items[i]->influence();
                    }
                }
            }
        }
        if (chosen)
            set_heir(chosen);
    }
    lbl_029219DC->unk_00C097F2(m_id);
    if (m_heir && announce)
        fn_008B41DA(m_heir);
}

unsigned int Faction::unk_004D0EC5(Array<Region*>& regions)
{
    // NONMATCHING - the original copies the region's index and the region into temporaries of their own
    for (unsigned int i = 0; i < num_regions(); i++) {
        int id = m_regions[i];
        regions.push_back(lbl_018CD80C->region(id));
    }
    return regions.size();
}

unsigned int Faction::get_regions(Array<Region*>& regions)
{
    // NONMATCHING - the original copies the region's index and the region into temporaries of their own
    for (unsigned int i = 0; i < num_regions(); i++) {
        int id = m_regions[i];
        regions.push_back(lbl_018CD80C->region(id));
    }
    return regions.size();
}

void Faction::add_mission(SenateMissionImpl* mission)
{
    m_missions.push_back(mission);
}

void Faction::add_offer(SenateMissionImpl* mission)
{
    m_offers.push_back(mission);
}

void Faction::accept_offer(SenateMissionImpl* mission)
{
    m_missions.push_back(mission);
    m_offers.erase(m_offers.find(mission));
}

void Faction::drop_offer(SenateMissionImpl* mission)
{
    m_offers.erase(m_offers.find(mission));
#line 7306
    CA_DELETE(mission);
}

void Faction::unk_004D10BD(Array<SenateMissionImpl*>& missions)
{
    for (Array<SenateMissionImpl*>::iterator it = m_missions.begin(); it != m_missions.end(); ++it) {
        if ((*it)->unk_18())
            missions.push_back(*it);
    }
}

// The senate counts it as a success or a failure, unless the senate's faction is out of the game.
void Faction::drop_mission(Array<SenateMissionImpl*>::iterator& it)
{
    // NONMATCHING - the original has one more stack slot, which it never uses, after erase's
    Faction* senate = lbl_018CD818->senate_faction();
    if (senate && senate->unk_178() < 2)
        lbl_018CD818->mission_finished(*it);
#line 7338
    CA_DELETE(*it);
    it = m_missions.erase(it);
}

// The missions that ran out fail (run_out gives the penalty). While the senate's faction is out of the game, they're
// all deleted, uncounted.
void Faction::end_missions()
{
    // NONMATCHING - the original has one more stack slot, which it never uses, after the loops' iterators
    Faction* senate = lbl_018CD818->senate_faction();
    if (!senate)
        return;
    Array<SenateMissionImpl*>::iterator it = m_missions.begin();
    if (senate->unk_178() >= 2) {
        while (it != m_missions.end()) {
#line 7359
            CA_DELETE(*it);
            it = m_missions.erase(it);
        }
    } else {
        while (it != m_missions.end()) {
            if ((*it)->run_out() == 0) {
                ++it;
            } else {
                if (lbl_018CD818->senate()->unk_44()->unk_14(m_id))
                    return;
                drop_mission(it);
            }
        }
    }
}

void Faction::drop_mission(SenateMissionImpl* mission)
{
    Array<SenateMissionImpl*>::iterator it = m_missions.begin();
    while (it != m_missions.end()) {
        if (*it == mission) {
            drop_mission(it);
            break;
        }
        ++it;
    }
}

void Faction::check_missions(AISenateFaction* senate)
{
    Array<SenateMissionImpl*>::iterator it = m_missions.begin();
    while (it != m_missions.end()) {
        if ((*it)->possible(senate))
            ++it;
        else
            drop_mission(it);
    }
}

// A save keeps each mission's builder, which builds the mission and deletes itself. Each mission then tells the
// faction of it again.
void Faction::build_missions()
{
    Array<SenateMissionImpl*>::iterator it = m_missions.begin();
    for (; it != m_missions.end(); ++it)
        *it = (*it)->build();
    for (it = m_offers.begin(); it != m_offers.end(); ++it)
        *it = (*it)->build();
    for (it = m_missions.begin(); it != m_missions.end(); ++it)
        (*it)->unk_00();
}

// A random number from 0 to N.
inline int random_index(int n)
{
    return lbl_018CD818->m_random.up_to(n);
}

// Unit types by their cost in a custom battle, cheapest first.
struct CustomCostOrder {
    bool operator()(int a, int b) const
    {
        return lbl_01589640.m_types[a].m_custom_cost < lbl_01589640.m_types[b].m_custom_cost;
    }
};

// A heap sort (DATA from 1 while it works).
template <class T, class Order>
void sift_down(T* heap, unsigned int i, unsigned int count, const Order& order, T& swap)
{
    unsigned int child;
    while ((child = i * 2) <= count) {
        if (child != count && order(heap[child], heap[child + 1]))
            child++;
        if (!order(heap[i], heap[child]))
            return;
        swap = heap[child];
        heap[child] = heap[i];
        heap[i] = swap;
        i = child;
    }
}

template <class T, class Order>
void heap_sort(T* data, unsigned int count, const Order& order, T& swap)
{
    data--;
    for (unsigned int i = count / 2; i > 1; i--)
        sift_down(data, i, count, order, swap);
    while (count > 1) {
        sift_down(data, 1, count, order, swap);
        swap = data[1];
        data[1] = data[count];
        data[count] = swap;
        count--;
    }
}

template <class T, class Order>
inline void sort(T* data, unsigned int count, const Order& order)
{
    T swap;
    heap_sort(data, count, order, swap);
}

// Each unit type its settlements' buildings can train, once. It returns how many there are.
unsigned int Faction::buildable_units(Array<int>& units)
{
    // NONMATCHING - the original has end()'s temporaries and find's result in other stack slots
    BuildingEffects effects;
    unsigned int types = 0;
    for (PointerList<Settlement>::iterator it = m_settlements.begin(); it != m_settlements.end(); ++it) {
        Settlement* settlement = *it;
        settlement->unk_005BCA50(&effects);
        unsigned int num_recruits = effects.num_recruits();
        for (unsigned int i = 0; i < num_recruits; i++) {
            BuildingRecruit item;
            effects.recruit(&item, i);
            BuildingRecruit* recruit = &item;
            if (recruit->m_experience.total() >= 0) {
                int type = recruit->m_unit;
                if (units.end() == units.find(type)) {
                    units.push_back(type);
                    types++;
                }
            }
        }
    }
    CustomCostOrder order;
    sort(&units[0], types, order);
    return types;
}

// COUNT new units of the type, at its capital: a ship in the capital's port, or else the first port of its
// settlements' that has m_3c; any other unit in the capital's garrison, made if need be, or a new army beside the
// capital when the garrison is full. It stops when a unit can't be made.
void Faction::give_units(int type, int count, RewardUnit* unit)
{
    for (unsigned int i = 0; i < count; i++) {
        unit->m_unit = lbl_01589640.unk_0091AEAA(m_capital->region(), type, m_id, 0, -1);
        if (!unit->m_unit)
            return;
        unit->m_place = m_capital;
        if (unit->m_unit->category() == 4) {
            SettlementUnk184* port = unit->m_place->m_184;
            PointerList<Settlement>::iterator it = PointerList<Settlement>::iterator(m_settlements.begin());
            while ((!port || !port->unk_3c()) && it != m_settlements.end()) {
                unit->m_place = *it;
                port = unit->m_place->m_184;
                ++it;
            }
            port->unk_0049E4D1(unit->m_unit);
            continue;
        }
        if (m_capital->has_room(true)) {
            if (!m_capital->unk_40()) {
#line 7646
                Army* army = new (THIS_FILE, __FILE__, __LINE__, true) Army(0, 0);
                m_capital->unk_84(army);
                unk_004CA5BA(army);
                lbl_018CD80C->unk_0048204D(army, lbl_018CD80C->tile(*m_capital->position())->m_region);
            }
            m_capital->unk_40()->unk_0071D66C(unit->m_unit, 0);
        } else {
            TilePosition position;
            bool found = m_capital->unk_005BA4CA(&position, 0, 0, 0);
            Army* army = unk_004C9439(&position, 0);
            army->unk_0071D66C(unit->m_unit, 0);
            army->unk_00721E6B(&position);
        }
    }
}

// Takes out the general units (general_unit in export_descr_unit.txt).
void remove_general_units(Array<int>& units)
{
    Array<int>::iterator it = units.begin();
    while (it != units.end()) {
        UnitUnk50* type = &lbl_01589640.m_types[*it];
        if (type->m_general_unit)
            it = units.erase(it);
        else
            ++it;
    }
}

// The senate's units (code\senate_mission.cpp's rewards): COUNT units of a type it can build, of a fixed type, or of
// one of a fixed list at random, each list looked up in export_descr_unit.txt once.
// The count is buildable_units' from before the general units are taken out, so with general units buildable, the
// picks can land past the list's end, where erase leaves its old items.
void Faction::give_buildable_units(int count, RewardUnit* unit)
{
    Array<int> units;
    unsigned int found = buildable_units(units);
    remove_general_units(units);
    if (found > 0) {
        int i = lbl_018CD818->m_random.range(found / 2, found - 1);
        give_units(units[i], count, unit);
    }
}

void Faction::give_best_buildable_units(int count, RewardUnit* unit)
{
    Array<int> units;
    unsigned int found = buildable_units(units);
    remove_general_units(units);
    if (found > 0)
        give_units(units[found - 1], count, unit);
}

void Faction::give_triarii(int count, RewardUnit* unit)
{
    String name("roman triarii");
    give_units(lbl_01589640.find(name), count, unit);
}

void Faction::give_minor_exotic_units(int count, RewardUnit* unit)
{
    static int s_minor_exotic[7] = {
        lbl_01589640.find(String("roman velite gladiator")),
        lbl_01589640.find(String("merc rhodian slingers")),
        lbl_01589640.find(String("merc greek hoplites")),
        lbl_01589640.find(String("merc barbarian infantry")),
        lbl_01589640.find(String("merc numidian cavalry")),
        lbl_01589640.find(String("merc barbarian cavalry")),
        lbl_01589640.find(String("roman scorpion")),
    };
    give_units(s_minor_exotic[random_index(6)], count, unit);
}

// The first cohorts are given only once the Marian reforms have happened.
void Faction::give_major_exotic_units(int count, RewardUnit* unit)
{
    static int s_major_exotic[10] = {
        lbl_01589640.find(String("roman arcani")),
        lbl_01589640.find(String("roman samnite gladiator")),
        lbl_01589640.find(String("roman mirmillo gladiator")),
        lbl_01589640.find(String("merc bastarnae")),
        lbl_01589640.find(String("merc elephants")),
        lbl_01589640.find(String("merc horse archers")),
        lbl_01589640.find(String("merc sarmatian cavalry")),
        lbl_01589640.find(String("naval deceres")),
        lbl_01589640.find(String("roman legionary first cohort i")),
        lbl_01589640.find(String("roman legionary first cohort ii")),
    };
    if (lbl_018CD818->marian_reforms())
        give_units(s_major_exotic[random_index(9)], count, unit);
    else
        give_units(s_major_exotic[random_index(7)], count, unit);
}

// The senate's gift (SenateFamily::check_gift): a "roman legionary first cohort i" or "ii", 50-50, once the Marian
// reforms have happened.
void Faction::give_first_cohort()
{
    if (!lbl_018CD818->marian_reforms())
        return;
    RewardUnit unit;
    if (lbl_018CD818->m_random.percent() > 50) {
        String name("roman legionary first cohort i");
        give_units(lbl_01589640.find(name), 1, &unit);
    } else {
        String name("roman legionary first cohort ii");
        give_units(lbl_01589640.find(name), 1, &unit);
    }
}

// A suitor offered for an unmarried daughter's hand (0x10 bytes).
class MarriageOffer : public SenateMissionAnswer {
public:
    MarriageOffer(CharacterRecord* suitor, CharacterRecord* bride)
        : m_answered(false), m_accepted(false), m_suitor(suitor), m_bride(bride) {}
    virtual ~MarriageOffer() {}
    virtual void accepted();            // 0x004D4AF4
    virtual void declined();            // 0x004D4B75
    virtual void* accept_text() { return 0; }
    virtual void* decline_text() { return 0; }

    bool m_answered;                    // +0x4
    bool m_accepted;                    // +0x5
    CharacterRecord* m_suitor;          // +0x8
    CharacterRecord* m_bride;           // +0xc
};

// A man offered to a family man for adoption (0x14 bytes).
class AdoptionOffer : public SenateMissionAnswer {
public:
    AdoptionOffer(CharacterRecord* adoptee, CharacterRecord* father)
        : m_answered(false), m_accepted(false), m_adoptee(adoptee), m_father(father), m_general(0) {}
    virtual ~AdoptionOffer() {}
    virtual void accepted();            // 0x004D49C0
    virtual void declined();            // 0x004D4A77
    virtual void* accept_text() { return 0; }
    virtual void* decline_text() { return 0; }

    bool m_answered;                    // +0x4
    bool m_accepted;                    // +0x5
    CharacterRecord* m_adoptee;         // +0x8
    CharacterRecord* m_father;          // +0xc
    Character* m_general;               // +0x10: a lesser general adopted in the adoptee's place, when he isn't on the map
};

extern "C" {
void fn_00CB21E9(CharacterRecord* bride, CharacterRecord* suitor, MarriageOffer* offer);      // the offer's message
void fn_00CB23B0(CharacterRecord* father, CharacterRecord* adoptee, AdoptionOffer* offer);    // the offer's message
void fn_00CB2478(CharacterRecord* record);      // the offered man has died (a guess)
void fn_00CB15FA(Character* character);         // the wedding's message (a guess)
}

// Once in 4 turns, when no marriage offer waits, a suitor is offered for an unmarried daughter; then, when neither
// offer waits, a man is offered for adoption when the family is small for its land: always under 0.6
// (family_per_region), and on a percent() under 20 under 0.8.
void FactionUnk88C::make_offers()
{
    int roll = lbl_018CD818->m_random.up_to(3);
    if (roll == 0 && m_marriage == 0) {
        CharacterRecord* bride = this->bride();
        if (bride) {
            CharacterRecord* suitor = bride->create_suitor();
#line 8167
            m_marriage = NEW MarriageOffer(suitor, bride);
            fn_00CB21E9(bride, suitor, m_marriage);
        }
    }
    if (m_adoption == 0 && m_marriage == 0) {
        bool offer = false;
        float family = family_per_region();
        if (family < 0.6f)
            offer = true;
        else if (family < 0.8f && lbl_018CD818->m_random.percent() < 20)
            offer = true;
        else
            offer = false;
        if (offer == true) {
            CharacterRecord* father = adopting_father(true, false, 0);
            if (father) {
                CharacterRecord* adoptee = father->create_adoptee();
#line 8200
                m_adoption = NEW AdoptionOffer(adoptee, father);
                fn_00CB23B0(father, adoptee, m_adoption);
            }
        }
    }
}

// The faction's family for the size of its land: the living family-tree members who are men of age on the map as
// named characters, boys of 14 or more, or unmarried women of 12 or more, over 0.75 per region held + 1.
float FactionUnk88C::family_per_region()
{
    float divisor = 0.75f * m_faction->m_regions.size() + 1.0f;
    int count = 0;
    for (unsigned int i = 0; i < m_faction->m_records.size(); i++) {
        CharacterRecord* kin = m_faction->m_records[i];
        if (kin->m_alive && kin->m_flag16) {
            if (kin->m_male) {
                if (!kin->m_flag2 && kin->m_character != 0 && kin->m_character->type() == 5)
                    count++;
                else if (kin->m_flag2 && kin->m_age >= 14)
                    count++;
            } else if (kin->m_age >= 12 && kin->m_spouse == 0) {
                count++;
            }
        }
    }
    return (float)count / divisor;
}

// The children the family's men could still have: 4 less the children of each living family-tree man of 16 or more on
// the map as a named character with fewer than 4.
unsigned int FactionUnk88C::child_slots()
{
    unsigned int slots = 0;
    for (unsigned int i = 0; i < m_faction->m_characters.size(); i++) {
        if (m_faction->m_characters[i]->type() == 5) {
            CharacterRecord* record = m_faction->m_characters[i]->record();
            if (record->m_alive && record->m_male && record->m_flag16 && record->m_age >= 16
                && record->m_num_children < 4)
                slots += 4 - record->m_num_children;
        }
    }
    return slots;
}

// The adoption offer is done with.
void FactionUnk88C::end_adoption_offer()
{
    if (m_adoption) {
#line 8315
        CA_DELETE(m_adoption);
        m_adoption = 0;
    }
}

// The marriage offer is done with.
void FactionUnk88C::end_marriage_offer()
{
    if (m_marriage) {
#line 8324
        CA_DELETE(m_marriage);
        m_marriage = 0;
    }
}

// The first of the faction's living unmarried women of 12 to 50 who is in one of its settlements, when that
// settlement isn't besieged and has room (Settlement::unk_005BA392).
CharacterRecord* FactionUnk88C::bride()
{
    for (unsigned int i = 0; i < m_faction->m_records.size(); i++) {
        CharacterRecord* record = m_faction->m_records[i];
        if (record->m_alive && !record->m_male && record->m_spouse == 0 && record->m_age >= 12 && record->m_age <= 50) {
            Settlement* settlement = m_faction->unk_004C44F0(record);
            if (settlement && !settlement->besieged() && settlement->unk_005BA392())
                return record;
        }
    }
    return 0;
}

// The first living family-tree man of 16 or more on the map as a named character, other than OTHER, with fewer than 4
// children; with OLDER, only one younger than OTHER, and with IN_SETTLEMENT, only one in a settlement that isn't
// besieged and has room.
CharacterRecord* FactionUnk88C::adopting_father(bool in_settlement, bool older, CharacterRecord* other)
{
    for (unsigned int i = 0; i < m_faction->m_characters.size(); i++) {
        if (m_faction->m_characters[i]->type() == 5) {
            CharacterRecord* record = m_faction->m_characters[i]->m_record;
            if ((!other || other != record) && record->m_alive && record->m_male && record->m_flag16
                && record->m_age >= 16 && (!older || (other && other->m_age - record->m_age > 0))
                && record->m_num_children < 4) {
                Settlement* settlement = m_faction->unk_004C44F0(record);
                if (!in_settlement || (settlement && !settlement->besieged() && settlement->unk_005BA392()))
                    return record;
            }
        }
    }
    return 0;
}

// The lesser general is adopted in the adoptee's place; LesserGeneralOfferedForAdoption fires for an AI faction.
void FactionUnk88C::offer_lesser_general(Character* general, CharacterRecord* adoptee)
{
    general->unk_0057F7A5(adoptee);
    if (m_faction->control() == 0)
        fire_triggers(LesserGeneralOfferedForAdoptionEvent(general->record()));
}

// Accepted: the adoptee (living, or with death cause 9) is adopted, or the lesser general in his place when he isn't
// on the map; one who has died gets fn_00CB2478's message.
void AdoptionOffer::accepted()
{
    m_accepted = true;
    m_answered = true;
    if (m_adoptee->m_alive || m_adoptee->m_1c0 == 9) {
        if (m_adoptee->m_character == 0 && m_general) {
            m_father->m_faction->m_88c.offer_lesser_general(m_general, m_adoptee);
            m_adoptee->destroy();
        } else {
            m_adoptee->adopt(m_father);
        }
    } else {
        fn_00CB2478(m_adoptee);
    }
    m_father->m_faction->m_88c.end_adoption_offer();
}

// Declined: the adoptee is destroyed when he isn't on the map.
void AdoptionOffer::declined()
{
    m_answered = true;
    if (m_adoptee && m_adoptee->m_character == 0)
        m_adoptee->destroy();
    m_father->m_faction->m_88c.end_adoption_offer();
}

// Accepted: the suitor marries the bride, when he's still alive.
void MarriageOffer::accepted()
{
    m_accepted = true;
    m_answered = true;
    if (m_suitor->m_alive) {
        m_suitor->marry(m_bride);
        if (m_suitor->m_character)
            fn_00CB15FA(m_suitor->m_character);
    } else {
        fn_00CB2478(m_suitor);
    }
    m_bride->m_faction->m_88c.end_marriage_offer();
}

// Declined: the suitor is destroyed.
void MarriageOffer::declined()
{
    m_answered = true;
    m_suitor->destroy();
    m_bride->m_faction->m_88c.end_marriage_offer();
}

// The Marian reforms come: the bodyguards of its named characters are upgraded, when it owns a general_unit_upgrade unit.
void Faction::marian_reforms()
{
    // NONMATCHING - the original keeps the character's CharacterSpyingInfo pointer in a temporary for the virtual call
    for (unsigned int i = 0; i < m_characters.size(); i++) {
        if (m_characters.m_items[i]->type() == 5 && lbl_01589640.has_general_upgrade(m_id))
            m_characters.m_items[i]->marian_reforms();
    }
}

// The character's Subterfuge, 0 to 10: what each spy adds to the unrest of a foreign settlement it's in.
int CharacterRecord::subterfuge()
{
    return m_attributes[ATTR_SUBTERFUGE] > 0
        ? ((unsigned int)m_attributes[ATTR_SUBTERFUGE] < 10 ? m_attributes[ATTR_SUBTERFUGE] : 10)
        : 0;
}

// The character's Influence, 0 to 10.
int CharacterRecord::influence()
{
    return m_attributes[ATTR_INFLUENCE] > 0
        ? ((unsigned int)m_attributes[ATTR_INFLUENCE] < 10 ? m_attributes[ATTR_INFLUENCE] : 10)
        : 0;
}

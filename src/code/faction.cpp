#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\faction.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A faction's campaign record: its turn update.
#include "unsorted.h"

#include "character.h"
#include "character_record.h"

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
            m_88c.unk_004D2ACF();
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

class UnkC0A6AF {
public:
    void unk_00C0A6AF(Faction* faction);  // 0x00C0A6AF
};

extern "C" {
extern UnkC0A6AF* lbl_029219DC;
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
        unk_004CED7E();
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
            m_heir->m_flag4 = false;
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

// The Marian reforms come: the bodyguards of its named characters are upgraded, when it owns a general_unit_upgrade unit.
void Faction::marian_reforms()
{
    // NONMATCHING - the original keeps the character's CharacterKind pointer in a temporary for the virtual call
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

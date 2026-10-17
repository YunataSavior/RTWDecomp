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
        for (i = 0; i < m_f4.size(); i++)
            m_f4.m_items[i]->unk_00719E61();
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

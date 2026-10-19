#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\character.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A character on the strategy map.
#include "character.h"

extern "C" {
void fn_00CB09C7(CharacterRecord* record);  // the message that it died of plague (a guess)
}

// A named character's bodyguard is upgraded when its faction owns a general_unit_upgrade unit.
void Character::marian_reforms()
{
    if (m_80->m_type == 5) {
        int faction = m_7c->m_faction->m_id;
        if (lbl_01589640.has_general_upgrade(faction))
            unk_00581268();
    }
}

inline PathFinder* paths()
{
    PathFinder* paths = lbl_02921A1C;
    return paths;
}

// The character enters a settlement: it moves onto the settlement's tile, becomes the governor if it would make a better
// one (Settlement::better_governor), and catches plague from the settlement, or gives the settlement its own.
void Character::enter_settlement(Settlement* settlement)
{
    unk_00581E22();
    unk_005877ED(0);
    float cost = paths()->step_cost(*(RoadPoint*)position(), *(RoadPoint*)settlement->position());
    unk_00581638(cost, 1);
    unk_00588989(*settlement->position());
    if (led_army())
        unk_00580F8F();
    set_settlement(settlement);
    unk_02921A38()->unk_00C09FDE(this);
    unk_02921A18()->unk_004BE7CB(settlement);
    unk_02921A38()->unk_00C0A4F4(UnkB5DC7E(settlement));
    paths()->unk_004AC808(this);
    if (settlement->better_governor(this))
        settlement->set_governor(this);
    if (settlement->has_plague())
        catch_plague();
    if (m_plague)
        settlement->catch_plague();
}

// The same for a fort, without the governor.
void Character::enter_fort(Fort* fort)
{
    unk_00581E22();
    unk_005877ED(0);
    float cost = paths()->step_cost(*(RoadPoint*)position(), *(RoadPoint*)fort->position());
    unk_00581638(cost, 1);
    unk_00588989(*fort->position());
    if (led_army())
        unk_00580F8F();
    set_settlement((Settlement*)fort);
    unk_02921A38()->unk_00C09FDE(this);
    unk_02921A18()->unk_004BE81C(fort);
    unk_02921A38()->unk_00C0A592(UnkB5C51D(fort));
    paths()->unk_004AC808(this);
    if (fort->has_plague())
        catch_plague();
    if (m_plague)
        fort->catch_plague();
}

// Gives the character plague, if it hasn't got it: its plague ticks start again, and CharacterDamagedByDisaster fires.
void Character::catch_plague()
{
    if (!has_plague()) {
        m_plague_ticks = 0;
        m_plague = true;
        fire_triggers(CharacterDamagedByDisasterEvent(m_record, DISASTER_PLAGUE));
    }
}

// One plague tick of a sick character. From the fifth, it recovers, unless the settlement it's in still has plague;
// until then, it dies of plague with a chance of 1 in 20 each tick, and the army it leads loses 4-10% of its soldiers
// (added to soldiers_lost). Returns whether it died.
bool Character::plague_tick(int* soldiers_lost)
{
    bool died = false;
    if (has_plague()) {
        m_plague_ticks++;
        bool recovered = false;
        if (m_plague_ticks >= 5) {
            if (m_settlement) {
                if (!m_settlement->has_plague())
                    recovered = true;
            } else {
                recovered = true;
            }
        }
        if (recovered) {
            m_plague = false;
            m_plague_ticks = 0;
            return false;
        }
        if (m_58.random()->range(0, 19) == 0) {
            set_death_cause(7);
            kill();
            if (m_80->m_type == 5)
                m_7c->m_faction->m_898.set_14(21);
            died = true;
            fn_00CB09C7(m_record);
        }
        if (led_army())
            *soldiers_lost += led_army()->kill_soldiers(m_58.random()->range(4, 10));
    }
    return died;
}

// The settlement it's in. Plague passes both ways: when either the character or the settlement has it, both catch it.
void Character::set_settlement(Settlement* settlement)
{
    m_settlement = settlement;
    if (!m_settlement)
        return;
    if (m_settlement && (m_settlement->has_plague() || m_plague)) {
        m_settlement->catch_plague();
        catch_plague();
    }
}

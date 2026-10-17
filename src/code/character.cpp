#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\character.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A character on the strategy map.
#include "character.h"

extern "C" {
void fn_00CB09C7(CharacterRecord* record);  // the message that it died of plague (a guess)
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

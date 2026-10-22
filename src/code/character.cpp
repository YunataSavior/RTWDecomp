#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\character.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A character on the strategy map.
#include "character.h"
#include "campaign_triggers.h"
#include "event_manager.h"
#include "fort.h"
#include "settlement.h"
#include "strategy_map.h"
#include "unit_db.h"
#include "world.h"
#include "world_map.h"
#include "character_record.h"
#include "secure_int.h"

extern "C" {
void fn_00CB09C7(CharacterRecord* record);  // the message that it died of plague (a guess)
void fn_008B3858(CharacterRecord* record);
void fn_008B2BAF(CharacterRecord* child);   // after a child is born
void fn_008B352F(CharacterRecord* husband, CharacterRecord* wife);  // after a marriage
CharacterRecord* fn_008E2CE8(Faction* faction, int first_name, int surname, int age, bool male, int, int);
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

inline int max_int(const int& a, const int& b)
{
    return a < b ? b : a;
}

inline int min_int(const int& a, const int& b)
{
    return a < b ? a : b;
}

inline int clamp_int(const int& value, const int& low, const int& high)
{
    return min_int(max_int(value, low), high);
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

inline int Character::attribute(int id)
{
    int index = id;
    return m_record->attribute_value(index);
}

inline int Character::famine_threat()
{
    return unk_00586C48()->m_famine_threat;
}

// At the end of its faction's turn. A character over 60 dies of old age when range(60, 127) comes out over 122. A
// family member on the map as a named character may then have a child, and an unmarried one may marry.
// The chance of a child starts at 8% for each of 4 - round(3 x family_per_region()), at least 1% and at most 100%;
// each point of Fertility adds a third of it, and it's halved when the family has 2 child slots or fewer. It's
// rolled first, with percent() (0 to 100) under it; then he must have a wife of 50 or less, fewer than 4 children,
// and no child born within the last 2 years. The child is a boy or a girl, 50-50, of age 0.
// An unmarried man marries with a chance of 1 in 21 (1 in 11 for the leader or the heir), a wife of his age -30 to
// +10, from 12 to 50. Last, CharacterDamagedByDisaster fires for famine when its region has a famine threat of 2 or more.
void Character::unk_0057DD3B()
{
    // NONMATCHING - the original has one more unused compiler temporary, so its slots below -0xb0 differ
    reset_movement();
    if (m_record->m_age > 60 && m_58.random()->range(60, 127) > 122) {
        set_death_cause(1);
        kill();
        if (m_80->m_type == 5)
            m_7c->m_faction->m_898.set_14(21);
        fn_008B3858(record());
        if (record() == m_7c->m_faction->leader()) {
            unk_005821A4();
            m_7c->m_faction->unk_004CED7E();
        }
        if (m_record == m_7c->m_faction->heir()) {
            m_record->m_flag4 = false;
            m_7c->m_faction->unk_004CECB3(0);
            m_7c->m_faction->unk_004CEEEA(0, 1);
        }
    }
    if (m_80->m_type == 5 && m_record->m_flag16 && !m_7c->m_faction->is_unk_a7()) {
        float family = m_7c->m_faction->m_88c.family_per_region();
        float odds = 8.0f;
        odds *= 4 - float_to_int(3.0f * family);
        odds = clamp_float(odds, 1.0f, 100.0f);
        odds *= attribute(31) / 3.0f + 1.0f;   // Fertility
        unsigned int free_slots = m_7c->m_faction->m_88c.child_slots();
        if (free_slots <= 2)
            odds *= 0.5f;
        odds = clamp_float(odds, 1.0f, 100.0f);
        int rolled = m_58.random()->percent();
        if (rolled < odds && m_record->m_spouse && m_record->m_spouse->m_age <= 50 && m_record->m_num_children < 4) {
            bool spaced = true;
            for (unsigned int i = 0; i < m_record->m_num_children; i++) {
                if (campaign()->m_date.year() - m_record->m_children[i]->m_birth_date.m_year < 2) {
                    spaced = false;
                    break;
                }
            }
            if (spaced) {
                bool male = m_58.random()->up_to(1) == 0;
                int given_name = 0;
                int last_name = 0;
                fn_008FF5CB(m_58.random(), m_7c->m_faction->id(), male, &given_name, &last_name);
                CharacterRecord* child = fn_008E2CE8(m_7c->m_faction, given_name, last_name,
                    SecureInt2(Scrambled(0x756b1143)).value(), male, 1, 0);    // age 0
                child->m_flag16 = true;
                if (m_7c->m_faction->id() == m_record->m_name_faction)
                    child->take_surname(m_record, 5);
                m_record->m_children[m_record->m_num_children++] = child;
                child->m_father = m_record;
                fn_008B2BAF(child);
            }
        }
        if (!m_record->m_spouse && m_record->m_male
            && m_58.random()->up_to((m_record->is_leader() || m_record->is_heir() ? -10 : 0) + 20) == 0) {
            int her_age = m_record->m_age + m_58.random()->range(-30, 10);
            her_age = clamp_int(her_age, 12, 50);
            int given_name = 0;
            int surname = 0;
            fn_008FF5CB(m_58.random(), m_7c->m_faction->id(), false, &given_name, &surname);
            CharacterRecord* wife = fn_008E2CE8(m_7c->m_faction, given_name, surname, her_age, false, 1, 0);
            wife->m_flag16 = true;
            wife->m_flag2 = false;
            m_record->m_spouse = wife;
            wife->m_spouse = m_record;
            fn_008B352F(m_record, wife);
        }
    }
    unk_00581619(unk_005815D7());
    m_b8 = -1;
    if (famine_threat() >= 2)
        fire_triggers(CharacterDamagedByDisasterEvent(m_record, DISASTER_FAMINE));
}

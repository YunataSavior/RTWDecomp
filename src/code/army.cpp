#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\army.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// An army of the campaign map: its units.
#include "unsorted.h"
#include "character.h"

// 0 while it's in a battle with m_139 set.
unsigned int Army::strength()
{
    if (m_129 && m_139)
        return 0;
    return m_strength;
}

void Army::update_strength()
{
    if (m_139)
        m_strength = 0;
    else
        m_strength = compute_strength();
}

// In a battle a unit counts its battle value, and nothing once it's out of the fight or in state 13; on the campaign map
// it counts its campaign value.
unsigned int Army::compute_strength()
{
    float strength = 0;
    for (unsigned int i = 0; i < num_units(); i++) {
        Unit* unit = m_units[i];
        if (m_129) {
            if (unit->unk_007C8444() && unit->unk_60() != 13)
                strength += fn_00432C9F(m_units[i]);
        } else {
            strength += fn_004335A9(m_units[i]);
        }
    }
    return float_to_int(strength);
}

// Each unit loses that share of its soldiers, and a unit left with none and no m_298 slots filled is removed.
// Returns how many died.
int Army::kill_soldiers(int percent)
{
    int killed = 0;
    for (int i = num_units() - 1; i >= 0; i--) {
        killed += m_units[i]->kill_soldiers(percent);
        if (m_units[i]->num_soldiers() == 0 && !m_units[i]->unk_007B9438())
            remove_unit(m_units[i]);
    }
    return killed;
}

// The soldiers a settlement's public order counts for its garrison: all of them, except that peasants count half.
int Army::garrison_soldiers()
{
    int soldiers = 0;
    for (unsigned int i = 0; i < num_units(); i++) {
        if (m_units[i]->unk_50()->m_is_peasant)
            soldiers += m_units[i]->num_soldiers() / 2;
        else
            soldiers += m_units[i]->num_soldiers();
    }
    return soldiers;
}

// Every soldier, peasants included, and the filled m_298 slots.
int Army::unk_00722430()
{
    int soldiers = 0;
    for (unsigned int i = 0; i < num_units(); i++) {
        soldiers += m_units[i]->num_soldiers();
        soldiers += m_units[i]->unk_007B9438();
    }
    return soldiers;
}

// The settlement it's in updates its mechanics (unless the unit has attribute 7), and outside some battles (see
// Campaign::unk_00454978) the unit's +0x40 is cleared.
void Army::unk_0071E2C8(Unit* unit)
{
    if (unit->unk_44())
        unit->unk_44()->unk_08(unit);
    for (unsigned int i = 0; i < m_units.size(); i++) {
        if (unit == m_units[i]) {
            m_units.erase(i);
            break;
        }
    }
    update_strength();
    if (unk_11c()) {
        Settlement* settlement = unk_11c()->settlement();
        if (settlement && !unit->unk_50()->m_attribute_7)
            settlement->unk_005B9837(unit);
    }
    if (!campaign()->unk_00454978())
        unit->m_40 = 0;
    if (leader())
        leader()->reset_movement();
}

// Its bodyguard's character dies with the unit.
// A command unit (the attribute of export_descr_unit.txt) lost while the army is in the field gives the first army of the
// battle's other side the eagle.
void Army::remove_unit(Unit* unit)
{
    Army* enemy;
    if (unit->unk_278() && !unit->unk_278()->unk_a0())
        unit->unk_278()->kill();
    if (m_in_battle && m_faction->unk_00445B2B() && leader() && leader()->in_field() &&
        unit->unk_50()->m_command) {
        int side = lbl_018BF5E0->side_of(this);
        enemy = lbl_018BF5E0->unk_005FF3D9(side == 0)->first_army();
        enemy->m_captured_eagle = true;
        if (campaign()->unk_00454978())
            fn_00CB67A8(m_faction);
    }
    if (unit->unk_48()) {
        unit->unk_48()->unk_005BBDAA(unit, 0, 1);
        unk_02921A38()->unk_00C0A2EE(unit);
    }
    unk_0071E2C8(unit);
#line 2810
    CA_DELETE(unit);
}

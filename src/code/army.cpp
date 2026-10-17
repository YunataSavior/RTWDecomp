#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\army.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// An army of the campaign map: its units.
#include "unsorted.h"

// Each unit loses that share of its soldiers, and a unit left with none and no m_298 slots filled is removed.
// Returns how many died.
int Army::kill_soldiers(int percent)
{
    int killed = 0;
    for (int i = num_units() - 1; i >= 0; i--) {
        killed += m_units[i]->kill_soldiers(percent);
        if (m_units[i]->num_soldiers() == 0 && !m_units[i]->unk_007B9438())
            unk_0071E460(m_units[i]);
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

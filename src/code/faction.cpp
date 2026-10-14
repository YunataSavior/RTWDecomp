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

#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\faction_economics.cpp"
// (The original's path: its THIS_FILE comes between faction.cpp's and faction_render.cpp's, though nothing asserts.)
static char THIS_FILE[] = __FILE__;

// A faction's finances: its treasury and income.
#include "unsorted.h"

#include "ai/ai_cheat_manager.h"

// Adds the turn's income to the treasury, and for an AI faction its cash from the difficulty: ai_turn_cash for each
// settlement, plus 0 to 100.
void FactionFinance::end_turn()
{
    if (m_faction->unk_178() != 0)
        return;
    m_treasury += m_income;
    int cash = AICheatManager::ai_turn_cash(m_faction->id());
    if (cash != 0)
        m_treasury += cash * m_faction->m_settlements.size() + lbl_018CD818->m_random.range(0, 100);
    m_history.current().unk_004D7EAA();
}

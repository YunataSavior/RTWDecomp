#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\Ai\\ai_global_strategy_director.cpp"
static char THIS_FILE[] = __FILE__;

// A faction's AI's overall strategy. (The original's path, which its __FILE__s give.)
#include "ai_global_strategy_director.h"
#include "../horde.h"
#include "../romans_cmd_line.h"
#include "ai_faction_base.h"
#include "ai_resource_allocation_manager.h"

inline void AIGlobalStrategyDirector::make_horde_controller()
{
    if (m_ai->resources())
        m_ai->resources()->unk_00525B64();
    unk_0050B2A2();
#line 223
    m_horde = NEW AIHordeController(m_ai);
}

// A horde faction's AI has a horde controller. Under -ai, so does a faction that has no settlements yet could become a
// horde, when its m_4f8's faction is none.
void AIGlobalStrategyDirector::update_horde()
{
    bool horde = m_faction->unk_4f4();
    if (lbl_012CA766 && !horde && m_faction->num_settlements() == 0 && m_faction->unk_004D43D5()
        && m_faction->unk_4f8()->unk_4() == 21)
        horde = true;
    if (horde) {
        if (!m_horde)
            make_horde_controller();
    } else if (m_horde) {
#line 257
        CA_DELETE(m_horde);
        m_horde = 0;
    }
}

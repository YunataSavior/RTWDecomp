#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\settlement_state_inspection.cpp"
// The settlements' trigger conditions. (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"
#include "settlement_base.h"
#include "triggers.h"

extern "C" bool fn_0088F69C(bool value, bool target, int test);    // a Condition's test of a bool (a guess)

// The SettlementHasPlague condition: the event's settlement has plague (SettlementBase::m_plague).
class SettlementHasPlague {
public:
    virtual bool test(const TriggerEvent& event);   // 0x0089DA2B

    int m_test;                         // +0x4: how the value is tested (the condition's "not", a guess)
};

bool SettlementHasPlague::test(const TriggerEvent& event)
{
    // NONMATCHING - the original loads ecx before eax for the second settlement() call
    if (event.settlement() == 0)
        return false;
    SettlementBase* settlement = (SettlementBase*)event.settlement();
    bool sick = settlement->m_plague;
    return fn_0088F69C(sick, true, m_test);
}

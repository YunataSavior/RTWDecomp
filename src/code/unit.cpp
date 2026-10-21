#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\unit.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A unit of an army.
#include "unsorted.h"
#include "unit.h"

int Unit::unk_007B9438()
{
    int count = 0;
    for (unsigned int i = 0; i < 4; i++)
        if (m_298[i])
            count++;
    return count;
}

// At 100% or more every soldier dies, but a unit with m_278 set keeps 1 (and the return still counts it).
// Otherwise floor(percent * soldiers / 100) die, but never the last one of a unit with m_278 set.
int Unit::kill_soldiers(int percent)
{
    unsigned int killed = 0;
    if ((unsigned int)percent >= 100) {
        killed = m_num_soldiers;
        m_num_soldiers = m_278 != 0;
        return killed;
    }
    killed = percent * m_num_soldiers / 100;
    if (m_278 && killed && m_num_soldiers == killed)
        killed--;
    m_num_soldiers -= killed;
    return killed;
}

// False in states 18 to 21, or with neither m_288 nor m_2b8 left.
bool Unit::unk_007C8444()
{
    if (unk_60() == 18)
        return false;
    if (unk_60() == 20)
        return false;
    if (unk_60() == 19)
        return false;
    if (unk_60() == 21)
        return false;
    if (m_288 + m_2b8 == 0)
        return false;
    return true;
}

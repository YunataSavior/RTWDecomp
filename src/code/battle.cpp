#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\battle.cpp"
// (The original's path, which its __FILE__s give.)

#include "battle.h"
#include "romans_cmd_line.h"

extern "C" int fn_005F5D4F(float speed)
{
    if (lbl_012CA768)
        return 3;
    if (speed == 0.0f)
        return 0;
    if (speed < 1.5f)
        return 1;
    if (speed < 2.5f)
        return 2;
    return 3;
}

extern "C" float fn_005F5D9D(int index)
{
    if (lbl_012CA768)
        return 6.0f;
    switch (index) {
    case 0:
        return 0.0f;
    case 1:
        return 1.0f;
    case 2:
        return 2.0f;
    }
    return 6.0f;
}

int Battle::speed_index()
{
    if (paused())
        return 0;
    return fn_005F5D4F(speed());
}

void Battle::set_speed(int player, float speed)
{
    if (lbl_012CA768) {
        m_speed = 6.0f;
        return;
    }
    if (m_num_players == 0) {
        m_speed = speed;
        return;
    }
    bool any = false;
    float slowest = 1.0f;
    unsigned int i;
    for (i = 0; i < m_num_players; i++) {
        if (m_players[i].m_id == player)
            m_players[i].m_speed = speed;
        if (m_players[i].m_active) {
            if (!any || slowest > m_players[i].m_speed) {
                any = true;
                slowest = m_players[i].m_speed;
            }
        }
    }
    m_speed = slowest;
}

void Battle::set_all_speeds(float speed)
{
    if (lbl_012CA768) {
        m_speed = 6.0f;
        return;
    }
    for (unsigned int i = 0; i < m_num_players; i++)
        m_players[i].m_speed = speed;
    m_speed = speed;
}

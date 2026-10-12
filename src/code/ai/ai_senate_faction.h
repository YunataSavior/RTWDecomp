#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\ai\\ai_senate_faction.h"
// The senate's missions (code\ai\ai_senate_faction.h; the original's path, which its __FILE__ gives). Its line
// numbers are the original's.
#ifndef AI_SENATE_FACTION_H
#define AI_SENATE_FACTION_H

#include "unsorted.h"

// A mission the senate gives (not identified).
class SenateMission {
public:
    ~SenateMission();
};

// A character's senate mission, if it has one (16 bytes; not identified).
struct SenateMissionSlot {
    SenateMissionSlot() : m_0(-1), m_c(0) {}
    ~SenateMissionSlot()
    {
        if (m_c)
#line 123
            CA_DELETE(m_c);
    }

    int m_0;
    Date m_date;
    SenateMission* m_c;
};

#endif

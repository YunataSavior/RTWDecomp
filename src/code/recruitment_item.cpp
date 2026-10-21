#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\recruitment_item.cpp"
// (The original's path: its THIS_FILE comes between culture.cpp's and settlement.cpp's, though nothing asserts.)
static char THIS_FILE[] = __FILE__;

// A unit in a settlement's recruitment queue, or one it could recruit.
#include <math.h>
#include "unsorted.h"
#include "recruitment_item.h"
#include "settlement.h"
#include "unit_db.h"
#include "world.h"

extern "C" int lbl_010FDF20;            // bestbuy's percentage: 100, or 90

template <class T>
inline T ca_max(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <class T>
inline T ca_min(const T& a, const T& b)
{
    return a < b ? a : b;
}

template <class T>
inline void ca_clamp(T& value, const T& low, const T& high)
{
    value = ca_min(ca_max(value, low), high);
}

inline unsigned char round_up(float value)
{
    return float_to_int(ceilf(value));
}

RecruitmentItem::RecruitmentItem()
{
    m_settlement = 0;
    m_0c = 0;
    m_0d = 0;
    m_experience = 0;
    m_armour = 0;
    m_weapon = 0;
    m_11 = 0;
    m_12 = 0;
    m_13 = 0;
    m_turns = 0;
    m_cost = 0;
    m_soldiers = 0;
    m_1c = 0;
}

void RecruitmentItem::reset()
{
    *this = RecruitmentItem();
    m_turn = campaign()->turn();
}

// The unit percent% of the way to a full one: its turns rounded up and its cost scaled (and cut by bestbuy), each at
// least 1 and at most the full unit's.
void RecruitmentItem::unk_005AE4F0(Settlement* settlement, UnitUnk50* type, unsigned char experience,
                                   unsigned char armour, unsigned char weapon, int percent)
{
    // NONMATCHING - two of the stack slots of the inlined rounding are swapped
    reset();
    m_settlement = settlement;
    m_1c = 1;
    m_experience = experience;
    m_armour = armour;
    m_weapon = weapon;
    m_11 = 0xff;
    m_0d = 0;
    m_type = type;
    m_13 = 0;
    m_0c = 0;
    m_12 = 0;
    m_turns = round_up((float)(type->m_turns * percent) / 100.0f);
    ca_clamp(m_turns, (unsigned char)1, (unsigned char)type->m_turns);
    m_cost = type->m_cost * percent * lbl_010FDF20 / 10000;
    ca_clamp(m_cost, (unsigned short)1, type->m_cost);
    m_soldiers = type->soldiers() + type->m_154;
}

void RecruitmentItem::unk_005AE6C3(Settlement* settlement, UnitUnk50* type, unsigned char experience,
                                   unsigned char armour, unsigned char weapon, int percent)
{
    // NONMATCHING - two of the stack slots of the inlined rounding are swapped
    reset();
    m_settlement = settlement;
    m_1c = 1;
    m_experience = experience;
    m_armour = armour;
    m_weapon = weapon;
    m_11 = 0xff;
    m_0d = 1;
    m_type = type;
    m_13 = 0;
    m_0c = 0;
    m_12 = 0;
    m_turns = round_up((float)(type->m_turns * percent) / 100.0f);
    ca_clamp(m_turns, (unsigned char)1, (unsigned char)type->m_turns);
    m_cost = type->m_cost * percent * lbl_010FDF20 / 10000;
    ca_clamp(m_cost, (unsigned short)1, type->m_cost);
    m_soldiers = type->soldiers() + type->m_154;
}

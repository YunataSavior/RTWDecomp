#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\garrison.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// A settlement's garrison and the characters inside it.
#include "unsorted.h"
#include "character_record.h"

// The character's Management, 0 to 10.
int CharacterRecord::management()
{
    return m_attributes[ATTR_MANAGEMENT] > 0
        ? ((unsigned int)m_attributes[ATTR_MANAGEMENT] < 10 ? m_attributes[ATTR_MANAGEMENT] : 10)
        : 0;
}

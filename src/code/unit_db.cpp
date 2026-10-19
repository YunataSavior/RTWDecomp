#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\unit_db.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// The unit types of export_descr_unit.txt.
#include "unsorted.h"

// Whether the faction owns a unit type with general_unit_upgrade.
bool UnitDB::has_general_upgrade(int faction)
{
    for (int i = 0; i < count(); i++) {
        UnitUnk50* type = &m_types[i];
        if (type->m_general_unit_upgrade && type->owned_by(faction))
            return true;
    }
    return false;
}

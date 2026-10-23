#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\localisation\\string_table_manager.cpp"
// (The original's path, which its __FILE__s give.)

#include "string_table_manager.h"


// Text INDEX of battle.txt.
const UniString* battle_text(int index)
{
    return string_table(BATTLE_TABLE)->text(index);
}

// Text INDEX of shared.txt.
const UniString* shared_text(int index)
{
    return string_table(SHARED_TABLE)->text(index);
}

// Text INDEX of strat.txt.
const UniString* strat_text(int index)
{
    return string_table(STRAT_TABLE)->text(index);
}

// Text INDEX of battle_ed.txt.
const UniString* battle_ed_text(int index)
{
    return string_table(BATTLE_ED_TABLE)->text(index);
}

// Text INDEX of diplomacy.txt.
const UniString* diplomacy_text(int index)
{
    return string_table(DIPLOMACY_TABLE)->text(index);
}

// Text INDEX of tooltips.txt.
const UniString* tooltips_text(int index)
{
    return string_table(TOOLTIPS_TABLE)->text(index);
}


// Table INDEX.
StringTable* table_at(int index)
{
    return string_table(index);
}

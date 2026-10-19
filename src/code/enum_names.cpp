#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\enum_names.cpp"
// (The original's file has no asserts, so this name is a guess. The run of code from 0xE30A8F holds the
// name lookups for many of the game's enums.)
static char THIS_FILE[] = __FILE__;

#include <string.h>

#pragma intrinsic(strlen)

extern "C" {
bool fn_00E2DDF0(const char* text, const char* word);  // whether text starts with word, then a space or the end
int fn_00E3C474(const char* name);                     // a culture from its name (7 for none)
}

// A character attribute from its name in export_descr_character_traits.txt and
// export_descr_ancillaries.txt, or 55 for none.
// "Negotiation" gives Influence, so no name gives 27. Combat_V_<culture> gives 34 + the culture, and an unknown
// culture gives 41, the same as Combat_V_Slave.
extern "C" int fn_00E41B13(const char* name)
{
    if (fn_00E2DDF0("Command", name)) return 0;
    if (fn_00E2DDF0("Influence", name)) return 1;
    if (fn_00E2DDF0("Management", name)) return 2;
    if (fn_00E2DDF0("Subterfuge", name)) return 3;
    if (fn_00E2DDF0("Loyalty", name)) return 4;
    if (fn_00E2DDF0("TroopMorale", name)) return 5;
    if (fn_00E2DDF0("MovementPoints", name)) return 6;
    if (fn_00E2DDF0("Attack", name)) return 7;
    if (fn_00E2DDF0("Defence", name)) return 8;
    if (fn_00E2DDF0("SiegeAttack", name)) return 9;
    if (fn_00E2DDF0("SiegeDefence", name)) return 10;
    if (fn_00E2DDF0("Ambush", name)) return 11;
    if (fn_00E2DDF0("NavalCommand", name)) return 12;
    if (fn_00E2DDF0("SiegeEngineering", name)) return 13;
    if (fn_00E2DDF0("NightBattle", name)) return 14;
    if (fn_00E2DDF0("PersonalSecurity", name)) return 15;
    if (fn_00E2DDF0("PublicSecurity", name)) return 16;
    if (fn_00E2DDF0("Bribery", name)) return 17;
    if (fn_00E2DDF0("BribeResistance", name)) return 18;
    if (fn_00E2DDF0("Electability", name)) return 19;
    if (fn_00E2DDF0("LineOfSight", name)) return 20;
    if (fn_00E2DDF0("TrainingUnits", name)) return 21;
    if (fn_00E2DDF0("TrainingAgents", name)) return 22;
    if (fn_00E2DDF0("Construction", name)) return 23;
    if (fn_00E2DDF0("Trading", name)) return 24;
    if (fn_00E2DDF0("LocalPopularity", name)) return 25;
    if (fn_00E2DDF0("FootInTheDoor", name)) return 26;
    if (fn_00E2DDF0("Negotiation", name)) return 1;
    if (fn_00E2DDF0("Farming", name)) return 28;
    if (fn_00E2DDF0("Mining", name)) return 29;
    if (fn_00E2DDF0("TaxCollection", name)) return 30;
    if (fn_00E2DDF0("Fertility", name)) return 31;
    if (fn_00E2DDF0("CavalryCommand", name)) return 32;
    if (fn_00E2DDF0("InfantryCommand", name)) return 33;
    if (fn_00E2DDF0("GrainTrading", name)) return 42;
    if (fn_00E2DDF0("SlaveTrading", name)) return 43;
    if (fn_00E2DDF0("Health", name)) return 44;
    if (fn_00E2DDF0("Squalor", name)) return 45;
    if (fn_00E2DDF0("Unrest", name)) return 46;
    if (fn_00E2DDF0("Law", name)) return 47;
    if (fn_00E2DDF0("Looting", name)) return 48;
    if (fn_00E2DDF0("SenateStanding", name)) return 49;
    if (fn_00E2DDF0("PopularStanding", name)) return 50;
    if (fn_00E2DDF0("BodyguardValour", name)) return 51;
    if (fn_00E2DDF0("HitPoints", name)) return 52;
    if (fn_00E2DDF0("TrainingAnimalUnits", name)) return 53;
    if (fn_00E2DDF0("BattleSurgery", name)) return 54;
    if (fn_00E2DDF0("Combat_V_Slave", name)) return 41;
    const char* prefix = "Combat_V_";
    if (strncmp(prefix, name, strlen(prefix)) == 0) {
        const char* culture_name = name;
        culture_name += strlen(prefix);
        int culture = fn_00E3C474(culture_name);
        return culture + 34;
    }
    return 55;
}

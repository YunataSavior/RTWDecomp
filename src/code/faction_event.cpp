#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\faction_event.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// The faction events: the records of what happened to a faction, made while lbl_014D6058 is set.
#include "unsorted.h"
#include "character.h"
#include "character_record.h"

// What every record is built on (a virtual base, of 0xc bytes).
class FactionEvent {
public:
    static void* operator new(unsigned int size);   // 0x008B7881
    static void operator delete(void* p);           // 0x008B788F
    virtual void unk_00();

    char m_pad4[8];
};

// A foreign spy caught in a settlement (0x30 bytes).
class SpyCaughtFactionEvent : public virtual FactionEvent {
public:
    SpyCaughtFactionEvent(SettlementBase* settlement);  // 0x008B1CAA

    char m_pad4[0x24 - 4];
};

// A settlement's revolt (0x48 bytes).
class CityRebelsFactionEvent : public virtual FactionEvent {
public:
    CityRebelsFactionEvent(Settlement* settlement, Faction* faction);  // 0x008B48A7

    char m_pad4[0x3c - 4];
};

extern "C" {
extern bool lbl_014D6058;                   // whether faction events are recorded (a guess)
void fn_00CBC87A(Faction* owner, Character* spy, int outcome);  // the message (a guess)
void fn_00CB09C7(CharacterRecord* spy);

// A foreign spy is caught in the settlement: a message (outcome 0 also runs fn_00CB09C7 on the spy), and the record.
void fn_008B1C3F(CharacterRecord* spy, Settlement* settlement, int outcome)
{
    fn_00CBC87A(settlement->unk_9c(), spy->m_character, outcome);
    if (outcome == 0)
        fn_00CB09C7(spy);
    if (lbl_014D6058)
        new SpyCaughtFactionEvent(settlement);
}

// The settlement revolts: the GovernorCityRebels event when it has a governor, then CityRebels, and the record.
void fn_008B4820(Settlement* settlement, Faction* faction, Character* governor)
{
    if (governor)
        fire_triggers(GovernorCityRebelsEvent(governor->record(), settlement, faction));
    fire_triggers(CityRebelsEvent(settlement, faction));
    if (lbl_014D6058)
        new CityRebelsFactionEvent(settlement, faction);
}
}

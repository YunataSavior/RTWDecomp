// The settlements' trigger events (code\settlement_triggers.cpp).
#ifndef SETTLEMENT_TRIGGERS_H
#define SETTLEMENT_TRIGGERS_H

#include "unsorted.h"

// The SettlementTurnEnd event.
class SettlementTurnEndEvent : public TriggerEvent {
public:
    SettlementTurnEndEvent(Settlement* settlement);  // 0x008A8465
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Character* m_governor;
};

// The SettlementTurnStart event, at the start of the owner's turn.

class SettlementTurnStartEvent : public TriggerEvent {
public:
    SettlementTurnStartEvent(Settlement* settlement);  // 0x008A8396
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
};

// The CityRebels event.
class CityRebelsEvent : public TriggerEvent {
public:
    CityRebelsEvent(Settlement* settlement, Faction* faction);  // 0x008A9319
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Faction* m_faction;
};

// The GovernorCityRebels event.
class GovernorCityRebelsEvent : public TriggerEvent {
public:
    GovernorCityRebelsEvent(CharacterRecord* governor, Settlement* settlement, Faction* faction);  // 0x008A9400
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_governor;
    Settlement* m_settlement;
    Faction* m_faction;
};

// The CityRiots event.
class CityRiotsEvent : public TriggerEvent {
public:
    CityRiotsEvent(Settlement* settlement, Faction* faction);  // 0x008A9133
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Faction* m_faction;
};

// The GovernorCityRiots event.
class GovernorCityRiotsEvent : public TriggerEvent {
public:
    GovernorCityRiotsEvent(CharacterRecord* governor, Settlement* settlement, Faction* faction);  // 0x008A921A
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_governor;
    Settlement* m_settlement;
    Faction* m_faction;
};

#endif

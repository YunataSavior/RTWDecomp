// The campaign's trigger events (code\campaign_triggers.cpp).
#ifndef CAMPAIGN_TRIGGERS_H
#define CAMPAIGN_TRIGGERS_H

#include "unsorted.h"

// An event for the triggers (not identified; code\landmark.cpp makes one from a port's +0xa98 when a wonder is
// captured).
class UnkMessage : public TriggerEvent {
public:
    UnkMessage(int);                    // 0x008A4067
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    int m_4;
};

// The OfferedForAdoption event.
class UnkMessage8A4586 : public UnkMessage {
public:
    UnkMessage8A4586(CharacterRecord* record);  // 0x008A4586
};

// The senate's penalties' events, for the faction leader.
class SenateExposureEvent : public TriggerEvent {
public:
    SenateExposureEvent(CharacterRecord* leader);  // 0x008A58EE
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

class QuaestorInvestigationMinorEvent : public TriggerEvent {
public:
    QuaestorInvestigationMinorEvent(CharacterRecord* leader);  // 0x008A59BD
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

class QuaestorInvestigationEvent : public TriggerEvent {
public:
    QuaestorInvestigationEvent(CharacterRecord* leader);  // 0x008A5A8C
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

class QuaestorInvestigationMajorEvent : public TriggerEvent {
public:
    QuaestorInvestigationMajorEvent(CharacterRecord* leader);  // 0x008A5B5B
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

// The CharacterDamagedByDisaster event.
class CharacterDamagedByDisasterEvent : public TriggerEvent {
public:
    CharacterDamagedByDisasterEvent(CharacterRecord* record, int disaster);  // 0x008A43D0
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
    int m_disaster;
};

// The CeasedFactionLeader event.
class CeasedFactionLeaderEvent : public TriggerEvent {
public:
    CeasedFactionLeaderEvent(CharacterRecord* record);  // 0x008A4F3A
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
};

// The BecomesFactionLeader event.
class BecomesFactionLeaderEvent : public TriggerEvent {
public:
    BecomesFactionLeaderEvent(CharacterRecord* record);  // 0x008A48C2
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
};

// The FactionTurnStart event.
class FactionTurnStartEvent : public TriggerEvent {
public:
    FactionTurnStartEvent(Faction* faction);  // 0x008A3B75
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Faction* m_faction;
};

#endif

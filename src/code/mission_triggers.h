// The senate missions' trigger events (code\mission_triggers.cpp).
#ifndef MISSION_TRIGGERS_H
#define MISSION_TRIGGERS_H

#include "unsorted.h"

class LeaderSenateMissionSuccessEvent : public TriggerEvent {
public:
    LeaderSenateMissionSuccessEvent(CharacterRecord* leader);  // 0x008A7CCE
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

class LeaderSenateMissionFailedEvent : public TriggerEvent {
public:
    LeaderSenateMissionFailedEvent(CharacterRecord* leader);  // 0x008A7D9D
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

#endif

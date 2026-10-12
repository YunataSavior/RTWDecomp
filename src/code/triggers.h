// The triggers: what export_descr_character_traits.txt, export_descr_ancillaries.txt and export_descr_advice.txt
// say happens on an event (code\triggers.cpp).
#ifndef TRIGGERS_H
#define TRIGGERS_H

#include "unsorted.h"

#include "core_linked_list.h"
#include "ca_auto_ptr.h"
#include "hash_map.h"
#include "character_traits_db.h"
#include "character_ancillaries_db.h"
#include "character_record.h"
#include "character.h"

// A trigger's Affects line: a trait, the points it gives (or takes), and the chance in percent.
struct TraitAffect {
    TraitAffect() : m_trait(0), m_points(0), m_chance(0) {}

    TraitDefinition* m_trait;
    int m_points;
    int m_chance;
};

struct TraitAffects : public FixedArray<TraitAffect, 10> {
    TraitAffects() { m_count = 0; }

    unsigned int m_count;
};

// A trigger's AdviceThread line: an advice thread, and how much the trigger adds to it.
struct AdviceThreadRef {
    String m_name;
    short m_increment;
};

struct AdviceThreads : public FixedArray<AdviceThreadRef, 5> {
    AdviceThreads() { m_count = 0; }

    unsigned int m_count;
};

// A trigger's AcquireAncillary line: an ancillary and the chance in percent.
struct AncillaryChance {
    AncillaryChance() : m_ancillary(0), m_chance(0) {}

    AncillaryDescription* m_ancillary;
    int m_chance;
};

// An advice thread (code\advice_db.cpp).
class AdviceThread {
public:
    unsigned char priority();                                           // 0x008CB978 (a guess)
    void increment(const TriggerEvent& event, short amount, bool);      // 0x008CBA12
};

AdviceThread* find_advice_thread(const String& name);                  // 0x008CBEEE

class Condition;

// A trigger's Condition lines, all of which must hold (code\game_state_inspection.cpp). It owns them until it is
// copied: the copy takes them.
class ConditionList {
public:
    ConditionList() : m_owns(false) {}
    ConditionList(ScriptReader& reader);                                // 0x00898F8F
    ConditionList(const ConditionList& other);                          // 0x0089901A
    ~ConditionList();                                                   // 0x00899099
    ConditionList& operator=(const ConditionList& other);               // 0x00899048
    bool test(const TriggerEvent& event);                               // 0x00899123

    bool m_owns;
    LinkedList<Condition*> m_conditions;
};

// A trigger (0x30 bytes).
class Trigger {
public:
    Trigger(const char* name, const ConditionList& conditions, const TraitAffects& affects,
            const AdviceThreads& advice, const AncillaryChance& ancillary);
    bool less_than(const Trigger& other) const;
    Trigger& operator=(const Trigger& other);
    bool test(const TriggerEvent& event);
    const char* name();
    TraitAffects* affects();
    AdviceThreads* advice();
    AncillaryChance* ancillary();
    bool passed();
    void reset();

    static HashMap<String, Trigger*, StringHash, StringEqual> s_by_name;

    String m_name;                              // +0x00
    ConditionList m_conditions;                 // +0x08
    ca_auto_ptr<TraitAffects> m_affects;        // +0x14: 0 when it has no Affects
    ca_auto_ptr<AdviceThreads> m_advice;        // +0x1c
    ca_auto_ptr<AncillaryChance> m_ancillary;   // +0x24
    bool m_tested;                              // +0x2c
    bool m_result;                              // +0x2d
};

bool read_triggers(ScriptReader& reader);       // 0x008AA428
Trigger* find_trigger(const char* name);        // 0x008AABCA
void init_triggers();                           // 0x008AB5C9
void free_triggers();                           // 0x008AB5CE

// What a settlement or fort event is about (not identified: a StrategyObject with more virtual functions).
class TriggerPlace : public StrategyObject {
public:
    virtual void unk_74();
    virtual void unk_78();
    virtual void unk_7c();
    virtual void unk_80();
    virtual void unk_84();
    virtual void unk_88();
    virtual int unk_8c();
    virtual void unk_90();
    virtual void unk_94();
    virtual void unk_98();
    virtual Faction* faction();                 // +0x9c
};

// The events about a character, for the UI (CharacterPanelOpen's): they give no traits or ancillaries.
class UiCharacterEvent : public TriggerEvent {
public:
    virtual CharacterRecord* unk_18() const;
    virtual Faction* faction() const;
    virtual int unk_38() const;
    virtual int character_type() const;

    CharacterRecord* m_record;
};

// The events about a character.
class CharacterEvent : public TriggerEvent {
public:
    virtual CharacterRecord* character_record() const;
    virtual CharacterRecord* unk_18() const;
    virtual Faction* faction() const;
    virtual int unk_38() const;
    virtual int character_type() const;

    CharacterRecord* m_record;
};

// The events about a settlement.
class SettlementEvent : public TriggerEvent {
public:
    virtual Faction* faction() const;
    virtual TriggerPlace* settlement() const;
    virtual int unk_38() const;

    TriggerPlace* m_settlement;
};

// The events about a fort (a guess: UngarrisonedFort's).
class FortEvent : public TriggerEvent {
public:
    virtual Faction* faction() const;
    virtual TriggerPlace* fort() const;
    virtual int unk_38() const;

    TriggerPlace* m_fort;
};

// The events about a faction.
class FactionEvent : public TriggerEvent {
public:
    virtual Faction* faction() const;

    Faction* m_faction;
};

// An event with nothing in it, to test conditions outside events (code\advice_db.cpp and the console make them).
class EmptyEvent : public TriggerEvent {
public:
    virtual bool unk_74() const;
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    static LinkedList<Trigger> s_triggers;
};

// Puts a kind of event's triggers under its name, for WhenToTest (each kind of event has a static one).
class EventRegistrar {
public:
    EventRegistrar(const char* name, LinkedList<Trigger>* triggers);
};

#endif

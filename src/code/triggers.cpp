#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\triggers.cpp"
// The triggers: each kind of event's list of them, read from the Trigger blocks of the trait, ancillary and advice
// files, and what they do when their event happens. (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include <stdio.h>

#include "triggers.h"

#include "unsorted.h"
#include "faction.h"
#include "romans_utils.h"
#include "world.h"
#include "map_constants.h"
#include "unk_id.h"

extern "C" {
AncillaryDescription* fn_008DF89D(const char* name);   // find_ancillary (code\character_ancillaries_db.cpp)
void fn_008AFE7E();                                     // (code\advisor.cpp)
}

// The players (not identified).
struct GameSettings4 {
    bool unk_bool() const { return m_45 || m_74; }      // (a guess: a multiplayer game)

    char m_pad0[0x45];
    bool m_45;
    char m_pad46[0x74 - 0x46];
    bool m_74;
};

class PlayerSlots {
public:
    FactionSet human_factions();                        // 0x00415C25
};

class GameSettings {
public:
    char m_pad0[4];
    GameSettings4 m_4;                                  // +0x04
    char m_pad79[0xe4 - 0x79];
    PlayerSlots m_e4;                                   // +0xe4
};

extern "C" GameSettings* lbl_018CD828;

static String s_unused_key("unused_hash");
static String s_deleted_key("deleted_hash");

Trigger::Trigger(const char* name, const ConditionList& conditions, const TraitAffects& affects,
                 const AdviceThreads& advice, const AncillaryChance& ancillary)
    : m_name(name), m_conditions(conditions),
#line 104
      m_affects(affects.m_count == 0 ? 0 : NEW TraitAffects(affects)),
      m_advice(advice.m_count == 0 ? 0 : NEW AdviceThreads(advice)),
      m_ancillary(ancillary.m_ancillary == 0 ? 0 : NEW AncillaryChance(ancillary)), m_tested(false), m_result(false)
{
}

bool Trigger::less_than(const Trigger& other) const
{
    if (!m_advice.get())
        return false;
    if (!other.m_advice.get())
        return true;
    const AdviceThreads& advice = *m_advice;
    unsigned char highest_priority = 0;
    unsigned int i;
    for (i = 0; i < advice.m_count; i++) {
        unsigned char priority = find_advice_thread(advice.m_items[i].m_name)->priority();
        highest_priority = highest_priority < priority ? priority : highest_priority;
    }
    const AdviceThreads& other_advice = *other.m_advice;
    unsigned char other_highest_priority = 0;
    unsigned int j;
    for (j = 0; j < other_advice.m_count; j++) {
        unsigned char priority = find_advice_thread(other_advice.m_items[j].m_name)->priority();
        other_highest_priority = other_highest_priority < priority ? priority : other_highest_priority;
    }
    return highest_priority < other_highest_priority;
}

Trigger& Trigger::operator=(const Trigger& other)
{
    if (m_name == other.m_name)
        return *this;
    m_name = other.m_name;
    m_conditions = other.m_conditions;
    m_affects = other.m_affects;
    m_advice = other.m_advice;
    m_ancillary = other.m_ancillary;
    m_tested = other.m_tested;
    m_result = other.m_result;
    return *this;
}

bool Trigger::test(const TriggerEvent& event)
{
    m_result = m_conditions.test(event);
    m_tested = true;
    return m_result;
}

const char* Trigger::name()
{
    return m_name.c_str();
}

TraitAffects* Trigger::affects()
{
    return m_affects.get();
}

AdviceThreads* Trigger::advice()
{
    return m_advice.get();
}

AncillaryChance* Trigger::ancillary()
{
    return m_ancillary.get();
}

bool Trigger::passed()
{
    return m_result && m_tested;
}

void Trigger::reset()
{
    m_result = false;
    m_tested = false;
}

static HashMap<TextKey, LinkedList<Trigger>*, TextHash, TextEqual>* s_event_types;

// Reads the Trigger blocks that follow. A trigger whose WhenToTest is not a kind of event is skipped.
bool read_triggers(ScriptReader& reader)
{
    while (!reader.at_end()) {
        while (!reader.match("Trigger"))
            reader.next_line();
        reader.skip_word();
        const char* text = reader.word();
        text_compare(text, "agents28");
        text_compare(text, "agents29");
        reader.next_line();
        if (!reader.match("WhenToTest")) {
            SCRIPT_ERROR(reader, "Unknown identifier for trigger(%s) when expecting WhenToTest.", text);
            return false;
        }
        reader.skip_word();
        const char* event_name = reader.word();
        LinkedList<Trigger>** trigger_list = s_event_types->find(event_name);
        if (trigger_list) {
            reader.next_line();
            ConditionList conditions;
            if (reader.match("Condition"))
                conditions = ConditionList(reader);
            if (!reader.match("Affects") && !reader.match("AdviceThread") && !reader.match("AcquireAncillary")) {
                SCRIPT_ERROR(reader,
                             "Unknown identifier for trigger(%s) when expecting an Affect, Advice or Ancillary.",
                             text);
                return false;
            }
            TraitAffects affects;
            while (reader.match("Affects")) {
                reader.skip_word();
                const char* trait_name = reader.word();
                reader.skip_word();
                int points = reader.read_int();
                reader.skip_word();
                reader.skip_word();
                int chance = reader.read_int();
                TraitAffect& entry = affects[affects.m_count++];
                entry.m_trait = find_trait(trait_name);
                entry.m_points = points;
                entry.m_chance = chance;
                reader.next_line();
            }
            AncillaryChance ancillary_to_acquire;
            if (reader.match("AcquireAncillary")) {
                reader.skip_word();
                const char* ancillary_name = reader.word();
                reader.skip_word();
                reader.skip_word();
                int chance = reader.read_int();
                AncillaryDescription* ancillary = fn_008DF89D(ancillary_name);
                if (!ancillary) {
                    SCRIPT_ERROR(reader, "ancillary description (%s) not recognised", ancillary_name);
                    return false;
                }
                ancillary_to_acquire.m_ancillary = ancillary;
                ancillary_to_acquire.m_chance = chance;
                reader.next_line();
            }
            AdviceThreads advice;
            while (reader.match("AdviceThread")) {
                reader.skip_word();
                String thread_name(reader.word());
                AdviceThread* existing = find_advice_thread(thread_name);
                if (!existing) {
                    SCRIPT_ERROR(reader, "Advice thread (%s) not recognised", thread_name.c_str());
                    return false;
                }
                reader.skip_word();
                if (!reader.at_number()) {
                    SCRIPT_ERROR(reader, "%s", "Advice thread increment not supplied");
                    return false;
                }
                short delta = reader.read_uint();
                advice[advice.m_count].m_name = thread_name;
                advice[advice.m_count++].m_increment = delta;
                reader.next_line();
            }
            String trigger_name(text);
            Trigger::s_by_name.insert(
                trigger_name,
                &(*trigger_list)->push_back(Trigger(text, conditions, affects, advice, ancillary_to_acquire))->m_data);
        }
    }
    return true;
}

Trigger* find_trigger(const char* name)
{
    String key;
    key.set_scratch(name);
    Trigger** result = Trigger::s_by_name.find(key);
    key.detach();
    return result ? *result : 0;
}

HashMap<String, Trigger*, StringHash, StringEqual> Trigger::s_by_name(s_unused_key, s_deleted_key, 200);

int TriggerEvent::unk_10() const
{
    return 0;
}

int TriggerEvent::unk_04() const
{
    return 0;
}

int TriggerEvent::unk_08() const
{
    return 0;
}

int TriggerEvent::unk_0c() const
{
    return 0;
}

CharacterRecord* TriggerEvent::character_record() const
{
    return 0;
}

CharacterRecord* TriggerEvent::unk_18() const
{
    return 0;
}

int TriggerEvent::unk_1c() const
{
    return 0;
}

TriggerPlace* TriggerEvent::settlement() const
{
    return 0;
}

TriggerPlace* TriggerEvent::fort() const
{
    return 0;
}

int TriggerEvent::unk_24() const
{
    return 0;
}

Faction* TriggerEvent::faction() const
{
    return 0;
}

int TriggerEvent::unk_30() const
{
    return 0;
}

int TriggerEvent::unk_34() const
{
    return 0;
}

int TriggerEvent::unk_38() const
{
    return -1;
}

int TriggerEvent::unk_3c() const
{
    return 0;
}

int TriggerEvent::unk_40() const
{
    return 0;
}

int TriggerEvent::unk_44() const
{
    return 0;
}

int TriggerEvent::unk_48() const
{
    return 0;
}

int TriggerEvent::unk_4c() const
{
    return 0;
}

int TriggerEvent::unk_50() const
{
    return 4;
}

static String s_no_string;

const String& TriggerEvent::unk_54() const
{
    return s_no_string;
}

int TriggerEvent::unk_58() const
{
    return 6;
}

int TriggerEvent::character_type() const
{
    return 6;
}

int TriggerEvent::unk_60() const
{
    return 0x2f;
}

int TriggerEvent::unk_64() const
{
    return 4;
}

int TriggerEvent::unk_68() const
{
    return 6;
}

int TriggerEvent::unk_6c() const
{
    return 0xa5;
}

bool TriggerEvent::unk_70() const
{
    return false;
}

bool TriggerEvent::unk_74() const
{
    return false;
}

static Trigger* s_current_trigger;

// What an ancillary's and a trait's chance (in percent) are multiplied by before the roll.
static const float k_ancillary_chance_scale = 1.0f;
static const float k_trait_chance_scale = 1.0f;

// Tells the triggers of an event. Each trigger whose conditions hold gives its advice, its ancillary and its trait
// points, each with its own roll of the chance.
void fire_triggers(const TriggerEvent& event)
{
    if (lbl_018CD828->m_4.unk_bool() && lbl_018CD828->m_e4.human_factions().count() > 1)
        return;
    lbl_018CD818->unk_004531EF(event);
    LinkedList<Trigger>* triggers = event.triggers();
    for (LinkedList<Trigger>::iterator it = triggers->head(); it != LinkedList<Trigger>::iterator(); ++it) {
        Trigger& trigger = *it;
        s_current_trigger = &trigger;
        if (trigger.test(event)) {
            if (trigger.advice() && lbl_018CD818->unk_b8() == 1) {
                AdviceThreads* advice = trigger.advice();
                unsigned int size = advice->m_count;
                AdviceThreadRef* items = advice->m_items;
                for (unsigned int i = 0; i < size; i++) {
                    AdviceThread* thread = find_advice_thread(items[i].m_name);
                    if (thread)
                        thread->increment(event, items[i].m_increment, true);
                }
            }
            if (trigger.ancillary()) {
                CharacterRecord* record = event.character_record();
                if (record->m_character && record->m_character->type() != 4) {
                    AncillaryChance* chance = trigger.ancillary();
                    AncillaryDescription* ancillary = chance->m_ancillary;
                    if (!ancillary->m_exclude_cultures.test(record->m_faction->culture())) {
                        int roll = lbl_018CD818->m_random.percent();
                        if ((float)roll <= k_ancillary_chance_scale * (float)chance->m_chance)
                            record->add_ancillary(ancillary);
                    }
                }
            }
            if (trigger.affects()) {
                CharacterRecord* record = event.character_record();
                if (record) {
                    TraitAffects* affects = trigger.affects();
                    for (unsigned int i = 0; i < affects->m_count; i++) {
                        TraitAffect& entry = affects->m_items[i];
                        TraitDefinition* trait = entry.m_trait;
                        bool allowed = false;
                        if (record->m_character)
                            allowed = trait->m_characters.test(record->m_character->type());
                        else
                            allowed = trait->m_characters.test(5);
                        if (allowed && !trait->m_exclude_cultures.test(record->m_faction->culture())) {
                            int roll = lbl_018CD818->m_random.percent();
                            if ((float)roll <= k_trait_chance_scale * (float)entry.m_chance)
                                record->add_trait_points(trait, affects->m_items[i].m_points, true);
                        }
                    }
                }
            }
        }
    }
    for (LinkedList<Trigger>::iterator it2 = triggers->head(); it2 != LinkedList<Trigger>::iterator(); ++it2)
        (*it2).reset();
    fn_008AFE7E();
}

bool EmptyEvent::unk_74() const
{
    return true;
}

LinkedList<Trigger>* EmptyEvent::triggers() const
{
    return &s_triggers;
}

static int s_empty_event_7c;

const void* EmptyEvent::unk_7c() const
{
    return &s_empty_event_7c;
}

LinkedList<Trigger> EmptyEvent::s_triggers;

CharacterRecord* UiCharacterEvent::unk_18() const
{
    return m_record;
}

Faction* UiCharacterEvent::faction() const
{
    return m_record->m_faction;
}

int UiCharacterEvent::unk_38() const
{
    Character* character = m_record->m_character;
    if (!character)
        return -1;
    return character->unk_00586C03();
}

int UiCharacterEvent::character_type() const
{
    Character* character = m_record->m_character;
    if (!character)
        return 6;
    return character->type();
}

CharacterRecord* CharacterEvent::character_record() const
{
    return m_record;
}

CharacterRecord* CharacterEvent::unk_18() const
{
    return m_record;
}

Faction* CharacterEvent::faction() const
{
    return m_record->m_faction;
}

int CharacterEvent::unk_38() const
{
    Character* character = m_record->m_character;
    if (!character)
        return -1;
    return character->unk_00586C03();
}

int CharacterEvent::character_type() const
{
    Character* character = m_record->m_character;
    if (!character)
        return 6;
    return character->type();
}

Faction* SettlementEvent::faction() const
{
    return m_settlement->faction();
}

TriggerPlace* SettlementEvent::settlement() const
{
    return m_settlement;
}

int SettlementEvent::unk_38() const
{
    return m_settlement->unk_8c();
}

Faction* FortEvent::faction() const
{
    return m_fort->faction();
}

TriggerPlace* FortEvent::fort() const
{
    return m_fort;
}

int FortEvent::unk_38() const
{
    return m_fort->unk_8c();
}

Faction* FactionEvent::faction() const
{
    return m_faction;
}

static char s_unused_event_key;
static char s_deleted_event_key;

EventRegistrar::EventRegistrar(const char* name, LinkedList<Trigger>* triggers)
{
    if (!s_event_types)
#line 929
        s_event_types = NEW HashMap<TextKey, LinkedList<Trigger>*, TextHash, TextEqual>(&s_unused_event_key,
                                                                                         &s_deleted_event_key, 10);
    s_event_types->insert(name, triggers);
}

void init_triggers()
{
}

void free_triggers()
{
    // NONMATCHING - stack slot order differs
    if (s_event_types) {
        HashMap<TextKey, LinkedList<Trigger>*, TextHash, TextEqual>* event_types = s_event_types;
        for (HashMap<TextKey, LinkedList<Trigger>*, TextHash, TextEqual>::iterator it = event_types->begin();
             it != event_types->end(); ++it) {
        }
#line 974
        CA_DELETE(s_event_types);
    }
}

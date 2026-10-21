#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\character_record.cpp"
// A character's record: its name, family, attributes, traits and ancillaries. (The original's path, which its
// __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include <stdio.h>
#include <string.h>

#include "character_record.h"

#include "unsorted.h"
#include "unit_db.h"
#include "fog_of_war.h"
#include "core_strings.h"
#include "army.h"
#include "battle.h"
#include "campaign_triggers.h"
#include "faction.h"
#include "romans_utils.h"
#include "settlement.h"
#include "strategy_map.h"
#include "unit.h"
#include "version_registry.h"
#include "world.h"
#include "world_map.h"
#include "map_constants.h"
#include "character.h"

static const ConstInt s_unknown_9(9);

int CharacterRecord::s_next_id;
static char s_name[1024];               // a character's full name, as set_name builds it
static char s_no_name[4];
static char s_no_text[4];

extern "C" {
void fn_00CB17CB(CharacterRecord* record, TraitLevel* level);       // tells the player a trait was gained
void fn_00CB19ED(CharacterRecord* record, TraitLevel* level);       // tells the player a trait was lost
void fn_00CB1C02(Character* character, CharacterAncillary* ancillary);  // tells the player of a new ancillary
void fn_00CB22B1(CharacterRecord* record);
void fn_00B2BC04(int message, Character* character, int);
void fn_008E2F67(CharacterRecord* record, int type, int faction, int name_faction);
CharacterRecord* fn_008E2CE8(Faction* faction, int first_name, int surname, int age, int, int, int);
Character* fn_008E2A1C(CharacterRecord* record);                    // puts the record's character on the map
void fn_008B3858(CharacterRecord* record);
void fn_008B2F15(CharacterRecord* record);
const UniString* fn_008FF387(int kind, int faction, const char* name);  // a name (0 male, 1 female, 2 surname)
int fn_008FF505(int kind, int faction, String* name);
int fn_008FF4D7(int kind, int faction, const UniString* name);      // a name's ID, to save
const char* fn_008FF4A3(int kind, int faction, int id);             // a saved name's key
void fn_008FF55B(const char* full_name, String* first_name, String* surname);
const UniString* fn_00D92EE8(int type);
void fn_00528CD1(SaveStream& stream, SenateCareer& career);
void fn_00528DD8(SaveStream& stream, SenateCareer& career);
void fn_004136D7(void* object, int saved_address);
void* fn_004138B0(void* saved);                                     // a saved pointer's new value
void* fn_004137F3(void* saved);
void fn_0042BD6D(unsigned int& points, unsigned int& level);
extern UnkC0A37E* lbl_029219DC;
extern Battle* lbl_029219E4;
}

// Messages to the triggers about a character's family (not identified).
class UnkMessage8A4724 : public UnkMessage {
public:
    UnkMessage8A4724(CharacterRecord* record);  // 0x008A4724
};

class UnkMessage8A47F3 : public UnkMessage {
public:
    UnkMessage8A47F3(CharacterRecord* record);  // 0x008A47F3
};

inline int num_traits_defined()
{
    return TraitDatabase::s_traits.size();
}

inline int num_ancillaries_defined()
{
    return AncillaryDatabase::s_ancillaries.size();
}

CharacterTrait::CharacterTrait(TraitDefinition* definition, int points)
{
    m_definition = definition;
    m_level = &m_definition->m_levels[0];
    set_points(points);
}

CharacterTrait::CharacterTrait(TraitDefinition* definition, unsigned int level)
{
    m_definition = definition;
    if (level > m_definition->m_num_levels)
        level = m_definition->m_num_levels;
    m_level = &m_definition->m_levels[level];
    m_points = m_level->m_threshold;
}

bool CharacterTrait::set_points(int points)
{
    m_points = points;
    if (m_points < 0)
        m_points = 0;
    return update_level(true);
}

bool CharacterTrait::add_points(const int& points)
{
    m_points += points;
    return update_level(true);
}

bool CharacterTrait::remove_points(const int& points, int& leftover)
{
    m_points -= points;
    if (m_points < 0)
        leftover = abs_int(m_points);
    return update_level(false);
}

// Works out the level from the points. When increasing, only a level up counts as a change, and NoGoingBackLevel
// does not hold; when decreasing, only a level down counts, and the level stops at NoGoingBackLevel.
bool CharacterTrait::update_level(bool increase)
{
    unsigned int previous_level = m_level->m_level;
    unsigned int new_level = previous_level;
    int current_points = m_points;
    if (current_points < 0)
        current_points = 0;
    for (int i = m_definition->m_num_levels - 1; i >= 0; i--) {
        if (current_points >= m_definition->m_levels[i].m_threshold) {
            new_level = m_definition->m_levels[i].m_level;
            if (!increase && previous_level >= m_definition->m_no_going_back_level && new_level < previous_level) {
                new_level = m_definition->m_no_going_back_level;
                m_points = m_definition->m_levels[new_level].m_threshold;
            }
            break;
        }
    }
    m_level = &m_definition->m_levels[new_level];
    if (new_level > previous_level && increase)
        return true;
    if (new_level < previous_level && !increase)
        return true;
    return false;
}

CharacterAncillary::CharacterAncillary(AncillaryDescription* definition)
{
    m_definition = definition;
}

CharacterRecord::CharacterRecord(Faction* faction)
    : m_id(s_next_id++), m_first_name(0), m_surname(0), m_epithet(0), m_religious_beliefs(0),
      m_religious_orders(0), m_character(0), m_epithet_trait(-1), m_epithet_level(-1), m_num_children(0),
      m_faction(faction), m_name_faction(faction ? faction->id() : 21), m_father(0), m_spouse(0), m_1c0(0),
      m_1d4(-1)
{
    m_alive = true;
    m_male = true;
    m_flag2 = false;
    m_leader = false;
    m_flag4 = false;
    m_flag5 = false;
    m_age = 0;
    m_flags13 = 0;
    m_flag16 = false;
    m_flag17 = false;
    for (unsigned int i = 0; i < 4; i++)
        m_children[i] = 0;
    memset(&m_attributes, 0, sizeof(m_attributes));
    m_194.m_value = 0;
}

CharacterRecord::CharacterRecord(const CharacterRecord& other) : m_religious_beliefs(0), m_religious_orders(0)
{
    m_id = s_next_id++;
    m_attributes = other.m_attributes;
    m_character = other.m_character;
    m_name = other.m_name;
    m_first_name = other.m_first_name;
    m_surname = other.m_surname;
    m_epithet = other.m_epithet;
    m_first_name_key = other.m_first_name_key;
    m_surname_key = other.m_surname_key;
    m_full_name_key = other.m_full_name_key;
    m_epithet_trait = other.m_epithet_trait;
    m_epithet_level = other.m_epithet_level;
    m_birth_date = other.m_birth_date;
    m_death_date = other.m_death_date;
    m_num_children = other.m_num_children;
    m_faction = other.m_faction;
    m_name_faction = other.m_name_faction;
    m_194 = other.m_194;
    m_father = other.m_father;
    m_spouse = other.m_spouse;
    m_children = other.m_children;
    m_career = other.m_career;
    m_1c0 = other.m_1c0;
    m_1c4 = other.m_1c4;
    m_1cc = other.m_1cc;
    m_1d4 = other.m_1d4;
    m_alive = other.m_alive;
    m_male = other.m_male;
    m_flag2 = other.m_flag2;
    m_leader = other.m_leader;
    m_flag4 = other.m_flag4;
    m_flag5 = other.m_flag5;
    m_age = other.m_age;
    m_flags13 = other.m_flags13;
    m_flag16 = other.m_flag16;
    m_flag17 = other.m_flag17;
}

CharacterRecord::~CharacterRecord()
{
    m_1c4.clear();
}

// Adds up the religious beliefs and orders of the character's traits and ancillaries.
void CharacterRecord::update_religion()
{
    ReligionTotals total_beliefs;
    total_beliefs.clear();
    ReligionTotals orders;
    orders.clear();
    for (LinkedList<CharacterTrait*>::iterator it = m_traits.head(); it != LinkedList<CharacterTrait*>::iterator();
         ++it) {
        add_religion(total_beliefs, (*it)->level()->m_religious_belief);
        add_religion(orders, (*it)->level()->m_religious_order);
    }
    for (Array<CharacterAncillary*>::iterator at = m_ancillaries.data(); at != m_ancillaries.end(); ++at) {
        add_religion(total_beliefs, (*at)->definition()->m_religious_belief);
        add_religion(orders, (*at)->definition()->m_religious_order);
    }
    m_religious_beliefs = total_beliefs;
    m_religious_orders = orders;
}

void CharacterRecord::kill()
{
    m_character = 0;
    m_alive = false;
    if (m_character && m_character->m_record->is_leader())
        m_flag5 = true;
    m_death_date = lbl_018CD818->m_date;
}

void CharacterRecord::clear_traits_and_ancillaries()
{
    while (m_traits.size()) {
#line 316
        CA_DELETE(m_traits.tail()->m_data);
        m_traits.remove_last();
    }
    while (m_ancillaries.size() != 0) {
#line 322
        CA_DELETE(m_ancillaries.back());
        m_ancillaries.pop_back();
    }
}

CharacterTrait* CharacterRecord::find_trait(TraitDefinition* definition)
{
    for (LinkedList<CharacterTrait*>::iterator it = m_traits.head(); it != LinkedList<CharacterTrait*>::iterator();
         ++it)
        if ((*it)->definition() == definition)
            return *it;
    return 0;
}

// Gives the character exactly level of the trait (what the give_trait console command does): any level it has is
// taken away first, with its effects, whatever its NoGoingBackLevel.
void CharacterRecord::set_trait(TraitDefinition* definition, unsigned int level, bool notify)
{
    if (m_character && m_character->type() == 4)
        return;
    if (level > definition->m_num_levels)
        level = definition->m_num_levels;
    for (LinkedList<CharacterTrait*>::iterator it = LinkedList<CharacterTrait*>::iterator(m_traits.begin());
         it != LinkedList<CharacterTrait*>::iterator();
         ++it) {
        if ((*it)->definition() == definition) {
            Array<TraitEffect>& effects = (*it)->level()->m_effects;
            for (unsigned int i = 0; i < effects.size(); i++) {
                int value = m_attributes[effects[i].m_attribute] - effects[i].m_value;
                set_attribute(effects[i].m_attribute, value);
            }
#line 370
            CA_DELETE(*it);
            m_traits.erase(it);
            break;
        }
    }
    add_trait_points(definition, definition->m_levels[level].m_threshold, notify);
    if (religions_enabled())
        update_religion();
}

// Takes the trait away without touching the attributes.
void CharacterRecord::remove_trait(TraitDefinition* definition)
{
    for (LinkedList<CharacterTrait*>::iterator it = m_traits.head(); it != LinkedList<CharacterTrait*>::iterator();
         ++it) {
        if ((*it)->definition() == definition) {
#line 388
            CA_DELETE(*it);
            m_traits.remove(it);
            if (religions_enabled())
                update_religion();
            break;
        }
    }
}

// Gives points to a trait (what a trigger's Affects does, and the give_trait_points console command). The points
// first go to cancelling the traits that list this one as an anti-trait. A negative amount lowers the level of a
// trait the character has, but only a level up updates the attributes, so the old level's effects stay.
void CharacterRecord::add_trait_points(TraitDefinition* definition, int points, bool notify)
{
    if (!m_male)
        return;
    if (m_character && m_character->type() == 4)
        return;
    if (points == 0)
        return;
    bool give = true;
    for (int i = 0; i < m_traits.size(); i++) {
        TraitDefinition* other = m_traits[i]->definition();
        for (unsigned int j = 0; j < other->m_num_anti_traits; j++) {
            TraitDefinition* anti_trait = other->m_anti_traits[j];
            if (anti_trait == definition) {
                int leftover = 0;
                if (remove_trait_points(other, points, leftover, true))
                    i--;
                if (leftover != 0) {
                    points = leftover;
                    give = true;
                    break;
                }
                if (points == 0) {
                    give = false;
                    break;
                }
                if (points > 0)
                    give = false;
            }
        }
    }
    if (give) {
        bool found = false;
        for (LinkedList<CharacterTrait*>::iterator it = m_traits.head();
             it != LinkedList<CharacterTrait*>::iterator(); ++it) {
            CharacterTrait* trait = *it;
            if (trait->definition() == definition) {
                TraitLevel* old_level = trait->m_level;
                if (trait->add_points(points)) {
                    Array<TraitEffect>& old_effects = old_level->m_effects;
                    for (unsigned int i = 0; i < old_effects.size(); i++) {
                        int value = m_attributes[old_effects[i].m_attribute] - old_effects[i].m_value;
                        set_attribute(old_effects[i].m_attribute, value);
                    }
                    Array<TraitEffect>& gained_effects = trait->level()->m_effects;
                    for (unsigned int k = 0; k < gained_effects.size(); k++) {
                        int value = m_attributes[gained_effects[k].m_attribute] + gained_effects[k].m_value;
                        set_attribute(gained_effects[k].m_attribute, value);
                    }
                    TraitLevel* new_level = trait->m_level;
                    if (new_level->m_epithet)
                        set_epithet(trait->definition()->m_index, new_level->m_level);
                    if (notify && m_character && lbl_018CD818->unk_4() == 0 && new_level->m_gain_message
                        && !m_character->unk_a0())
                        fn_00CB17CB(this, new_level);
                }
                found = true;
                break;
            }
        }
        if (!found) {
#line 504
            CharacterTrait* new_trait = NEW CharacterTrait(definition, abs_int(points));
            m_traits.push_back(new_trait);
            if (religions_enabled())
                update_religion();
            TraitLevel* new_level = new_trait->m_level;
            Array<TraitEffect>& effects = new_trait->level()->m_effects;
            for (unsigned int i = 0; i < effects.size(); i++) {
                int value = m_attributes[effects[i].m_attribute] + effects[i].m_value;
                set_attribute(effects[i].m_attribute, value);
            }
            if (new_level->m_epithet)
                set_epithet(new_trait->definition()->m_index, new_level->m_level);
            if (notify && m_character && lbl_018CD818->unk_4() == 0 && new_level->m_gain_message
                && !m_character->unk_a0())
                fn_00CB17CB(this, new_level);
        }
    }
    if (m_character)
        lbl_029219DC->unk_00C0A37E(m_character);
}

// Takes points from a trait (an anti-trait being given). A level down updates the attributes, and the trait goes
// once its points reach 0. points is left as the trait's points; leftover gets what was not needed.
bool CharacterRecord::remove_trait_points(TraitDefinition* definition, int& points, int& leftover, bool notify)
{
    if (points == 0)
        return false;
    for (LinkedList<CharacterTrait*>::iterator it = m_traits.head(); it != LinkedList<CharacterTrait*>::iterator();
         ++it) {
        CharacterTrait* trait = *it;
        if (trait->definition() == definition) {
            TraitLevel* old_level = trait->m_level;
            if (trait->remove_points(points, leftover)) {
                Array<TraitEffect>& old_effects = old_level->m_effects;
                for (unsigned int i = 0; i < old_effects.size(); i++) {
                    int value = m_attributes[old_effects[i].m_attribute] - old_effects[i].m_value;
                    set_attribute(old_effects[i].m_attribute, value);
                }
                Array<TraitEffect>& lower_effects = trait->level()->m_effects;
                for (unsigned int k = 0; k < lower_effects.size(); k++) {
                    int value = m_attributes[lower_effects[k].m_attribute] + lower_effects[k].m_value;
                    set_attribute(lower_effects[k].m_attribute, value);
                }
                if (notify && m_character && lbl_018CD818->unk_4() == 0 && old_level->m_lose_message
                    && !m_character->unk_a0())
                    fn_00CB19ED(this, old_level);
                if (trait->points() >= 0 && trait->level()) {
                }
            }
            points = trait->points();
            if (points <= 0) {
#line 611
                CA_DELETE(trait);
                m_traits.remove(it);
                if (religions_enabled())
                    update_religion();
                return true;
            }
            return false;
        }
    }
    return false;
}

void CharacterRecord::copy_traits(CharacterRecord* other)
{
    LinkedList<CharacterTrait*>& traits = other->m_traits;
    for (unsigned int i = 0; i < traits.size(); i++)
        set_trait(traits[i]->definition(), traits[i]->level()->m_level, false);
}

void CharacterRecord::get_visible_traits(Array<CharacterTrait*>& traits)
{
    for (LinkedList<CharacterTrait*>::iterator it = m_traits.head(); it != LinkedList<CharacterTrait*>::iterator();
         ++it)
        if (!(*it)->definition()->m_hidden && (unsigned int)(*it)->level()->m_level > 0)
            traits.push_back(*it);
}

void CharacterRecord::clear_traits()
{
    while (m_traits.size()) {
#line 653
        CA_DELETE(m_traits.tail()->m_data);
        m_traits.pop_back();
    }
}

void CharacterRecord::remove_leader_traits()
{
    TraitDefinition* trait = ::find_trait("Factionleader");
    if (trait)
        remove_trait(trait);
    trait = ::find_trait("RomanFactionleader");
    if (trait && find_trait(trait))
        remove_trait(trait);
    trait = ::find_trait("Factionheir");
    if (trait && find_trait(trait))
        remove_trait(trait);
    trait = ::find_trait("Exheir");
    if (trait && find_trait(trait))
        remove_trait(trait);
}

bool CharacterRecord::has_ancillary(AncillaryDescription* definition)
{
    for (Array<CharacterAncillary*>::iterator it = m_ancillaries.data(); it != m_ancillaries.end(); ++it)
        if ((*it)->definition() == definition)
            return true;
    return false;
}

bool CharacterRecord::can_add_ancillary(AncillaryDescription* definition)
{
    if (!m_character || m_character->type() == 4)
        return false;
    if (m_ancillaries.size() >= 8)
        return false;
    if (has_ancillary(definition))
        return false;
    if (definition->m_exclude_cultures.test(m_faction->culture()))
        return false;
    for (Array<CharacterAncillary*>::iterator it = m_ancillaries.data(); it != m_ancillaries.end(); ++it) {
        AncillaryDescription* other = (*it)->definition();
        for (unsigned int i = 0; i < other->m_num_excluded; i++) {
            AncillaryDescription* excluded = other->m_excluded[i];
            if (excluded == definition)
                return false;
        }
    }
    return true;
}

bool CharacterRecord::add_ancillary(AncillaryDescription* definition)
{
    if (can_add_ancillary(definition)) {
        if (definition->m_unique && lbl_018CD818->unk_00458257(definition))
            return false;
        Array<TraitEffect>& effects = definition->m_effects;
        for (unsigned int i = 0; i < effects.size(); i++) {
            int value = m_attributes[effects[i].m_attribute] + effects[i].m_value;
            set_attribute(effects[i].m_attribute, value);
        }
#line 778
        CharacterAncillary* ancillary = NEW CharacterAncillary(definition);
        m_ancillaries.push_back(ancillary);
        if (religions_enabled())
            update_religion();
        if (lbl_018CD818->unk_4() == 0 && !m_character->unk_a0())
            fn_00CB1C02(m_character, ancillary);
        if (m_character->unk_0058AFA9())
            m_character->unk_0058AFA9()->m_dd8.update(true);
        if (m_character)
            lbl_029219DC->unk_00C0A37E(m_character);
        return true;
    }
    return false;
}

void CharacterRecord::remove_ancillary(AncillaryDescription* definition)
{
    Array<TraitEffect>& effects = definition->m_effects;
    for (unsigned int i = 0; i < effects.size(); i++) {
        int value = m_attributes[effects[i].m_attribute] - effects[i].m_value;
        set_attribute(effects[i].m_attribute, value);
    }
    for (Array<CharacterAncillary*>::iterator it = m_ancillaries.data(); it != m_ancillaries.end(); ++it) {
        if ((*it)->definition() == definition) {
#line 820
            CA_DELETE(*it);
            m_ancillaries.erase(it);
            if (religions_enabled())
                update_religion();
            if (m_character->unk_0058AFA9())
                m_character->unk_0058AFA9()->m_dd8.update(true);
            lbl_029219DC->unk_00C0A37E(m_character);
            break;
        }
    }
}

bool CharacterRecord::give_ancillary(AncillaryDescription* definition, CharacterRecord* to, bool report)
{
    if (m_character->type() == to->m_character->type() && to->can_add_ancillary(definition)) {
        remove_ancillary(definition);
        to->add_ancillary(definition);
        lbl_029219DC->unk_00C0A37E(m_character);
        return true;
    }
    if (report)
        fn_00B2BC04(22, m_character, 0);
    return false;
}

void CharacterRecord::get_ancillaries(Array<CharacterAncillary*>& ancillaries)
{
    for (Array<CharacterAncillary*>::iterator it = m_ancillaries.data(); it != m_ancillaries.end(); ++it)
        ancillaries.push_back(*it);
}

void CharacterRecord::set_attribute(int attribute, int value)
{
    if (attribute == 20 && m_character && m_faction->unk_180())
        m_faction->unk_180()->unk_00492498(m_character, true);
    m_attributes[attribute] = value;
    if (attribute == 20 && m_character && m_faction->unk_180())
        m_faction->unk_180()->unk_00492498(m_character, false);
    if (m_character)
        m_character->reset_movement();
}

void CharacterRecord::set_subterfuge(int value)
{
    attribute(ATTR_SUBTERFUGE) = value;
}

inline int min_int(const int& a, const int& b)
{
    return a < b ? a : b;
}

inline int max_int(const int& a, const int& b)
{
    return a < b ? b : a;
}

inline int clamp_int(const int& value, const int& low, const int& high)
{
    return min_int(max_int(value, low), high);
}

inline unsigned int max_unsigned(const unsigned int& a, const unsigned int& b)
{
    return a < b ? b : a;
}

// The character's Command in a battle army's units (with those of units, its reinforcements?), 0 to 10.
int CharacterRecord::command(Army* army, Array<Unit*>& units)
{
    int skill_level = attribute(ATTR_COMMAND);
    if (m_character && m_character->unk_005814BA()) {
        if (m_character->type() == 3)
            skill_level += attribute(12);
    } else {
        return 0;
    }
    bool leading = army->num_units() && army->unit(0)->unk_278() ? true : false;
    unsigned int total = army->num_units() + units.size();
    int count = 0;
    unsigned int i;
    count = army->unk_007225D5(true, false);
    for (i = 0; i < units.size(); i++)
        if (units[i]->unk_50()->m_2c == 1)
            count++;
    if (count > (int)total >> 1) {
        skill_level += attribute(32);
    } else {
        count = 0;
        total = 0;
        for (i = 0; i < army->num_units(); i++) {
            if (army->unit(i)->unk_278())
                leading = true;
            total++;
            if (army->unit(i)->unk_50()->m_2c == 0)
                count++;
        }
        for (i = 0; i < units.size(); i++) {
            if (units[i]->unk_278())
                leading = true;
            total++;
            if (units[i]->unk_50()->m_2c == 0)
                count++;
        }
        if (total == 0)
            total = 1;
        if (total >= 2 && leading)
            total--;
        if ((float)count / (float)total >= 0.8f)
            skill_level += attribute(33);
    }
    skill_level = clamp_int(skill_level, 0, 10);
    return skill_level;
}

// The character's Command, with its bonuses in the battle being fought, 0 to 10.
int CharacterRecord::command(BattleUnk* battle)
{
    int command = attribute(ATTR_COMMAND);
    if (m_character && m_character->unk_005814BA()) {
        Army* army = m_character->army();
        if (army && lbl_029219E4->unk_24() && army->unk_134()) {
            int side = lbl_029219E4->side_of(army);
            if (side >= 0) {
                BattleSide* own = lbl_029219E4->side(side);
                BattleSide* enemy = lbl_029219E4->side(side == 0);
                bool commanding = own->army(0) == army ? true : false;
                if (own->unk_0()) {
                    command += attribute(8);
                    if (commanding && lbl_029219E4->unk_8() == 3)
                        command += attribute(10);
                } else {
                    command += attribute(7);
                    if (commanding && lbl_029219E4->unk_8() == 3)
                        command += attribute(9);
                }
                if (lbl_029219E4->unk_c())
                    command += attribute(14);
                if (lbl_029219E4->unk_8() == 0 && m_character->unk_b4() == 10)
                    command += attribute(11);
                int best = 0;
                for (unsigned int i = 0; i < enemy->num_armies() + enemy->num_reinforcements(); i++) {
                    if (enemy->army(i)) {
                        int bonus = 0;
                        if (enemy->army(i)->faction()->is_unk_a7()) {
                            bonus = attribute(41);
                        } else {
                            int culture = enemy->army(i)->faction()->description()->m_40->m_0;
                            bonus = m_attributes[34 + culture];
                        }
                        if (bonus > best)
                            best = bonus;
                    }
                }
                command += best;
            }
        } else if (m_character->type() == 3) {
            command += attribute(12);
        }
        if (army) {
            if ((battle && battle->unk_2ec()) || army->unk_007225D5(true, false) > army->num_units() >> 1) {
                command += attribute(32);
            } else if (battle && battle->unk_2e8()) {
                command += attribute(33);
            } else {
                int num = army->unk_007225D5(false, true);
                unsigned int total = army->num_units() ? army->num_units() : 1;
                if (total > 1 && army->unit_at(0)->unk_278())
                    total--;
                if ((float)num / (float)total >= 0.8f)
                    command += attribute(33);
            }
        }
    }
    command = clamp_int(command, 0, 10);
    return command;
}

// The skill that counts most for what the character is doing: Command leading an army, Management governing a
// settlement, and Influence otherwise.
int CharacterRecord::main_skill()
{
    if (m_character) {
        if (m_character->army())
            return command((BattleUnk*)0);
        if (m_character->unk_0058AFA9())
            return management();
    }
    return influence();
}

void CharacterRecord::unk_0059146C(int a, unsigned int b)
{
    int bonus = 0;
    if (a == 2)
        bonus += 2;
    bonus += float_to_int(b * 0.25f);
    unsigned int stars = m_flags13;
    unsigned int leftover;
    unsigned int i;
    for (i = 0; i < stars; i++)
        fn_0042BD6D(stars, leftover);
    m_flags13 = stars;
}

// Once a year: ages the character, and takes away family members who die or leave.
void CharacterRecord::update_age()
{
    bool remove = false;
    if (lbl_018CD818->m_date.season() == 2 && m_alive) {
        m_age++;
        if (m_character)
            fn_008E2F67(this, m_character->type(), m_faction->id(), m_name_faction);
    }
    if (!m_character && m_alive) {
        if (m_age == 127)
            remove = true;
        else if (m_age > 60 && lbl_018CD818->m_random.range(60, 127) > 120)
            remove = true;
        if (m_father && !m_male && !m_spouse && m_age >= 60)
            remove = true;
    }
    if (remove) {
        if (m_father) {
            if (m_age < 10 || (!m_male && m_age == 60)) {
                unsigned int i;
                for (i = 0; i < m_father->m_num_children; i++) {
                    if (m_father->m_children[i] == this) {
                        m_father->m_children[i] = 0;
                        break;
                    }
                }
                if (m_father->m_num_children > 1 && i < 3) {
                    i++;
                    for (; i < m_father->m_num_children; i++) {
                        if (m_father->m_children[i - 1] == 0) {
                            m_father->m_children[i - 1] = m_father->m_children[i];
                            m_father->m_children[i] = 0;
                        }
                    }
                }
                if (m_father->m_num_children)
                    m_father->m_num_children--;
            }
        }
        if (m_age < 10) {
            destroy();
        } else if (!m_male && m_age == 60) {
            fn_00CB22B1(this);
            m_alive = false;
            m_father = 0;
            m_flag16 = false;
            m_death_date = lbl_018CD818->m_date;
        } else {
            m_alive = false;
            m_1c0 = 1;
            m_death_date = lbl_018CD818->m_date;
            fn_008B3858(this);
        }
    } else {
        if (m_faction->is_unk_a7())
            return;
        if (m_male && m_flag2) {
            if (m_age >= 16) {
                int result = m_faction->unk_004C4589(this);
                if (result)
                    m_flag2 = false;
            }
        } else if (!m_male && m_flag2 && m_age == 12) {
            m_flag2 = false;
        }
    }
}

bool CharacterRecord::unk_005919E3()
{
    if (m_character && m_flag16 && !m_faction->is_unk_a7() && m_character->type() == 5) {
        if (m_num_children < 4)
            return true;
        for (unsigned int i = 0; i < m_num_children; i++) {
            CharacterRecord* child = m_children[i];
            if (child->m_alive && !child->m_male && !child->m_spouse && child->m_age >= 12 && child->m_age <= 50)
                return true;
        }
    }
    return false;
}

CharacterRecord* CharacterRecord::create_son()
{
    int age = m_age + lbl_018CD818->m_random.range(-10, 30);
    age = clamp_int(age, 16, 75);
    int given_name = 0;
    int last_name = 0;
    fn_008FF5CB(&lbl_018CD818->m_random, m_faction->id(), 1, &given_name, &last_name);
    CharacterRecord* son = fn_008E2CE8(m_faction, given_name, last_name, age, 1, 1, 0);
    son->m_flag16 = true;
    fn_008E2F67(son, 5, m_faction->id(), m_faction->id());
    fire_triggers(UnkMessage8A4724(son));
    return son;
}

CharacterRecord* CharacterRecord::create_unk_00591C43()
{
    int age = lbl_018CD818->m_random.range(20, 30);
    int given_name = 0;
    int last_name = 0;
    fn_008FF5CB(&lbl_018CD818->m_random, m_faction->id(), 1, &given_name, &last_name);
    CharacterRecord* member = fn_008E2CE8(m_faction, given_name, last_name, age, 1, 1, 0);
    member->m_flag16 = true;
    fn_008E2F67(member, 5, m_faction->id(), m_faction->id());
    fire_triggers(UnkMessage8A4586(member));
    return member;
}

void CharacterRecord::marry(CharacterRecord* wife)
{
    Settlement* town = 0;
    Army* joined = 0;
    TilePosition location;
    bool found = false;
    if (m_faction->unk_4f4()) {
        CharacterRecord* leader = m_faction->m_leader;
        if (leader) {
            joined = leader->m_character->army();
            if (joined && !joined->unk_007233E0(true))
                joined = 0;
            if (!joined) {
                joined = leader->m_character->kind_army();
                if (joined && !joined->unk_007233E0(true))
                    joined = 0;
            }
            if (joined) {
                found = true;
                location = *joined->unk_00719BB2();
            }
        }
        if (!found)
            return;
    } else {
        town = m_faction->unk_004C44F0(wife->m_father);
        if (!town)
            return;
        joined = town->garrison();
        found = town->unk_005BA3F0(&location);
        if (joined && location != *town->position())
            joined = lbl_029219E0->unk_004AAC55(&location);
        if (!found)
            return;
    }
    if (found) {
        Character* character = fn_008E2A1C(this);
        if (character) {
            m_spouse = wife;
            wife->m_spouse = this;
            m_flag16 = true;
            CharacterRecord* wife_father = wife->m_father;
            if (wife_father) {
                for (unsigned int i = 0; i < wife_father->m_num_children; i++)
                    if (wife_father->m_children[i] != this && wife_father->m_children[i]->m_male)
                        fire_triggers(UnkMessage8A47F3(wife_father->m_children[i]));
            }
            lbl_029219DC->unk_00C097F2(m_faction->id());
            lbl_029219E0->unk_004ABED9(character, &location);
#line 1482
            Army* own_army = NEW Army(character, 0);
            character->unk_00580DC3(own_army);
            lbl_018CD80C->unk_0048204D(own_army, lbl_018CD80C->tile(*character->position())->m_region);
            character->m_b4 = 0;
            character->unk_00581619(character->unk_005815D7());
            Unit* general_unit = own_army->unk_0071DADD(character, 0, 0, 0, 0);
            if (!general_unit) {
                own_army->unk_00724043(1, 1, 3, 1);
                return;
            }
            general_unit->m_27c = character->unk_c4();
            UnitUnk4D469D experience;
            int ok = town->unk_005BB0ED(general_unit, experience);
            if (!ok) {
                for (unsigned int i = 0; i < general_unit->num_soldiers(); i++) {
                    UnitSoldier* soldier = &general_unit->m_soldiers[i];
                    soldier->m_23 = max_unsigned(experience.m_24, soldier->m_23);
                    soldier->m_26 = max_unsigned(experience.m_1c, soldier->m_23);
                }
                general_unit->unk_007C28A2();
                general_unit->unk_007C2E5E();
            }
            m_faction->unk_004CA5BA(own_army);
            if (m_faction->is_unk_a7())
                own_army->unk_007201B3(0, lbl_018CD80C->unk_00482A48(character->position())->creator_faction());
            m_faction->unk_180()->unk_00492498(character, false);
            if (joined) {
                character->unk_004B7B27(2);
                joined->unk_00725C23(own_army);
            }
            character->unk_00587D27();
        }
    }
}

void CharacterRecord::adopt(CharacterRecord* father)
{
    if (father->m_num_children >= 4)
        return;
    if (m_character) {
        m_father = father;
        m_flag16 = true;
        for (unsigned int i = 0; i < m_father->m_num_children; i++)
            if (m_father->m_children[i] != this && m_father->m_children[i]->m_male)
                fire_triggers(UnkMessage8A47F3(m_father->m_children[i]));
        m_father->m_children[m_father->m_num_children++] = this;
        fn_008B2F15(this);
        lbl_029219DC->unk_00C097F2(m_faction->id());
        return;
    }
    Settlement* town = 0;
    Army* joined = 0;
    TilePosition location;
    bool found = false;
    if (m_faction->unk_4f4()) {
        CharacterRecord* leader = m_faction->m_leader;
        if (leader) {
            joined = leader->m_character->army();
            if (joined && !joined->unk_007233E0(true))
                joined = 0;
            if (!joined) {
                joined = leader->m_character->kind_army();
                if (joined && !joined->unk_007233E0(true))
                    joined = 0;
            }
            if (joined) {
                found = true;
                location = *joined->unk_00719BB2();
            }
        }
        if (!found)
            return;
    } else {
        town = m_faction->unk_004C44F0(father);
        if (!town)
            return;
        joined = town->garrison();
        found = town->unk_005BA3F0(&location);
        if (joined && location != *town->position())
            joined = lbl_029219E0->unk_004AAC55(&location);
        if (!found)
            return;
    }
    Character* new_character = fn_008E2A1C(this);
    if (new_character) {
        m_father = father;
        m_flag16 = true;
        for (unsigned int i = 0; i < m_father->m_num_children; i++)
            if (m_father->m_children[i] != this && m_father->m_children[i]->m_male)
                fire_triggers(UnkMessage8A47F3(m_father->m_children[i]));
        m_father->m_children[m_father->m_num_children++] = this;
        fn_008B2F15(this);
        lbl_029219DC->unk_00C097F2(m_faction->id());
        lbl_029219E0->unk_004ABED9(new_character, &location);
#line 1682
        Army* own_army = NEW Army(new_character, 0);
        new_character->unk_00580DC3(own_army);
        lbl_018CD80C->unk_0048204D(own_army, lbl_018CD80C->tile(*new_character->position())->m_region);
        new_character->m_b4 = 0;
        new_character->unk_00581619(new_character->unk_005815D7());
        Unit* general_unit = own_army->unk_0071DADD(new_character, 0, 0, 0, 0);
        if (!general_unit) {
            own_army->unk_00724043(1, 1, 3, 1);
            return;
        }
        general_unit->m_27c = new_character->unk_c4();
        m_faction->unk_004CA5BA(own_army);
        if (m_faction->is_unk_a7())
            own_army->unk_007201B3(0, lbl_018CD80C->unk_00482A48(new_character->position())->creator_faction());
        m_faction->unk_180()->unk_00492498(new_character, false);
        if (joined) {
            new_character->unk_004B7B27(2);
            joined->unk_00725C23(own_army);
        }
        new_character->unk_00587D27();
    }
}

void CharacterRecord::destroy()
{
    m_faction->unk_004C4CE7(this);
    clear_traits_and_ancillaries();
#line 1734
    CA_DELETE(this);
}

void CharacterRecord::add_child(CharacterRecord* child)
{
    if (m_num_children < 4) {
        for (unsigned int i = 0; i < 4; i++) {
            if (!m_children[i]) {
                m_children[i] = child;
                m_num_children++;
                break;
            }
        }
    }
}

void CharacterRecord::set_name(const char* first_name, const char* surname, const UniString* epithet, int type)
{
    if (first_name) {
        String first;
        first = first_name;
        String last;
        last = surname;
        m_first_name = fn_008FF387(!m_male, m_name_faction, first_name);
        m_surname = fn_008FF387(2, m_name_faction, surname);
        m_epithet = epithet;
        if (m_male) {
            if (type != 4 && type != 3) {
                if (m_surname && !epithet) {
                    sprintf(s_name, "%s %s", first_name, surname);
                    UniString space(UNI_SPACE);
                    m_name = *m_first_name + space + *m_surname;
                } else {
                    sprintf(s_name, "%s", first_name);
                    m_name = *m_first_name;
                }
            } else {
                sprintf(s_name, "%s %s", type == 4 ? "Captain" : "Admiral", first_name);
                m_name = *fn_00D92EE8(type);
                if (first_name)
                    m_name += UNI_SPACE + *m_first_name;
            }
        } else {
            sprintf(s_name, "%s %s", first_name, surname);
            m_name = *m_first_name;
        }
        m_first_name_key = first;
        m_surname_key = last;
        m_full_name_key = s_name;
        if (epithet) {
            m_name += UNI_SPACE;
            m_name += *epithet;
        }
    } else {
        m_name = UniString(s_no_name);
    }
}

void CharacterRecord::copy_name(CharacterRecord* other, int type)
{
    m_name_faction = other->m_name_faction;
    const char* surname = other->m_surname_key.is_set() ? other->m_surname_key.c_str() : 0;
    set_name(other->m_first_name_key.c_str(), surname, other->m_epithet, type);
}

void CharacterRecord::take_surname(CharacterRecord* other, int type)
{
    const char* surname = other->m_surname_key.is_set() ? other->m_surname_key.c_str() : 0;
    set_name(m_first_name_key.c_str(), surname, 0, type);
}

bool CharacterRecord::unk_00592CE9(String& name)
{
    if (name != m_full_name_key) {
        String first;
        String surname;
        fn_008FF55B(name.c_str(), &first, &surname);
        bool given_matches = (!name.is_set() && !m_first_name_key.is_set()) || first == m_first_name_key;
        bool surname_matches = (!surname.is_set() && !m_surname_key.is_set()) || surname == m_surname_key;
        return given_matches && surname_matches;
    }
    return true;
}

void CharacterRecord::write_name(SaveStream& stream)
{
    stream << m_first_name_key.c_str();
    if (m_surname_key.is_set()) {
        stream << " ";
        stream << m_surname_key.c_str();
    }
}

void CharacterRecord::get_name_ids(int& first_name, int& surname)
{
    first_name = fn_008FF505(!m_male, m_name_faction, &m_first_name_key);
    surname = fn_008FF505(2, m_name_faction, &m_surname_key);
}

void CharacterRecord::set_epithet(int trait, int level)
{
    TraitLevel* epithet_level = &TraitDatabase::s_traits[trait].m_levels[level];
    m_epithet_trait = trait;
    m_epithet_level = level;
    const UniString* epithet = epithet_level->m_epithet;
    m_name = *m_first_name;
    if (m_name != s_no_text) {
        m_name += UNI_SPACE;
        m_epithet = epithet;
        m_name += *epithet;
    }
}

bool CharacterRecord::save(SaveStream& stream)
{
    if (!stream.unk_00F3D990())
        return false;
    int first_name_id = 0;
    int surname = 0;
    stream.write_value(this);
    if (m_male)
        first_name_id = fn_008FF4D7(0, m_name_faction, m_first_name);
    else
        first_name_id = fn_008FF4D7(1, m_name_faction, m_first_name);
    surname = fn_008FF4D7(2, m_name_faction, m_surname);
    stream.write_value(m_character);
    stream.write_value(m_name_faction);
    stream.write_value(m_194);
    stream.write_value(first_name_id);
    stream.write_value(surname);
    stream.write_value(m_epithet_trait);
    stream.write_value(m_epithet_level);
    stream.write_value(m_birth_date);
    stream.write_value(m_death_date);
    stream.write_value(m_num_children);
    stream.write_value(m_father);
    stream.write_value(m_spouse);
    stream.write_value(child(0));
    stream.write_value(child(1));
    stream.write_value(child(2));
    stream.write_value(child(3));
    fn_00528CD1(stream, m_career);
    stream.write_value(m_1c0);
    unsigned int bits = m_alive;
    bits += m_male * 2;
    bits += m_flag2 * 4;
    bits += m_leader * 8;
    bits += m_flag4 << 4;
    bits += m_flag5 << 5;
    bits += m_age << 6;
    bits += m_flags13 << 13;
    bits += m_flag16 << 16;
    bits += m_flag17 << 17;
    stream.write_value(bits);
    for (unsigned int i = 0; i < NUM_ATTRIBUTES; i++)
        stream.write_value(m_attributes[i]);
    unsigned short num_traits = m_traits.size();
    unsigned short n = 0;
    stream.write_value(num_traits);
    for (n = 0; n < num_traits; n++)
        m_traits[n]->save(stream);
    unsigned short num_ancillaries = m_ancillaries.size();
    stream.write_value(num_ancillaries);
    for (n = 0; n < num_ancillaries; n++)
        m_ancillaries[n]->save(stream);
    m_1c4.save(stream);
    m_1cc.save(stream);
    stream.write_value(m_1d4);
    return true;
}

static VersionRegistrar s_version("CHARACTER_RECORD", 8);

// The attributes are loaded as saved: nothing works them out again from the traits and ancillaries.
bool CharacterRecord::load(SaveStream& stream)
{
    if (!stream.is_open())
        return false;
    unsigned int file_version = VersionRegistry::instance()->version("CHARACTER_RECORD");
    int first_name_id = 0;
    int surname = 0;
    int old_this = 0;
    old_this = stream.read_int();
    fn_004136D7(this, old_this);
    stream.read(&m_character, 4);
    stream.read(&m_name_faction, 4);
    if (file_version >= 4)
        stream.read(&m_194, 4);
    first_name_id = stream.read_int();
    surname = stream.read_int();
    m_epithet_trait = stream.read_int();
    m_epithet_level = stream.read_int();
    stream.read(&m_birth_date, 8);
    stream.read(&m_death_date, 8);
    m_num_children = stream.read_int();
    stream.read(&m_father, 4);
    stream.read(&m_spouse, 4);
    stream.read(&child(0), 4);
    stream.read(&child(1), 4);
    stream.read(&child(2), 4);
    stream.read(&child(3), 4);
    fn_00528DD8(stream, m_career);
    stream.read(&m_1c0, 4);
    unsigned int bits = 0;
    bits = stream.read_int();
    m_alive = bits;
    m_male = bits >> 1;
    m_flag2 = bits >> 2;
    m_leader = bits >> 3;
    m_flag4 = bits >> 4;
    m_flag5 = bits >> 5;
    m_age = bits >> 6;
    m_flags13 = bits >> 13;
    if (file_version >= 6)
        m_flag16 = bits >> 16;
    if (file_version >= 7)
        m_flag17 = bits >> 17;
    if (file_version < 5) {
        unsigned int i;
        for (i = 0; i < 34; i++) {
            unsigned int id = i;
            if (file_version <= 4) {
                if (id == 4) {
                    m_attributes[id] = 0;
                    continue;
                }
                if (i > 4)
                    id = i - 1;
                attribute(id) = stream.read_int_ref();
            }
        }
        int culture_count = 7 + (file_version > 4);
        for (int j = 0; j < culture_count; j++)
            attribute(34 + j) = stream.read_int_ref();
        for (i = 42; i < NUM_ATTRIBUTES; i++) {
            if (file_version >= 3) {
                if (i <= 54)
                    attribute(i) = stream.read_int_ref();
            } else if (file_version >= 2) {
                if (i <= 48)
                    attribute(i) = stream.read_int_ref();
            } else if (i <= 41) {
                attribute(i) = stream.read_int_ref();
            }
        }
    } else {
        for (unsigned int k = 0; k < NUM_ATTRIBUTES; k++)
            attribute(k) = stream.read_int_ref();
    }
    unsigned short num_traits = 0;
    unsigned short index = 0;
    num_traits = stream.read_ushort();
    for (index = 0; index < num_traits; index++) {
        int points = 0;
        points = stream.read_int();
        int id = 0;
        id = stream.read_int();
        if (id < num_traits_defined()) {
            TraitDescription* definition = &TraitDatabase::s_traits[id];
#line 2133
            CharacterTrait* trait = NEW CharacterTrait(definition, points);
            m_traits.push_back(trait);
        }
    }
    unsigned short num_ancillaries = 0;
    num_ancillaries = stream.read_ushort();
    for (index = 0; index < num_ancillaries; index++) {
        int id = 0;
        id = stream.read_int();
        if (id < num_ancillaries_defined()) {
            AncillaryDescription* definition = &AncillaryDatabase::s_ancillaries[id];
#line 2150
            CharacterAncillary* ancillary = NEW CharacterAncillary(definition);
            m_ancillaries.push_back(ancillary);
        }
    }
    m_1c4.load(stream);
    m_1cc.load(stream);
    m_1d4 = stream.read_int();
    m_first_name_key = fn_008FF4A3(!m_male, m_name_faction, first_name_id);
    m_surname_key = fn_008FF4A3(2, m_name_faction, surname);
    if (m_male && m_father && !m_flag16) {
        if (file_version >= 8) {
        } else {
            m_flag16 = true;
        }
    }
    if (m_epithet_trait >= 0) {
        TraitLevel* level = &TraitDatabase::s_traits[m_epithet_trait].m_levels[m_epithet_level];
        m_epithet = level->m_epithet;
    }
    return true;
}

void CharacterRecord::unk_00593CE1()
{
    m_flag16 = true;
    m_flag17 = true;
    if (!m_character)
        m_flag5 = true;
}

void CharacterRecord::resolve_pointers()
{
    m_father = (CharacterRecord*)fn_004138B0(m_father);
    m_spouse = (CharacterRecord*)fn_004138B0(m_spouse);
    child(0) = (CharacterRecord*)fn_004138B0(child(0));
    child(1) = (CharacterRecord*)fn_004138B0(child(1));
    child(2) = (CharacterRecord*)fn_004138B0(child(2));
    child(3) = (CharacterRecord*)fn_004138B0(child(3));
    bool character = false;
    if (character)
        m_character = (Character*)fn_004137F3(m_character);
    const char* surname = m_surname_key.is_set() ? m_surname_key.c_str() : 0;
    set_name(m_first_name_key.c_str(), surname, m_epithet, m_character ? m_character->type() : 5);
    if (religions_enabled())
        update_religion();
}

void CharacterRecord::remove_child(CharacterRecord* child_record)
{
    for (unsigned int i = 0; i < 4; i++) {
        if (m_children[i] == child_record) {
            for (unsigned int j = i; j < 3; j++)
                m_children[j] = m_children[j + 1];
            child(3) = 0;
        }
    }
    child_record->m_father = 0;
}

bool CharacterTrait::save(SaveStream& stream)
{
    stream.write_value(m_points);
    stream.write_value(definition()->m_index);
    return true;
}

bool CharacterAncillary::save(SaveStream& stream)
{
    stream.write_value(definition()->m_index);
    return true;
}

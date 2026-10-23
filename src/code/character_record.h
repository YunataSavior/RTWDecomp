// A character's record: its name, family, attributes, traits and ancillaries (code\character_record.cpp). Every
// character has one, and so does every family member who is not on the map.
#ifndef CHARACTER_RECORD_H
#define CHARACTER_RECORD_H

#include "unsorted.h"
#include "core_strings.h"
#include "army.h"
#include "calender.h"
#include "faction.h"
#include "romans_utils.h"
#include "unit.h"

#include "core_linked_list.h"
#include "character_traits_db.h"
#include "character_ancillaries_db.h"
#include "ai/ai_senate_faction.h"

class Character;
class CharacterRecord;

// The attributes a character's traits and ancillaries change (Command, Influence, Management, Subterfuge, ...), by
// the IDs character_traits_db.cpp reads Effect lines into.
enum Attribute {
    ATTR_COMMAND = 0,
    ATTR_INFLUENCE = 1,
    ATTR_MANAGEMENT = 2,
    ATTR_SUBTERFUGE = 3,
    ATTR_LOYALTY = 4,
    ATTR_PUBLIC_SECURITY = 0x10,
    ATTR_ELECTABILITY = 0x13,
    ATTR_TRAINING_UNITS = 0x15,
    ATTR_LOCAL_POPULARITY = 0x19,
    ATTR_FARMING = 0x1c,
    ATTR_GRAIN_TRADING = 0x2a,
    ATTR_SLAVE_TRADING = 0x2b,
    ATTR_HEALTH = 0x2c,
    ATTR_SQUALOR = 0x2d,
    ATTR_UNREST = 0x2e,
    ATTR_LAW = 0x2f,
    ATTR_SENATE_STANDING = 0x31,
    ATTR_POPULAR_STANDING = 0x32,
    ATTR_TRAINING_ANIMAL_UNITS = 0x35,
    NUM_ATTRIBUTES = 55,
};

inline int abs_int(const int& value)
{
    return value < 0 ? -value : value;
}

// A trait a character has: its definition, its current level and its points (12 bytes).
class CharacterTrait {
public:
    CharacterTrait(TraitDefinition* definition, int points);              // 0x0058E6B6
    CharacterTrait(TraitDefinition* definition, unsigned int level);      // 0x0058E6EB
    bool set_points(int points);                                          // 0x0058E743
    bool add_points(const int& points);                                   // 0x0058E771
    bool remove_points(const int& points, int& leftover);                 // 0x0058E797
    bool update_level(bool increase);                                     // 0x0058E7F5
    bool save(SaveStream& stream);                                        // 0x00593F56

    TraitDefinition* definition() const { return m_definition; }
    TraitLevel* level() const { return m_level; }
    int points() const { return m_points; }

    TraitDefinition* m_definition;
    TraitLevel* m_level;                        // in m_definition's m_levels
    int m_points;
};

// An ancillary a character has (4 bytes).
class CharacterAncillary {
public:
    CharacterAncillary(AncillaryDescription* definition);                 // 0x0058E8F4
    bool save(SaveStream& stream);                                        // 0x00593F8A

    AncillaryDescription* definition() const { return m_definition; }

    AncillaryDescription* m_definition;
};

// The religious beliefs or orders a character's traits and ancillaries add up to.
class ReligionTotals : public FixedArray<int, 10> {
public:
    ReligionTotals() {}
    void fill(const int& value);                                          // 0x0048B694
    void clear() { fill(0); }
    ReligionTotals(int value)
    {
        int v = value;
        for (unsigned int i = 0; i < 10; i++)
            m_items[i] = v;
    }
};

inline void add_religion(ReligionTotals& totals, const TraitReligion& religion)
{
    totals[religion.type()] += religion.value();
}

// (An int with a constructor; not identified.)
struct CharacterRecord194 {
    CharacterRecord194() { m_value = 0; }

    int m_value;
};

// A character's record (0x1dc bytes).
class CharacterRecord {
public:
    CharacterRecord(Faction* faction);                                    // 0x0058E90A
    CharacterRecord(const CharacterRecord& other);                        // 0x0058EC72
    ~CharacterRecord();                                                   // 0x0058F1B4

    void update_religion();                                               // 0x0058F25F
    void kill();                                                          // 0x0058F4EB
    void clear_traits_and_ancillaries();                                  // 0x0058F573
    CharacterTrait* find_trait(TraitDefinition* definition);              // 0x0058F6A5
    void set_trait(TraitDefinition* definition, unsigned int level, bool notify);     // 0x0058F70E
    void remove_trait(TraitDefinition* definition);                       // 0x0058F8E1
    void add_trait_points(TraitDefinition* definition, int points, bool notify);      // 0x0058F9D7
    bool remove_trait_points(TraitDefinition* definition, int& points, int& leftover, bool notify);  // 0x0058FF84
    void copy_traits(CharacterRecord* other);                             // 0x0059020E
    void get_visible_traits(Array<CharacterTrait*>& traits);              // 0x005902CE
    void clear_traits();                                                  // 0x00590352
    void remove_leader_traits();                                          // 0x005903E5
    bool has_ancillary(AncillaryDescription* definition);                 // 0x00590498
    bool can_add_ancillary(AncillaryDescription* definition);             // 0x0059051C
    bool add_ancillary(AncillaryDescription* definition);                 // 0x0059067D
    void remove_ancillary(AncillaryDescription* definition);              // 0x0059082A
    bool give_ancillary(AncillaryDescription* definition, CharacterRecord* to, bool report);  // 0x005909F6
    void get_ancillaries(Array<CharacterAncillary*>& ancillaries);        // 0x00590A9E
    void set_attribute(int attribute, int value);                         // 0x00590B15
    void set_subterfuge(int value);                                       // 0x00590BE7
    int command(Army* army, Array<Unit*>& units);                         // 0x00590C02
    int command(BattleUnk* battle);                                       // 0x00590EC6
    int main_skill();                                                     // 0x00591415
    int management();                                                     // 0x0049C5B6
    int subterfuge();                                                     // 0x004D491A: 0 to 10
    int influence();                                                      // 0x004D496D: 0 to 10
    int loyalty();                                                        // 0x0058D1E6: 0 to 10
    void unk_0059146C(int, unsigned int);                                 // 0x0059146C
    void update_age();                                                    // 0x00591522
    bool unk_005919E3();                                                  // 0x005919E3
    CharacterRecord* create_suitor();                                     // 0x00591B00
    CharacterRecord* create_adoptee();                                    // 0x00591C43
    void marry(CharacterRecord* wife);                                    // 0x00591D2D
    void adopt(CharacterRecord* father);                                  // 0x00592326
    void destroy();                                                       // 0x005928F1
    void add_child(CharacterRecord* child);                               // 0x0059292F
    void set_name(const char* first_name, const char* surname, const UniString* epithet, int type);  // 0x00592996
    void copy_name(CharacterRecord* other, int type);                     // 0x00592C23
    void take_surname(CharacterRecord* other, int type);                  // 0x00592C91
    bool unk_00592CE9(String& name);                                      // 0x00592CE9
    void write_name(SaveStream& stream);                                  // 0x00592E12: "first surname", as text
    void get_name_ids(int& first_name, int& surname);                     // 0x00592E6E
    void set_epithet(int trait, int level);                               // 0x00592EC9
    bool save(SaveStream& stream);                                        // 0x00592F6C
    bool load(SaveStream& stream);                                        // 0x005933E8
    void unk_00593CE1();                                                  // 0x00593CE1
    void resolve_pointers();                                              // 0x00593D39: after loading
    void remove_child(CharacterRecord* child);                            // 0x00593ECC

    int& attribute(int id) { return m_attributes[id]; }
    int attribute_value(const int& id) { return m_attributes.m_items[id]; }
    int attribute_at(int id) { int index = id; return attribute_value(index); }
    CharacterRecord*& child(int index) { return m_children[index]; }
    bool is_leader() const { return m_leader; }
    bool is_heir() const { return m_heir; }

    static int s_next_id;

    int m_id;                                   // +0x000
    UniString m_name;                           // +0x004: as shown, with the epithet
    const UniString* m_first_name;              // +0x008
    const UniString* m_surname;                 // +0x00c
    const UniString* m_epithet;                 // +0x010
    String m_first_name_key;                    // +0x014
    String m_surname_key;                       // +0x01c
    String m_full_name_key;                     // +0x024
    FixedArray<int, NUM_ATTRIBUTES> m_attributes;   // +0x02c
    LinkedList<CharacterTrait*> m_traits;       // +0x108
    Array<CharacterAncillary*> m_ancillaries;   // +0x110
    ReligionTotals m_religious_beliefs;         // +0x11c
    ReligionTotals m_religious_orders;          // +0x144
    Character* m_character;                     // +0x16c: 0 when not on the map
    int m_epithet_trait;                        // +0x170
    int m_epithet_level;                        // +0x174
    Date m_birth_date;                          // +0x178
    Date m_death_date;                          // +0x180
    unsigned int m_num_children;                // +0x188
    Faction* m_faction;                         // +0x18c
    int m_name_faction;                         // +0x190: the faction whose names it has
    CharacterRecord194 m_194;                   // +0x194
    CharacterRecord* m_father;                  // +0x198
    CharacterRecord* m_spouse;                  // +0x19c
    FixedArray<CharacterRecord*, 4> m_children; // +0x1a0
    SenateCareer m_career;                      // +0x1b0: his senate offices
    int m_1c0;                                  // +0x1c0
    String m_1c4;                               // +0x1c4
    String m_1cc;                               // +0x1cc
    int m_1d4;                                  // +0x1d4
    unsigned int m_alive : 1;                   // +0x1d8
    unsigned int m_male : 1;
    unsigned int m_flag2 : 1;
    unsigned int m_leader : 1;                  // the faction leader
    unsigned int m_heir : 1;                    // the faction heir
    unsigned int m_flag5 : 1;                   // set when the leader stands down (a guess: was the leader)
    unsigned int m_age : 7;
    unsigned int m_flags13 : 3;
    unsigned int m_flag16 : 1;
    unsigned int m_flag17 : 1;
};

#endif

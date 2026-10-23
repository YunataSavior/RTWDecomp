// A faction's campaign record (code\faction.cpp).
#ifndef FACTION_H
#define FACTION_H

#include "unsorted.h"
#include "faction_db.h"
#include "faction_economics.h"

class FactionAI;
class FactionUnk180;
class FactionUnk4F8;
class Fort;
class SettlementUnk184;

// A set of factions, a bit each.
class FactionSet {
public:
    int bits() const { return m_bits; }
    void clear() { m_bits = 0; }
    void add(int faction) { m_bits |= 1 << faction; }
    bool contains(int faction) const { return (m_bits & (1 << faction)) != 0; }
    unsigned int count() const
    {
        unsigned int bits = m_bits;
        unsigned int total = 0;
        while (bits) {
            if (bits & 1)
                total++;
            bits >>= 1;
        }
        return total;
    }

    int m_bits;
};

class Character;
class CharacterRecord;
class TriggerEvent;
class MarriageOffer;
class AdoptionOffer;

// The family's marriage and adoption offers (a human faction's).
class FactionUnk88C {
public:
    void make_offers();                 // 0x004D2ACF: may offer a suitor for a daughter, then a son for adoption
    float family_per_region();          // 0x004D2CCA: the family over 0.75 per region held + 1
    void end_adoption_offer();          // 0x004D30B0
    void end_marriage_offer();          // 0x004D30E8
    unsigned int child_slots();         // 0x004D3120: 4 less the children of each family man of 16 or more on the
                                        //   map with fewer than 4
    unsigned int unk_004D3219();        // 0x004D3219: room for adoptions: 4 less the children of each man (alive or
                                        //   not) of 16 or more with m_flag16, or 1 when no record has m_flag17

    CharacterRecord* bride();           // 0x004D34F3: the first unmarried daughter of 12 to 50 in a settlement
                                        //   free to take her
    CharacterRecord* adopting_father(bool in_settlement, bool older, CharacterRecord* other);  // 0x004D35DF
    void offer_lesser_general(Character* general, CharacterRecord* adoptee);  // 0x004D38B8

    Faction* m_faction;
    MarriageOffer* m_marriage;          // +0x4: the marriage offer waiting for an answer
    AdoptionOffer* m_adoption;          // +0x8: the adoption offer waiting for an answer
};

// Something of a faction's (not identified).
class FactionUnk898 {
public:
    void set_14(int value) { m_14 = value; }

    char m_pad0[0x14];
    int m_14;                           // +0x14: a faction (21 for none; set so when a named character dies in a riot)
};

struct FactionGoneEvent;

// A faction's listeners, told of events of its (not identified).
class FactionListeners {
public:
    void unk_004D71CF(const FactionGoneEvent& event);  // 0x004D71CF: tells each of them

    char m_pad0[0x10];
};

// Something of a faction's, told at the start of each round (not identified).
class FactionUnkD4 {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c(Faction* faction);  // +0x0c: at the end of its turn
    virtual void unk_10();
};

// A unit the senate gives, and where it is.
struct RewardUnit {
    RewardUnit() : m_unit(0), m_place(0) {}

    Unit* m_unit;
    Settlement* m_place;
};

// A faction's campaign record (code\faction.cpp).

class Faction {
public:
    int unk_178() const { return m_178; }
    unsigned int num_regions() const { return m_regions.size(); }
    FactionSet neighbours() const { return m_neighbours; }
    bool no_subfactions() const { return m_subfactions == 0; }
    bool has_subfactions() const { return !no_subfactions(); }  // a superfaction (the SPQR)
    bool is_superfaction_of(int faction) const { return (m_subfactions & (1 << faction)) != 0; }
    int superfaction() const { return m_superfaction; }
    bool at_war_with(Faction* other);   // 0x004CFC65
    bool unk_4f4() const { return m_4f4; }   // a horde (a guess: its AI gets a horde controller)
    FactionUnk4F8* unk_4f8() const { return m_4f8; }
    unsigned int num_settlements() const { return m_settlements.size(); }
    Settlement* capital() const { return m_capital; }
    FactionAI* ai() const { return m_ai; }
    bool unk_a7() const { return m_description->m_a7; }
    bool unk_1d4() const { return m_1d4; }
    int id() const { return m_id; }
    int control() const { return m_control; }   // 1 for a human
    int culture() const { return m_culture; }
    FactionDescription* description() const { return m_description; }
    bool is_unk_a7() const { return m_description->m_a7; }
    bool shadowed_by_set() const { return m_description->m_10 != 0; }
    int shadowed_by() const { return shadowed_by_set() ? m_description->m_shadowed_by : 21; }
    bool shadowing_set() const { return m_description->m_1c != 0; }
    int shadowing() const { return shadowing_set() ? m_description->m_shadowing : 21; }
    bool unk_4fc() const { return m_4fc; }
    FactionUnk180* unk_180() const { return m_180; }

    bool unk_00445B2B();                // 0x00445B2B: description's +0xa4 set and +0xa7 clear
    bool unk_004584F1();                // 0x004584F1: a superfaction or one of a superfaction's factions
    Character* unk_004C41EF(const char* name);          // 0x004C41EF: its character on the map with that name
    Settlement* unk_004C44F0(CharacterRecord* father);  // 0x004C44F0
    int unk_004C4589(CharacterRecord* record);          // 0x004C4589
    void unk_004C4CE7(CharacterRecord* record);         // 0x004C4CE7
    bool unk_004C5093(Character* character, bool, bool);  // 0x004C5093
    Army* unk_004C9439(TilePosition* position, int);    // 0x004C9439: a new army of its, there (a guess)
    void unk_004CA5BA(Army* army);                      // 0x004CA5BA
    void unk_004CA308(bool);                            // 0x004CA308: makes or replaces its AI (a guess)
    void unk_004CAC88();                                // 0x004CAC88: its turn update
    void unk_004CB8CC();                                // 0x004CB8CC: at the end of its turn
    bool unk_004CAEDB();                                // 0x004CAEDB: its part of the end of a round
    void marian_reforms();                              // 0x004D411D
    bool unk_004D43D5();                                // 0x004D43D5: it can become a horde (a guess: its culture's
                                                        //   +0xd9 is set, and it has fewer of something than its
                                                        //   description's +0xb8)
    void unk_004CB391();                                // 0x004CB391: works out m_neighbours
    void unk_004CB56C();                                // 0x004CB56C
    void start_turn();                                  // 0x004CB6EA
    void unk_004C9616(Character* spy);                  // 0x004C9616
    void plague_tick();                                 // 0x004CB040
    void catch_spy(Character* spy, float strength, Settlement* settlement);  // 0x004CB5A9
    void unk_004CF14F();                                // 0x004CF14F
    Army* unk_004C9515(Character* leader, int);         // 0x004C9515: an army for the new character
    void unk_004CDFCF(Character* leader);               // 0x004CDFCF: it comes back
    CharacterRecord* unk_004CE1E1();                    // 0x004CE1E1: its first record with m_flag17
    void set_leader(CharacterRecord* record);           // 0x004CEC05
    int stance(Faction* other);                         // 0x004CFC0B: 0 allied, 600 at war (DiplomaticRelation); 200 for itself or none
    int stance(int other);                              // 0x004CFBD3: the same, by the other's index
    void* display_name();                               // 0x004C3F6B: its name's text (a guess)
    CharacterRecord* leader() const { return m_leader; }
    CharacterRecord* heir() const { return m_heir; }
    int average_order();                                // 0x004CEB90: its settlements' public order percentages' average
    void give_first_cohort();                           // 0x004D245E: a "roman legionary first cohort i" or "ii", 50-50, once the +0x2b6 flag is set
    void unk_004D3E6F(int);                             // 0x004D3E6F
    void destroy();                                     // 0x004CE245: it's gone (the kill_faction command too)
    void unk_004CE839(int faction);                     // 0x004CE839
    void unk_004CE844(int faction, int);                // 0x004CE844
    void unk_004CE943(int faction, int);                // 0x004CE943: hands its ... to the faction (a guess)
    void unk_004CEA1C(int faction, int, int);           // 0x004CEA1C: the same (a guess)
    void unk_004CEAC8(int faction, int);                // 0x004CEAC8: the same (a guess)
    void set_heir(CharacterRecord* record);             // 0x004CECB3
    void kill_leader();                                 // 0x004CED61: the leader's character on the map dies
    void choose_leader();                               // 0x004CED7E: the heir, or the family's most influential
    void choose_heir(CharacterRecord* record, bool announce);  // 0x004CEEEA: the record, or the most influential
    void set_capital(Settlement* capital);              // 0x004CF09A: moves the settlements' m_is_capital too
    void unk_004D0905(Array<Region*>* regions);         // 0x004D0905: its settlements' regions (a guess)
    unsigned int unk_004D6EC0(LinkedList<Character*>* characters, int type);  // 0x004D6EC0: adds its characters of a type; how many
    bool unk_004CF466();                // 0x004CF466: one of its settlements is at 70% public order or less
    Settlement* unk_004CF4E5();         // 0x004CF4E5: its settlement with the lowest public order
    unsigned int unk_004D0EC5(Array<Region*>& regions); // 0x004D0EC5: the same as get_regions
    unsigned int get_regions(Array<Region*>& regions);  // 0x004D0F3F: adds its regions; how many there are then
    void add_mission(SenateMissionImpl* mission);   // 0x004D0FB9: adds a senate mission under way
    void add_offer(SenateMissionImpl* mission);     // 0x004D0FD6: adds a senate mission it's offered
    void accept_offer(SenateMissionImpl* mission);  // 0x004D0FF3: takes on an offered mission
    void drop_offer(SenateMissionImpl* mission);    // 0x004D1054: lets an offered mission go
    void unk_004D10BD(Array<SenateMissionImpl*>& missions); // 0x004D10BD: its missions whose unk_18 is true
    void drop_mission(Array<SenateMissionImpl*>::iterator& it); // 0x004D114E
    void end_missions();                            // 0x004D11EE: at the end of its turn
    void drop_mission(SenateMissionImpl* mission);  // 0x004D136A
    void check_missions(AISenateFaction* senate);   // 0x004D13E8: drops those that can't be done any more
    void build_missions();                          // 0x004D1479: after a load
    unsigned int buildable_units(Array<int>& units);    // 0x004D15FB: cheapest first; returns how many it found
    void give_units(int type, int count, RewardUnit* unit);  // 0x004D1BF3: COUNT units of the type, at its capital
    // The senate's unit rewards: COUNT units, and the last one's unit and place.
    void give_buildable_units(int count, RewardUnit* unit);  // 0x004D1F29: one of the dearer half of buildable_units
    void give_best_buildable_units(int count, RewardUnit* unit);  // 0x004D1F9B: the dearest of buildable_units
    void give_triarii(int count, RewardUnit* unit);     // 0x004D1FEF: "roman triarii"
    void give_minor_exotic_units(int count, RewardUnit* unit);  // 0x004D2036
    void give_major_exotic_units(int count, RewardUnit* unit);  // 0x004D21DF

    char m_pad0[0x90];
    FactionListeners m_listeners;       // +0x90
    int m_id;                           // +0xa0
    int m_culture;                      // +0xa4
    Settlement* m_capital;              // +0xa8
    CharacterRecord* m_leader;          // +0xac
    CharacterRecord* m_heir;            // +0xb0
    FactionDescription* m_description;  // +0xb4
    int m_control;                      // +0xb8 (1: human)
    FactionAI* m_ai;                    // +0xbc
    int m_c0;                           // +0xc0
    int m_c4;                           // +0xc4
    char m_padc8[0xd4 - 0xc8];
    FactionUnkD4* m_d4;                 // +0xd4
    char m_padd8[4];
    PointerList<CharacterRecord> m_records;     // +0xdc
    PointerList<Character> m_characters;        // +0xe8
    PointerList<Army> m_armies;                 // +0xf4
    Array<int> m_regions;                       // +0x100: its regions' indices
    PointerList<Settlement> m_settlements;      // +0x10c
    PointerList<Fort> m_forts;                  // +0x118
    char m_pad124[0x130 - 0x124];
    PointerList<SettlementUnk184> m_ports;  // +0x130: its settlements' ports (a guess)
    FactionSet m_neighbours;            // +0x13c: the factions whose regions border this one's
    Array<int> m_bordering_regions;     // +0x140: the other factions' regions beside its own
    bool m_14c;                         // +0x14c: it holds a sea region (a guess)
    char m_pad14d[0x170 - 0x14d];
    float m_170;                        // +0x170: set from a global at the start of its turn
    char m_pad174[0x178 - 0x174];
    int m_178;                          // +0x178
    char m_pad17c[4];
    FactionUnk180* m_180;               // +0x180
    Array<SenateMissionImpl*> m_missions;   // +0x184: its senate missions under way
    Array<SenateMissionImpl*> m_offers;     // +0x190: the senate missions it's offered
    char m_pad19c[0x1cc - 0x19c];
    int m_triumph_order;                // +0x1cc: public order for a triumph, down by 1 a turn
    int m_wonder_order;                 // +0x1d0: public order for capturing a wonder, 4 then down by 1 a turn
    bool m_1d4;                         // +0x1d4
    char m_pad1d5[0x4e4 - 0x1d5];
    bool m_4e4;                         // +0x4e4: set when one of its settlements' figures changes
    char m_pad4e5[0x4ec - 0x4e5];
    int m_subfactions;                  // +0x4ec: the factions it's the superfaction of, a bit each (0x0045798F)
    int m_superfaction;                 // +0x4f0: its superfaction, or 21 for none
    bool m_4f4;                         // +0x4f4
    char m_pad4f5[0x4f8 - 0x4f5];
    FactionUnk4F8* m_4f8;               // +0x4f8
    bool m_4fc;                         // +0x4fc: it took over from the faction it shadows, so its end has no message
    char m_pad4fd[0x500 - 0x4fd];
    FactionFinance m_finance;           // +0x500
    FactionUnk88C m_88c;                // +0x88c
    FactionUnk898 m_898;                // +0x898
};

#endif

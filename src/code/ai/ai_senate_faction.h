#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\ai\\ai_senate_faction.h"
// The senate's missions (code\ai\ai_senate_faction.h; the original's path, which its __FILE__ gives). Its line
// numbers are the original's.
#ifndef AI_SENATE_FACTION_H
#define AI_SENATE_FACTION_H

#include "unsorted.h"
#include "ai_faction_base.h"
#include "../calender.h"
#include "../faction.h"
#include "../port.h"
#include "../settlement.h"
#include "core_linked_list.h"
#include "secure_int.h"

class SenateReward;
class Character;

// An office a man held, and when.
struct OfficeTerm {
    OfficeTerm(int office, const Date& from, const Date& to) : m_office(office), m_from(from), m_to(to) {}

    int m_office;                       // OFFICE_*
    Date m_from;                        // +0x04
    Date m_to;                          // +0x0c
};

// A man's senate career (a character record's +0x1b0; SENATE_SERVICE_HISTORY in a save).
struct SenateCareer {
    SenateCareer() : m_office(-1), m_history(0) {}
    ~SenateCareer()
    {
        if (m_history)
#line 123
            CA_DELETE(m_history);
    }

    bool in_office() { return m_office != -1; }
    int leave_office();                 // 0x00528B04: adds the term to his history; the office he held
    bool has_held(int office);          // 0x00528BC6
    bool may_hold_again(int office);    // 0x00528C45: not when he left it today

    int m_office;                       // +0x00: OFFICE_*, or -1
    Date m_since;                       // +0x04
    LinkedList<OfficeTerm>* m_history;  // +0x0c: or 0 before his first office
};

// What a senate office restricts, and what it gives (from descr_senate.txt's Restrictions and Senate_benefits).
enum {
    OFFICE_NOT_CONSECUTIVELY = 0,   // can't be held again straight after
    OFFICE_QUAESTOR_TENURE = 1,     // must have been quaestor
    OFFICE_AEDILE_TENURE = 2,
    OFFICE_PRAETOR_TENURE = 3,
    OFFICE_CONSUL_TENURE = 4,
    OFFICE_UNKNOWN_RESTRICTION = 5,
};

enum {
    OFFICE_NO_CENSOR_SUICIDE = 0,   // a censor's investigation won't end in suicide
    OFFICE_CENSOR_IMMUNITY = 1,     // no investigation by the censor
};

// The offices, by name.
enum {
    OFFICE_QUAESTOR = 0,
    OFFICE_AEDILE = 1,
    OFFICE_PRAETOR = 2,
    OFFICE_CONSUL = 3,
    OFFICE_CENSOR = 4,
    OFFICE_PONTIFEX_MAXIMUS = 5,
    OFFICE_UNKNOWN = 6,
};

// A senate office (0x3c bytes). descr_senate.txt's offices make one for each of its Quantity.
class SenateOffice {
public:
    SenateOffice(int title, int text, int rank, int type, int duration, int sittings, const Array<int>& restrictions,
                 const Array<int>& benefits);   // 0x005283ED
    bool out_of_office(Character* character);   // 0x00528472: holds no office now
    bool eligible(Character* character);        // 0x005284A9: holds no office, and meets the restrictions
    int remove_ineligible(LinkedList<Character*>* characters);  // 0x005285CC: how many are left
    void appoint(CharacterRecord* record);      // 0x00528643: fires Become<office>, until today + the duration
    void vacate();                              // 0x00528855: fires Ceased<office>
    static void appoint_first(int type, CharacterRecord* record);   // 0x00528AAD: to the first office of the type
    CharacterRecord* holder() const { return m_holder; }

    int m_title;                        // +0x00: its name's text (0x106 + 2 * type; 0x51b when unknown)
    int m_text;                         // +0x04: the text after that (0x107 + 2 * type)
    int m_rank;                         // +0x08
    CharacterRecord* m_holder;          // +0x0c: or 0
    Date m_date;                        // +0x10: when the holder's term ends; the date it was made before that
    int m_type;                         // +0x18
    int m_duration;                     // +0x1c: in turns
    int m_sittings;                     // +0x20: how many times a man can hold it; 0 for no limit
    Array<int> m_restrictions;          // +0x24
    Array<int> m_benefits;              // +0x30
};

// A mission the senate gives (code\senate_mission.cpp; SENATE_MISSION_IMPL in a save), and the root of the builders
// that make them; see senate_mission.h.
class SenateMissionImpl {
public:
    virtual void unk_00();
    virtual void* title();              // +0x04: its name's text
    virtual void unk_08(int* a, int* b);
    virtual int unk_0c();
    virtual int turns_left();           // +0x10
    virtual bool reward_title(void** title);    // +0x14: its reward's name, and whether it's a reward
    virtual bool unk_18();
    virtual int unk_1c();               // 0x005470F2
    virtual int unk_20();               // (0 in the base)
    virtual void unk_24();              // +0x24: adds it to the faction's list
    virtual void unk_28();
    virtual int run_out();              // +0x2c: on its last turn, fails it and returns true
    virtual Faction* faction();         // +0x30: whom it was given to
    virtual int result();               // +0x34: MISSION_FAILED or MISSION_SUCCEEDED when it's over
    virtual bool possible(AISenateFaction* senate); // +0x38: false (and cancelled) when it can't be done any more
    virtual bool save(SaveStream& stream);  // +0x3c
    virtual SenateMissionImpl* build(); // +0x40: a builder's mission (a builder deletes itself)
    virtual ~SenateMissionImpl();       // +0x44: 0x005470E0
};

enum {
    MISSION_FAILED = 1,
    MISSION_SUCCEEDED = 3,
};

// What the senate makes of a faction this turn (0x1c bytes). Faction 3 is the senate's own.
struct SenateView {
    void reset();                       // 0x00528F7E
    void update_state(int tolerance);   // 0x00528FE0: from the flags below

    bool m_at_war_with_senate;          // +0x00: (for the families) at war with the senate
    bool m_at_war;                      // +0x01: at war with a family or the senate
    bool m_hostile;                     // +0x02: stance 400 with a family or the senate
    bool m_allied;                      // +0x03: allied with a family or the senate
    bool m_4;                           // +0x04: a diplomacy flag (0x005A0F3E) with a family or the senate; its plan
                                        //   is then always to assassinate
    bool m_5;                           // +0x05
    bool m_targeted;                    // +0x06: a family's AI has it in view: its region m_1c is the faction's, or
                                        //   its global strategy director's m_1e is set (not identified)
    int m_threat;                       // +0x08: the families' AIs' m_c for it, summed (not identified)
    int m_c;                            // +0x0c: their m_1c, summed
    int m_10;                           // +0x10: their m_28, summed
    int m_state;                        // +0x14: SENATE_VIEW_*
    int m_plan;                         // +0x18: SENATE_PLAN_*, the missions it gives about the faction
};

// What the senate plans for a faction (AISenateFaction::update_plans), and so which missions it gives the human's
// family about it. The missions step has no case for those without a comment.
enum {
    SENATE_PLAN_EXTERMINATE = 0,        // take one of its settlements and exterminate the people
    SENATE_PLAN_ENSLAVE = 1,            // take one and enslave them
    SENATE_PLAN_SUBJUGATE = 2,
    SENATE_PLAN_ASSASSINATE = 3,        // kill one of its named characters
    SENATE_PLAN_BLOCKADE = 5,           // blockade one of its ports
    SENATE_PLAN_BROKER_PEACE = 6,
    SENATE_PLAN_OCCUPY = 7,             // take one of its settlements
    SENATE_PLAN_ANNEX = 8,
    SENATE_PLAN_EXACT_TRIBUTE = 9,
    SENATE_PLAN_APPEASE = 13,
    SENATE_PLAN_GET_MAP_INFO = 14,
    SENATE_PLAN_TRADE = 15,             // make a trade agreement
    SENATE_PLAN_TARGETED = 12,          // hostile and near, with a family's AI having it in view (m_targeted)
    SENATE_PLAN_NONE = 16,              // dead, or neutral and not near
    SENATE_PLAN_FRIEND = 18,            // SENATE_VIEW_FRIEND
    SENATE_PLAN_VIEW_5 = 20,            // SENATE_VIEW_5
    SENATE_PLAN_SENATE = 21,            // the senate itself
    SENATE_PLAN_ENEMY_OF_SENATE = 22,   // a family at war with the senate
};

// The senate's mission types (their index in the missions' table; GivenMission::m_type).
enum {
    MISSION_HELP_PLAYER = 7,
    MISSION_ASSASSINATION = 8,
    MISSION_DEMAND_SUICIDE = 11,
    MISSION_OCCUPY_CITY = 12,
    MISSION_ENSLAVE_CITY = 14,
    MISSION_EXTERMINATE_CITY = 15,
    MISSION_LEAVE_REGION = 18,
    MISSION_BLOCKADE_PORT = 19,
    MISSION_EXACT_TRIBUTE = 20,
    MISSION_SUBJUGATE = 21,
    MISSION_ANNEX_CITY = 22,
    MISSION_GET_MAP_INFO = 23,
    MISSION_MAKE_TRADE_AGREEMENT = 24,
    MISSION_APPEASE = 25,
    MISSION_BROKER_PEACE = 26,
};

enum {
    SENATE_VIEW_ENEMY_OF_SENATE = 0,    // a family at war with the senate
    SENATE_VIEW_ENEMY = 1,              // at war with Rome, or one the senate always counts as an enemy
    SENATE_VIEW_HOSTILE = 2,
    SENATE_VIEW_NEUTRAL = 3,
    SENATE_VIEW_FRIEND = 4,
    SENATE_VIEW_5 = 5,
};

// One of the three Roman families as the senate sees it (0x20 bytes).
struct SenateFamily {
    SenateFamily();                     // 0x005290BD
    void update_popularity();           // 0x005290FB
    void check_gift();                  // 0x00529593: a legion's first cohort, now and then
    int standing_level();               // 0x00532F9E: 0 to 6, from its senate standing
    bool at_war_with_senate();          // 0x00533015
    Faction* faction() const { return m_faction; }
    void set_enemy(int enemy) { m_enemy = enemy; }
    SenateReward* penalty();            // 0x005295FA: by its level: p3 below 2, p2 below 4, p1 otherwise
    SenateReward* reward();             // 0x0052964A: by its level: r1 below 2, r2 below 4, r3 otherwise

    Faction* m_faction;                 // +0x00
    int m_enemy;                        // +0x04: the neighbour it should fight (21 for none)
    int m_appreciation;                 // +0x08: added to its senate standing, down by 2 a turn
    int m_displeasure;                  // +0x0c: taken from its senate standing, down by 2 a turn
    int m_senate_standing;              // +0x10: 0 to 99
    int m_popularity;                   // +0x14: its popular standing, 0 to 99
    unsigned int m_gift_turn;           // +0x18: when the senate last gave it a unit
    bool m_senate_rising;               // +0x1c: its senate standing went up at its last change
    bool m_popularity_rising;           // +0x1d: its popularity went up at its last change
};

// A man the senate could appoint to an office, and his chance.
struct OfficeCandidate {
    OfficeCandidate() {}

    Character* m_character;
    unsigned int m_weight;
};

// A mission the senate gave a family.
struct GivenMission {
    GivenMission(int type, int faction);    // 0x00529D78: today

    int m_type;                         // +0x00
    int m_family;                       // +0x04: the family's faction index (for most types, the target's faction)
    Date m_date;                        // +0x08: when
};

// A region the senate has asked the human's family to leave, and when.
struct LeaveRegionRequest {
    int m_region;
    unsigned int m_turn;
};

class AISenateFaction;
class SettlementUnk184;

// A mission the senate could give this turn (0x10 bytes): how hard it is, and how likely the senate is to pick it.
class SenateCandidate {
public:
    SenateCandidate(int difficulty, int weight) : m_difficulty(difficulty), m_weight(weight) {}

    // Asks the family to take it on, with the reward its table gives at the family's level and this difficulty.
    virtual void give(Faction* faction, int level, AISenateFaction* senate) = 0;

    int m_difficulty;                   // +0x04: 0 to 2 (the tables' E, M and D columns)
    int m_weight;                       // +0x08
};

// The candidates of each mission type, and their targets.
#define SENATE_CANDIDATE(name, Target)                                                                      \
    class name : public SenateCandidate {                                                                   \
    public:                                                                                                 \
        name(int difficulty, int weight, Target* target) : SenateCandidate(difficulty, weight), m_target(target) \
        {                                                                                                   \
        }                                                                                                   \
        virtual void give(Faction* faction, int level, AISenateFaction* senate);                            \
        Target* m_target;                                                                                   \
    }

SENATE_CANDIDATE(BlockadePortCandidate, SettlementUnk184);  // 0x005330B5
SENATE_CANDIDATE(AssassinationCandidate, CharacterRecord);  // 0x0053329A
SENATE_CANDIDATE(ExactTributeCandidate, Faction);           // 0x0053331B
SENATE_CANDIDATE(SubjugateCandidate, Faction);              // 0x00533396
SENATE_CANDIDATE(AnnexCityCandidate, Settlement);           // 0x005335FD
SENATE_CANDIDATE(GetMapInfoCandidate, Faction);             // 0x00533411
SENATE_CANDIDATE(TradeAgreementCandidate, Faction);         // 0x00533507
SENATE_CANDIDATE(AppeaseCandidate, Faction);                // 0x0053348C
SENATE_CANDIDATE(BrokerPeaceCandidate, Faction);            // 0x00533582
SENATE_CANDIDATE(OccupyCityCandidate, Settlement);          // 0x0053313B
SENATE_CANDIDATE(EnslaveCityCandidate, Settlement);         // 0x005331B0
SENATE_CANDIDATE(ExterminateCityCandidate, Settlement);     // 0x00533225

// The senate's base class's second vtable, at +0x44 (not identified).
class AISenateUnk44 {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual bool unk_14(int faction);   // +0x14 (not identified)
    virtual bool unk_18(int faction);   // +0x18 (not identified)
};

// The senate: an AI faction that judges the Roman families and gives them missions and offices.
class AISenateFaction {
public:
    void update();                                  // 0x0052A3D4: its turn
    void mission_finished(SenateMissionImpl* mission);  // 0x0052B168
    void start_appreciation();                      // 0x0052B22C
    void start_displeasure();                       // 0x0052B24B
    void withhold_offices();                        // 0x0052B26A: sets m_offices_withheld
    void promise_appointment();                     // 0x0052B27D: sets m_appointment_promised
    // Whether the first Censor (or Quaestor) office is empty or held by one of the faction's men: then its
    // investigations do nothing.
    bool censor_protects(Faction* faction);         // 0x0052B940
    bool quaestor_protects(Faction* faction);       // 0x0052B9DD
    bool at_war_with(Faction* faction);             // 0x0052B8F2: with a family or the senate
    void unk_0052E033();                            // 0x0052E033: runs what's due today from 0x014D5F70's list
    void update_families();                         // 0x0052E178
    void add_family_view(FactionAI* family_ai);     // 0x0052E7D2: adds a family's AI's figures to each view
    void update_diplomacy();                        // 0x0052E911: sets the views' diplomacy flags
    int choose_enemy(Faction* family);              // 0x0052EC0D: the neighbour the senate wants the family to fight
    void update_plans();                            // 0x0052EDA6: each faction's view's m_plan
    void roman_strength(int* armies, int* fleets);  // 0x0052F91E: the senate's and the families' at peace with it
    unsigned int rome_settlements();                // 0x0052FA68: the senate's and the families' settlements
    bool is_family_enemy(int faction);              // 0x0052FB4F: a family's m_enemy
    void update_senate_standing(SenateFamily* family);  // 0x0052FBAF
    void update_offices();                          // 0x0052FFC8: ends the terms due today, and fills them
    void unk_00530A0D();                            // 0x00530A0D: the missions
    bool given_recently(int type, int family, unsigned int turns);  // 0x00532F12
    void offer(const GivenMission& given, SenateMissionImpl* mission);  // 0x00532EDB
    AISenateUnk44* unk_44() { return (AISenateUnk44*)((char*)this + 0x44); }
    Faction* faction() const { return m_faction; }
    SenateView& view(int faction) { return m_views[faction]; }
    SenateFamily& family(int family) { return m_families[family]; }
    Faction* family_faction(int family) const { return m_families[family].m_faction; }
    int family_enemy(int family) const { return m_families[family].m_enemy; }

    char m_pad0[4];                     // its base class's (an AI faction's), with a second vtable at +0x44
    Faction* m_faction;                 // +0x04: the senate
    char m_pad8[0x48 - 8];
    SenateView m_views[21];             // +0x48: one per faction
    SenateFamily m_families[3];         // +0x294: the Julii, the Brutii and the Scipii
    Faction* m_senate;                  // +0x2f4
    SenateFamily* m_player_family;      // +0x2f8: the human's family, or 0
    Date m_appreciation_until;          // +0x2fc
    Date m_displeasure_until;           // +0x304
    bool m_offices_withheld;            // +0x30c: the offices_witheld penalty: the human's family is left out of the
                                        //   next appointments
    bool m_appointment_promised;        // +0x30d: the promise_of_appointment reward: only the human's family is
                                        //   considered for them
    bool m_mission_failed;             // +0x30e: set when a mission fails
    bool m_30f;                         // +0x30f
    unsigned int m_missions_succeeded;  // +0x310
    int m_missions_failed;              // +0x314
    LinkedList<GivenMission> m_given_missions;  // +0x318
    LinkedList<LeaveRegionRequest> m_leave_requests;  // +0x320: the last 3 turns'
    unsigned int m_advice_turn[10];     // +0x328: when the advisor last spoke of each standing change
};

#endif

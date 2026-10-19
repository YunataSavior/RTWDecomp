#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\Ai\\ai_senate_faction.cpp"
// The senate: descr_senate.txt, the Roman families' senate standing and popularity, and the senate's turn. (The
// original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include <float.h>
#include <math.h>

#include "unsorted.h"

#include "core_linked_list.h"
#include "../character.h"
#include "../character_record.h"
#include "../landmark.h"
#include "ai_senate_faction.h"
#include "../senate_mission.h"

enum {
    FACTION_JULII = 0,
    FACTION_BRUTII = 1,
    FACTION_SCIPII = 2,
    FACTION_SENATE = 3,
    NUM_FACTIONS = 21,
};

enum {
    CHARACTER_NAMED = 5,    // descr_character.txt's "named character" (0x00E3B72D)
};

enum {
    STANCE_HOSTILE = 400,   // (a guess, from the stances' order)
    STANCE_AT_WAR = 600,
};

extern "C" {
extern unsigned int lbl_010B5070;           // the popularity_land_percentage option (27)
extern LandmarkManager lbl_018BF5A8;        // the campaign's landmarks
void fn_00CBAE4F(Faction* faction);         // when a family's popularity goes above 66 (an event, 0x82)
void* fn_00D93A0C(int advice);              // the advisor's thread of this number (a guess)
void fn_00CBB282(void* advice, bool senate, bool rising);  // the advisor speaks of a standing's change (a guess)
Character* fn_008E24E2();                   // a new named character for the senate, at its capital; 0 without one
void fn_00CBB01C(LinkedList<SenateOffice*>* offices);  // tells the player of the offices the family's men got
}

// A faction event (code\faction_event.cpp) as the senate sees it: what the events are built on, a virtual base
// counting who holds it.
class FactionEvent {
public:
    virtual ~FactionEvent();

    // Lets go of it, and deletes it when nothing else holds it.
    bool release()
    {
        if (--m_holders == 0) {
            delete this;
            return true;
        }
        return false;
    }

    char m_pad4[4];
    unsigned char m_holders;            // +0x08
};

// A faction event as the list of them keeps it.
class FactionEventRecord : public virtual FactionEvent {
public:
    virtual void unk_00(AISenateFaction* senate);                       // +0x00: AISenateFaction::unk_0052E033
    virtual void judge(AISenateFaction* senate, SenateFamily* family);  // +0x04: what the senate makes of it

    Date m_date;                        // +0x08
    bool m_judged;                      // +0x10
};

// A sea route a fleet could take (not identified).
struct SeaRoute {
    char m_pad0[4];
    unsigned char m_turns;              // +0x04: how long it takes (a guess)
};

// What lbl_029219E0's +0x84 is (not identified).
class PathFinderUnk84 {
public:
    SeaRoute* unk_0049060E(int, int);                                           // 0x0049060E
    SeaRoute* unk_004908F6(int, int, TilePosition* from, int, int, TilePosition* to);  // 0x004908F6
};

inline PathFinderUnk84* sea_routes()
{
    return (PathFinderUnk84*)((char*)lbl_029219E0 + 0x84);
}

// What fn_008F60BF gives (not identified).
struct FactionNames {
    int m_0;
    String m_name;                      // +0x04: as in descr_sm_factions.txt
};

extern "C" {
extern LinkedList<FactionEventRecord*> lbl_014D5F70;  // the faction events (code\faction_event.cpp)
extern PathFinder* lbl_029219C4;
extern Campaign* lbl_02921AC0;
FactionNames* fn_008F60BF(int faction);
bool fn_0051E6C7(Region* from, Region* to, int* turns);  // whether an army can get there, and in how many turns (a guess)
bool fn_0051E515(Faction* faction, Faction* other, int* turns);  // the fewest of fn_0051E6C7's turns between their regions

// What each mission type makes of a target: 0 to 2 for its difficulty (the tables' E, M and D columns), or 3 when it
// can't be given; the cost is taken from its weight.
int fn_0054F8AD(SettlementUnk184* port, Army* fleet, int turns);            // code\sm_blockade_port.cpp: always 0
int fn_0054F8B4(SettlementUnk184* port, SettlementUnk184* from, int turns); // code\sm_blockade_port.cpp: always 0
int fn_0054C9A1(Faction* faction, CharacterRecord* target);                 // code\sm_assassination.cpp
int fn_005572F2(Faction* faction, Faction* target, int* cost);              // code\sm_exact_tribute.cpp
int fn_005609FC(Faction* faction, Faction* target, int* cost);              // code\sm_subjugate.cpp
int fn_0054A550(Faction* faction, Faction* target, int* cost);              // code\sm_annex_city.cpp
int fn_00558B3F(Faction* faction, Faction* target, int* cost);              // code\sm_get_map_info.cpp
int fn_0055DE42(Faction* faction, Faction* target, int* cost);              // code\sm_make_trade_agreement.cpp
int fn_0054B3ED(Faction* faction, Faction* target, int* cost);              // code\sm_appease.cpp
int fn_005516FE(Faction* faction, Faction* target, int* cost);              // code\sm_broker_peace.cpp
int fn_005621DA(Faction* faction, Settlement* target, int);                 // code\sm_take_city.cpp

// Each mission type's reward code (0 to 7, as fn_0054610E takes them) from its table, at a standing level and
// difficulty.
int fn_0054F8C2(int level, int difficulty);     // Blockade_Port
int fn_0054CADD(int level, int difficulty);     // Assassination
int fn_0055730D(int level, int difficulty);     // Exact_Tribute
int fn_00560A17(int level, int difficulty);     // Subjugate
int fn_0054A56B(int level, int difficulty);     // Annex_City
int fn_00558B5A(int level, int difficulty);     // Get_Map_Info
int fn_0055DE5D(int level, int difficulty);     // Make_Trade_Agreement
int fn_0054B408(int level, int difficulty);     // Appease
int fn_00551719(int level, int difficulty);     // Broker_Peace
int fn_005621C5(int level, int difficulty);     // Take_City, for Occupy_City, Enslave_City and Exterminate_City
int fn_0055B89C(int level, int difficulty);     // Help_Player

// Each mission type's builder (code\senate_mission.cpp, which calls the type's own).
SenateMissionImpl* fn_00547F48(const SenateMissionRequest& request);                        // Help_Player
SenateMissionImpl* fn_00547F56(const SenateMissionRequest& request);                        // Demand_Suicide
SenateMissionImpl* fn_00547F88(const SenateMissionRequest& request, Region* region);        // Leave_Region
SenateMissionImpl* fn_00547F9A(const SenateMissionRequest& request, SettlementUnk184* port); // Blockade_Port
SenateMissionImpl* fn_00547FAC(const SenateMissionRequest& request, CharacterRecord* target);  // Assassination
SenateMissionImpl* fn_0054800A(const SenateMissionRequest& request, Faction* target);       // Exact_Tribute
SenateMissionImpl* fn_0054801C(const SenateMissionRequest& request, Faction* target);       // Subjugate
SenateMissionImpl* fn_0054802E(const SenateMissionRequest& request, Settlement* target);    // Annex_City
SenateMissionImpl* fn_00548040(const SenateMissionRequest& request, Faction* target);       // Get_Map_Info
SenateMissionImpl* fn_00548052(const SenateMissionRequest& request, Faction* target);       // Make_Trade_Agreement
SenateMissionImpl* fn_00548064(const SenateMissionRequest& request, Faction* target);       // Appease
SenateMissionImpl* fn_00548076(const SenateMissionRequest& request, Faction* target);       // Broker_Peace
SenateMissionImpl* fn_005480AC(const SenateMissionRequest& request, Settlement* target);    // Occupy_City
SenateMissionImpl* fn_005480BE(const SenateMissionRequest& request, Settlement* target);    // Enslave_City
SenateMissionImpl* fn_005480D0(const SenateMissionRequest& request, Settlement* target);    // Exterminate_City
}

bool load_senate_missions(ScriptReader& reader);  // 0x005470FE: descr_senate.txt's missions

// 0 when the texts are the same (strcmp's sign reversed).
inline int compare_text(const char* text, const char* other)
{
    while (*text && *text == *other) {
        text++;
        other++;
    }
    return *other - *text;
}

// The AI's total strength of a faction's armies, and of its fleets.
inline int army_strength(const int& faction)
{
    return lbl_012D0F74[faction].m_strength;
}

inline int fleet_strength(const int& faction)
{
    return lbl_012D0F68[faction].m_strength;
}

// The length of the land border between two factions' regions (0 when they don't border).
inline int border_length(int faction, int other)
{
    return lbl_012D1548[faction * 21 + other].m_length;
}

inline int min_int(const int& a, const int& b)
{
    return a < b ? a : b;
}

inline int max_int(const int& a, const int& b)
{
    return a > b ? a : b;
}

// descr_senate.txt's figures. The attitude coefficients, the attitude boundaries and the random missions' chance are
// read, but nothing uses them.
static FixedArray<float, 6> s_attitude_boundaries(0.0f);
static LinkedList<SenateOffice> s_offices;
static float s_random_mission_chance;
static float s_turn_coefficient;
static float s_strength_coefficient;
static float s_relative_strength_coefficient;
static float s_combined_strength_coefficient;
static unsigned int s_appreciation_duration;
static unsigned int s_displeasure_duration;

// How willing the senate is to let each faction be (in the faction order of the original campaign): 0 for those it
// always counts as enemies (Carthage, the Gauls, the Greek cities and the rebels). Only whether it's 0 matters.
static const int s_tolerance[NUM_FACTIONS] = {10, 10, 10, 10, 5, 5, 5, 0, 5, 5, 0, 5, 5, 5, 5, 0, 5, 5, 5, 5, 0};

// One office of descr_senate.txt: its name, then Rank, Quantity, Duration, Sittings, Restrictions ...
// End_restrictions, Senate_benefits ... End_senate_benefits and End.
static bool load_office(ScriptReader& reader)
{
    // NONMATCHING - stack slot order differs, and the original reserves 0x60 more bytes of stack
    if (!reader.is_open())
        return false;
    const char* name = reader.rest_of_line();
    if (!find_key(reader, "Rank"))
        return false;
    int rank = reader.read_int();
    if (!find_key(reader, "Quantity"))
        return false;
    unsigned int quantity = reader.read_int();
    if (!find_key(reader, "Duration"))
        return false;
    int duration = reader.read_int();
    if (!find_key(reader, "Sittings"))
        return false;
    int sittings = reader.read_int();
    if (!find_key(reader, "Restrictions"))
        return false;
    if (!reader.next_line())
        return false;
    Array<int> restrictions;
    while (!reader.match("End_restrictions")) {
        const char* word = reader.word();
        int restriction = OFFICE_UNKNOWN_RESTRICTION;
        if (compare_text(word, "not_consecutively") == 0)
            restriction = OFFICE_NOT_CONSECUTIVELY;
        else if (compare_text(word, "quaestor_tenure") == 0)
            restriction = OFFICE_QUAESTOR_TENURE;
        else if (compare_text(word, "aedile_tenure") == 0)
            restriction = OFFICE_AEDILE_TENURE;
        else if (compare_text(word, "praetor_tenure") == 0)
            restriction = OFFICE_PRAETOR_TENURE;
        else if (compare_text(word, "consul_tenure") == 0)
            restriction = OFFICE_CONSUL_TENURE;
        restrictions.push_back(restriction);
        if (!reader.next_line())
            return false;
    }
    if (!find_key(reader, "Senate_benefits"))
        return false;
    if (!reader.next_line())
        return false;
    Array<int> benefits;
    while (!reader.match("End_senate_benefits")) {
        const char* word = reader.word();
        if (compare_text(word, "no_censor_suicide") == 0) {
            int benefit = OFFICE_NO_CENSOR_SUICIDE;
            benefits.push_back(benefit);
        } else if (compare_text(word, "censor_immunity") == 0) {
            int benefit = OFFICE_CENSOR_IMMUNITY;
            benefits.push_back(benefit);
        }
        if (!reader.next_line())
            return false;
    }
    if (!find_key(reader, "End"))
        return false;
    int title = 0x51b;
    int text = 0x51b;
    int type = OFFICE_UNKNOWN;
    if (compare_text(name, "Quaestor") == 0) {
        title = 0x106;
        text = 0x107;
        type = OFFICE_QUAESTOR;
    } else if (compare_text(name, "Aedile") == 0) {
        title = 0x108;
        text = 0x109;
        type = OFFICE_AEDILE;
    } else if (compare_text(name, "Praetor") == 0) {
        title = 0x10a;
        text = 0x10b;
        type = OFFICE_PRAETOR;
    } else if (compare_text(name, "Consul") == 0) {
        title = 0x10c;
        text = 0x10d;
        type = OFFICE_CONSUL;
    } else if (compare_text(name, "Censor") == 0) {
        title = 0x10e;
        text = 0x10f;
        type = OFFICE_CENSOR;
    } else if (compare_text(name, "Pontifex Maximus") == 0) {
        title = 0x110;
        text = 0x111;
        type = OFFICE_PONTIFEX_MAXIMUS;
    }
    for (unsigned int i = 0; i < quantity; i++)
        s_offices.push_back(SenateOffice(title, text, rank, type, duration, sittings, restrictions, benefits));
    if (!reader.next_line())
        return false;
    return true;
}

SenateOffice::SenateOffice(int title, int text, int rank, int type, int duration, int sittings,
                           const Array<int>& restrictions, const Array<int>& benefits)
    : m_title(title), m_text(text), m_rank(rank), m_holder(0), m_date(lbl_018CD818->m_date), m_type(type),
      m_duration(duration), m_sittings(sittings), m_restrictions(restrictions), m_benefits(benefits)
{
    // NONMATCHING - the original keeps the campaign's date in a temporary, read before the other members are set
}

bool SenateOffice::out_of_office(Character* character)
{
    // NONMATCHING - the original keeps the career's address in a temporary, and tests the office the long way round
    CharacterRecord* record = character->m_record;
    SenateCareer& career = record->m_career;
    return !career.in_office();
}

bool SenateOffice::eligible(Character* character)
{
    // NONMATCHING - stack slot order differs, and the original keeps the restriction in a temporary
    CharacterRecord* record = character->m_record;
    SenateCareer& career = record->m_career;
    bool ok = true;
    if (career.in_office())
        return false;
    for (Array<int>::iterator it = m_restrictions.begin(); it != m_restrictions.end(); ++it) {
        switch (*it) {
        case OFFICE_NOT_CONSECUTIVELY:
            ok = career.may_hold_again(m_type);
            break;
        case OFFICE_QUAESTOR_TENURE:
            ok = career.has_held(OFFICE_QUAESTOR);
            break;
        case OFFICE_AEDILE_TENURE:
            ok = career.has_held(OFFICE_AEDILE);
            break;
        case OFFICE_PRAETOR_TENURE:
            ok = career.has_held(OFFICE_PRAETOR);
            break;
        case OFFICE_CONSUL_TENURE:
            ok = career.has_held(OFFICE_CONSUL);
            break;
        }
        if (!ok)
            return false;
    }
    return true;
}

int SenateOffice::remove_ineligible(LinkedList<Character*>* characters)
{
    LinkedList<Character*>::iterator it = characters->head();
    while (it != LinkedList<Character*>::iterator()) {
        if (eligible(*it))
            ++it;
        else
            it = characters->erase(it);
    }
    return characters->size();
}

// The trigger events of taking and leaving each office (export_descr_character_traits.txt's).
#define OFFICE_EVENT(name, address)                                                                         \
    class name : public TriggerEvent {                                                                      \
    public:                                                                                                 \
        name(CharacterRecord* record);                                                                      \
        CharacterRecord* m_record;                                                                          \
    }

OFFICE_EVENT(BecomeQuaestorEvent, 0x008A4A60);
OFFICE_EVENT(BecomeAedileEvent, 0x008A4B2F);
OFFICE_EVENT(BecomePraetorEvent, 0x008A4BFE);
OFFICE_EVENT(BecomeConsulEvent, 0x008A4CCD);
OFFICE_EVENT(BecomeCensorEvent, 0x008A4D9C);
OFFICE_EVENT(BecomePontifexMaximusEvent, 0x008A4E6B);
OFFICE_EVENT(CeasedQuaestorEvent, 0x008A50D8);
OFFICE_EVENT(CeasedAedileEvent, 0x008A51A7);
OFFICE_EVENT(CeasedPraetorEvent, 0x008A5276);
OFFICE_EVENT(CeasedConsulEvent, 0x008A5345);
OFFICE_EVENT(CeasedCensorEvent, 0x008A5414);
OFFICE_EVENT(CeasedPontifexMaximusEvent, 0x008A54E3);

void SenateOffice::appoint(CharacterRecord* record)
{
    // NONMATCHING - stack slot order differs
    m_holder = record;
    switch (m_type) {
    case OFFICE_QUAESTOR:
        fire_triggers(BecomeQuaestorEvent(record));
        break;
    case OFFICE_AEDILE:
        fire_triggers(BecomeAedileEvent(record));
        break;
    case OFFICE_PRAETOR:
        fire_triggers(BecomePraetorEvent(record));
        break;
    case OFFICE_CONSUL:
        fire_triggers(BecomeConsulEvent(record));
        break;
    case OFFICE_CENSOR:
        fire_triggers(BecomeCensorEvent(record));
        break;
    case OFFICE_PONTIFEX_MAXIMUS:
        fire_triggers(BecomePontifexMaximusEvent(record));
        break;
    }
    // The same for each of the other families and the senate, on his army or on him (not identified).
    for (unsigned int family = 0; family <= FACTION_SENATE; family++) {
        if (family != (unsigned int)record->m_faction->m_id) {
            if (record->m_character->army() != 0)
                record->m_character->army()->unk_0071EBF6(family, 3);
            else
                record->m_character->unk_00580620(family, 3);
        }
    }
    Date& today = lbl_018CD818->m_date;
    int office = m_type;
    SenateCareer& career = record->m_career;
    career.m_office = office;
    career.m_since = today;
    m_date = lbl_018CD818->m_date;
    m_date.add_turns(m_duration);
}

void SenateOffice::vacate()
{
    if (m_holder != 0) {
        switch (m_holder->m_career.leave_office()) {
        case OFFICE_QUAESTOR:
            fire_triggers(CeasedQuaestorEvent(m_holder));
            break;
        case OFFICE_AEDILE:
            fire_triggers(CeasedAedileEvent(m_holder));
            break;
        case OFFICE_PRAETOR:
            fire_triggers(CeasedPraetorEvent(m_holder));
            break;
        case OFFICE_CONSUL:
            fire_triggers(CeasedConsulEvent(m_holder));
            break;
        case OFFICE_CENSOR:
            fire_triggers(CeasedCensorEvent(m_holder));
            break;
        case OFFICE_PONTIFEX_MAXIMUS:
            fire_triggers(CeasedPontifexMaximusEvent(m_holder));
            break;
        }
        m_holder = 0;
    }
}

void SenateOffice::appoint_first(int type, CharacterRecord* record)
{
    for (LinkedList<SenateOffice>::iterator it = s_offices.head(); it != LinkedList<SenateOffice>::iterator(); ++it) {
        if ((*it).m_type == type) {
            (*it).appoint(record);
            return;
        }
    }
}

int SenateCareer::leave_office()
{
    // NONMATCHING - stack slot order differs
    if (m_history == 0)
#line 554
        m_history = new (THIS_FILE, __FILE__, __LINE__, true) LinkedList<OfficeTerm>;
    Date& today = lbl_018CD818->m_date;
    m_history->push_back(OfficeTerm(m_office, m_since, today));
    int office = m_office;
    m_office = -1;
    return office;
}

bool SenateCareer::has_held(int office)
{
    // NONMATCHING - the original copies the history's first node into one temporary fewer
    if (m_history == 0)
        return false;
    for (LinkedList<OfficeTerm>::iterator it = m_history->begin(); it != m_history->end(); ++it) {
        if ((*it).m_office == office)
            return true;
    }
    return false;
}

bool SenateCareer::may_hold_again(int office)
{
    if (m_history == 0)
        return true;
    OfficeTerm& last = *m_history->last();
    return !(last.m_office == office && last.m_to == lbl_018CD818->m_date);
}

// A date this many turns from today (two turns a year).
static void turns_from_today(Date* date, unsigned int turns)
{
    *date = lbl_018CD818->m_date;
    date->m_year += turns / 2;
    if (turns % 2 == 1)
        date->next_turn();
}

void SenateView::reset()
{
    m_state = SENATE_VIEW_NEUTRAL;
    m_plan = 0x10;
    m_at_war_with_senate = false;
    m_at_war = false;
    m_targeted = false;
    m_hostile = false;
    m_allied = false;
    m_4 = false;
    m_5 = false;
    m_threat = 0;
    m_c = 0;
    m_10 = 0;
}

void SenateView::update_state(int tolerance)
{
    if (m_at_war_with_senate)
        m_state = SENATE_VIEW_ENEMY_OF_SENATE;
    else if (m_at_war || (m_targeted && m_threat > m_10) || tolerance == 0)
        m_state = SENATE_VIEW_ENEMY;
    else if (m_hostile && m_threat > 0 && m_threat > m_10)
        m_state = SENATE_VIEW_HOSTILE;
    else if (m_allied || (m_threat == 0 && m_10 > 0 && !m_hostile))
        m_state = SENATE_VIEW_FRIEND;
    else if (m_4)
        m_state = SENATE_VIEW_5;
    else
        m_state = SENATE_VIEW_NEUTRAL;
}

SenateFamily::SenateFamily()
{
    m_faction = 0;
    m_enemy = NUM_FACTIONS;
    m_appreciation = 0;
    m_displeasure = 0;
    m_senate_standing = 50;
    m_popularity = 30;
}

// The family's popular standing: its settlements' public order, its share of the land and of the wonders, and its
// characters' PopularStanding.
void SenateFamily::update_popularity()
{
    // NONMATCHING - stack slot order differs, and the original keeps the clamps' bounds and the counts in temporaries
    bool was_popular = (unsigned int)m_popularity > 66 && !lbl_018CD818->m_2b4;
    int old = m_popularity;
    m_popularity = 0;
    if (m_faction == 0 || m_faction->m_178 >= 2)
        return;
    if (m_displeasure > 0)
        m_displeasure -= 2;
    if (m_appreciation > 0)
        m_appreciation -= 2;
    int order = m_faction->average_order();
    order = min_int(max_int(order, 0), 200);
    unsigned int land_target = lbl_010B5070 * lbl_018CD80C->num_regions() / 100;
    unsigned int land = m_faction->m_regions.size() * 100 / land_target;
    if (lbl_018BF5A8.size() > 0) {
        unsigned int wonders = lbl_018BF5A8.num_held(m_faction->m_id) * 25 / lbl_018BF5A8.size();
        m_popularity = order / 10 + land / 2 + wonders / 2;
    } else {
        m_popularity = order / 10 + land / 2;
    }
    int characters = 0;
    unsigned int count = m_faction->m_characters.size();
    for (unsigned int i = 0; i < count; i++)
        characters += m_faction->m_characters.m_items[i]->m_record->attribute(ATTR_POPULAR_STANDING);
    float average = (float)characters;
    average /= (float)count;
    characters = float_to_int_chop(average);
    if (characters > 0 || m_popularity + characters > 0)
        m_popularity += characters;
    else
        m_popularity = 0;
    int popularity = m_popularity;
    m_popularity = min_int(max_int(popularity, 0), 99);
    if (lbl_018CD818->m_2b4) {
        popularity = m_popularity;
        m_popularity = min_int(max_int(popularity, 0), 65);
    }
    if (m_popularity != old)
        m_popularity_rising = m_popularity > old;
    if (!was_popular && (unsigned int)m_popularity > 66 && !lbl_018CD818->m_2b4)
        fn_00CBAE4F(m_faction);
}

void SenateFamily::check_gift()
{
    // NONMATCHING - the original reads the turn into a temporary each time
    if ((unsigned int)m_senate_standing > 80) {
        unsigned int turn = lbl_018CD818->m_turn;
        if (turn - m_gift_turn > 10 && (unsigned int)lbl_018CD818->m_random.percent() > 50) {
            m_gift_turn = lbl_018CD818->m_turn;
            m_faction->give_first_cohort();
        }
    }
}

bool load_senate_description()
{
    ScriptReader reader(fn_0040E567("descr_senate.txt"), 0);
    if (!reader.is_open())
        return false;
    if (!find_key(reader, "appreciation_duration"))
        return false;
    s_appreciation_duration = reader.read_int();
    if (!find_key(reader, "displeasure_duration"))
        return false;
    s_displeasure_duration = reader.read_int();
    if (!find_key(reader, "probability_of_random_mission"))
        return false;
    s_random_mission_chance = reader.read_float();
    if (!find_key(reader, "turn_coefficient"))
        return false;
    s_turn_coefficient = reader.read_float();
    if (!find_key(reader, "strength_coefficient"))
        return false;
    s_strength_coefficient = reader.read_float();
    if (!find_key(reader, "relative_strength_coefficient"))
        return false;
    s_relative_strength_coefficient = reader.read_float();
    if (!find_key(reader, "combined_strength_coefficient"))
        return false;
    s_combined_strength_coefficient = reader.read_float();
    if (!find_key(reader, "attitude_boundary_1"))
        return false;
    s_attitude_boundaries[0] = reader.read_float();
    if (!find_key(reader, "attitude_boundary_2"))
        return false;
    s_attitude_boundaries[1] = reader.read_float();
    if (!find_key(reader, "attitude_boundary_3"))
        return false;
    s_attitude_boundaries[2] = reader.read_float();
    if (!find_key(reader, "attitude_boundary_4"))
        return false;
    s_attitude_boundaries[3] = reader.read_float();
    if (!find_key(reader, "attitude_boundary_5"))
        return false;
    s_attitude_boundaries[4] = reader.read_float();
    if (!find_key(reader, "attitude_boundary_6"))
        return false;
    s_attitude_boundaries[5] = reader.read_float();
    if (!find_key(reader, "Begin_Offices"))
        return false;
    reader.next_line();
    while (!reader.match("End_Offices")) {
        if (!load_office(reader))
            return false;
    }
    return load_senate_missions(reader);
}

void AISenateFaction::update()
{
    // NONMATCHING - the original reads the human's family's faction and its +0x178 into temporaries
    unk_0052E033();
    update_families();
    update_offices();
    if (m_player_family != 0 && m_player_family->m_faction->m_178 == 0)
        unk_00530A0D();
}

void AISenateFaction::mission_finished(SenateMissionImpl* mission)
{
    switch (mission->result()) {
    case MISSION_SUCCEEDED:
        fire_triggers(LeaderSenateMissionSuccessEvent(mission->faction()->leader()));
        m_missions_succeeded++;
        break;
    case MISSION_FAILED:
        fire_triggers(LeaderSenateMissionFailedEvent(mission->faction()->leader()));
        m_missions_failed++;
        m_mission_failed = true;
        break;
    }
}

// The senate's appreciation and displeasure last descr_senate.txt's durations.
void AISenateFaction::start_appreciation()
{
    turns_from_today(&m_appreciation_until, s_appreciation_duration);
}

void AISenateFaction::start_displeasure()
{
    turns_from_today(&m_displeasure_until, s_displeasure_duration);
}

bool AISenateFaction::at_war_with(Faction* faction)
{
    int id = faction->m_id;
    SenateView& view = m_views[id];
    return view.m_at_war_with_senate || view.m_at_war;
}

// The families' standings each turn, and the advisor's words when the human's family's change.
void AISenateFaction::update_families()
{
    // NONMATCHING - stack slot order differs, and the original reads the human's family's standings through inline
    // accessors that keep them in temporaries
    Faction* julii = family(FACTION_JULII).m_faction;
    Faction* scipii = family(FACTION_SCIPII).m_faction;
    Faction* brutii = family(FACTION_BRUTII).m_faction;
    for (unsigned int i = 0; i < NUM_FACTIONS; i++)
        m_views[i].reset();
    if (julii)
        add_family_view(julii->ai());
    if (scipii)
        add_family_view(scipii->ai());
    if (brutii)
        add_family_view(brutii->ai());
    update_diplomacy();
    for (unsigned int j = 0; j < NUM_FACTIONS; j++)
        m_views[j].update_state(s_tolerance[j]);
    if (julii)
        family(FACTION_JULII).set_enemy(choose_enemy(julii));
    if (scipii)
        family(FACTION_SCIPII).set_enemy(choose_enemy(scipii));
    if (brutii)
        family(FACTION_BRUTII).set_enemy(choose_enemy(brutii));
    update_plans();
    if (julii) {
        family(FACTION_JULII).update_popularity();
        update_senate_standing(&family(FACTION_JULII));
        if (&family(FACTION_JULII) != m_player_family)
            family(FACTION_JULII).check_gift();
    }
    if (scipii) {
        family(FACTION_SCIPII).update_popularity();
        update_senate_standing(&family(FACTION_SCIPII));
        if (&family(FACTION_SCIPII) != m_player_family)
            family(FACTION_SCIPII).check_gift();
    }
    if (brutii) {
        family(FACTION_BRUTII).update_popularity();
        update_senate_standing(&family(FACTION_BRUTII));
        if (&family(FACTION_BRUTII) != m_player_family)
            family(FACTION_BRUTII).check_gift();
    }
    if (m_player_family == 0 || lbl_018CD818->m_prologue)
        return;
    bool popularity_rising = m_player_family->m_popularity_rising;
    bool senate_rising = m_player_family->m_senate_rising;
    int popularity = m_player_family->m_popularity / 10;
    int senate = m_player_family->m_senate_standing / 10;
    unsigned int turn = lbl_018CD818->m_turn;
    if (senate_rising && senate == 9 && turn - m_advice_turn[0] > 10) {
        m_advice_turn[0] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d0), true, true);
    }
    if (senate_rising && senate == 6 && turn - m_advice_turn[1] > 10) {
        m_advice_turn[1] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d1), true, true);
    }
    if (senate_rising && senate == 3 && turn - m_advice_turn[2] > 10) {
        m_advice_turn[2] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d2), true, true);
    }
    if (!senate_rising && senate == 4 && turn - m_advice_turn[3] > 10) {
        m_advice_turn[3] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d3), true, false);
    }
    if (!senate_rising && senate == 1 && turn - m_advice_turn[4] > 10) {
        m_advice_turn[4] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d4), true, false);
    }
    if (popularity_rising && popularity == 9 && turn - m_advice_turn[5] > 10) {
        m_advice_turn[5] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d5), false, true);
    }
    if (popularity_rising && popularity == 6 && turn - m_advice_turn[6] > 10) {
        m_advice_turn[6] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d6), false, true);
    }
    if (popularity_rising && popularity == 3 && turn - m_advice_turn[7] > 10) {
        m_advice_turn[7] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d7), false, true);
    }
    if (!popularity_rising && popularity == 4 && turn - m_advice_turn[8] > 10) {
        m_advice_turn[8] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d8), false, false);
    }
    if (!popularity_rising && popularity == 1 && turn - m_advice_turn[9] > 10) {
        m_advice_turn[9] = turn;
        fn_00CBB282(fn_00D93A0C(0x4d9), false, false);
    }
}

// Adds a family's AI's figures for each faction to the senate's views.
void AISenateFaction::add_family_view(FactionAI* family_ai)
{
    // NONMATCHING - stack slot order differs
    FactionAI* ai = family_ai;
    FactionAIUnk20* targets = ai->unk_20();
    for (unsigned int i = 0; i < NUM_FACTIONS; i++) {
        int faction_id = i;
        FactionAIUnk20Entry* entry = &targets->m_entries[faction_id];
        int& threat = m_views[i].m_threat;
        threat += entry->m_c;
        int& c = m_views[i].m_c;
        c += entry->m_1c;
        int& v10 = m_views[i].m_10;
        v10 += entry->m_28;
        if (ai->unk_38()) {
            if (targets->unk_18() && faction_id == lbl_018CD80C->region(targets->unk_1c())->m_owner->id())
                m_views[i].m_targeted = true;
            if (ai->unk_38()->unk_1e(faction_id))
                m_views[i].m_targeted = true;
        }
    }
}

// The views' flags from diplomacy with the families and the senate.
void AISenateFaction::update_diplomacy()
{
    // NONMATCHING - stack slot order differs
    DiplomacyMatrix* diplomacy = &lbl_018CD818->m_diplomacy;
    for (unsigned int i = 0; i < NUM_FACTIONS; i++) {
        int faction_id = i;
        if (i < 3 && diplomacy->stance(faction_id, FACTION_SENATE) == 600)
            m_views[i].m_at_war_with_senate = true;
        if (diplomacy->stance(faction_id, FACTION_JULII) == 600 || diplomacy->stance(faction_id, FACTION_BRUTII) == 600
            || diplomacy->stance(faction_id, FACTION_SCIPII) == 600 || diplomacy->stance(faction_id, FACTION_SENATE) == 600)
            m_views[i].m_at_war = true;
        if (diplomacy->stance(faction_id, FACTION_JULII) == 400 || diplomacy->stance(faction_id, FACTION_BRUTII) == 400
            || diplomacy->stance(faction_id, FACTION_SCIPII) == 400 || diplomacy->stance(faction_id, FACTION_SENATE) == 400)
            m_views[i].m_hostile = true;
        if (diplomacy->stance(faction_id, FACTION_JULII) == 0 || diplomacy->stance(faction_id, FACTION_BRUTII) == 0
            || diplomacy->stance(faction_id, FACTION_SCIPII) == 0 || diplomacy->stance(faction_id, FACTION_SENATE) == 0)
            m_views[i].m_allied = true;
        if (diplomacy->unk_005A0F3E(faction_id, FACTION_JULII) || diplomacy->unk_005A0F3E(faction_id, FACTION_BRUTII)
            || diplomacy->unk_005A0F3E(faction_id, FACTION_SCIPII) || diplomacy->unk_005A0F3E(faction_id, FACTION_SENATE))
            m_views[i].m_4 = true;
    }
}

// The neighbour the senate wants the family to fight: of those it's at war with, the one with the highest m_threat;
// failing that, the one with the highest m_threat that isn't friendly. 21 for none.
int AISenateFaction::choose_enemy(Faction* family)
{
    // NONMATCHING - stack slot order differs
    int enemy = NUM_FACTIONS;
    int best = -1;
    bool at_war = false;
    DiplomacyMatrix* diplomacy = &lbl_018CD818->m_diplomacy;
    int family_id = family->m_id;
    for (unsigned int i = 0; i < NUM_FACTIONS; i++) {
        int faction_id = i;
        Faction* faction = lbl_02921AC0->unk_00454960(faction_id);
        if (faction == 0 || faction->unk_004584F1() || faction->unk_a7())
            continue;
        if (family->neighbours().contains(faction_id)) {
            if (at_war) {
                if (diplomacy->stance(family_id, faction_id) == 600 && m_views[i].m_threat > best) {
                    enemy = faction_id;
                    best = m_views[i].m_threat;
                }
            } else if (diplomacy->stance(family_id, faction_id) == 600) {
                enemy = faction_id;
                best = m_views[i].m_threat;
                at_war = true;
            } else if (m_views[i].m_threat > best && m_views[i].m_state <= SENATE_VIEW_NEUTRAL) {
                enemy = faction_id;
                best = m_views[i].m_threat;
            }
        }
    }
    return enemy;
}

// Each faction's plan, from how the senate views it, how strong the Roman armies near it are against its own, and how
// strong Rome's armies are against its enemies'.
void AISenateFaction::update_plans()
{
    // NONMATCHING - stack slot order differs, the original copies the SecureInt constants in a different order, and
    // reads the capital's order slot without a temporary
    unsigned int rome = rome_settlements();
    unsigned int capital_order = 0;
    if (m_senate->capital())
        capital_order = m_senate->capital()->m_dd8.m_current.order(16);
    int armies = 0;
    int fleets = 0;
    roman_strength(&armies, &fleets);
    int enemy_armies = SecureInt(Scrambled(0xfbd6aa30)).value();   // 1
    int enemy_fleets = SecureInt(Scrambled(0xfbd6aa30)).value();   // 1
    unsigned int i;
    for (i = 0; i < NUM_FACTIONS; i++) {
        int faction_id = i;
        if (lbl_018CD818->is_faction_alive(faction_id) && m_views[i].m_state == SENATE_VIEW_ENEMY) {
            enemy_armies += army_strength(faction_id);
            enemy_fleets += fleet_strength(faction_id);
        }
    }
    float roman_ratio = (float)armies / (float)enemy_armies;
    Faction* julii = lbl_018CD818->faction(FACTION_JULII);
    Faction* scipii = lbl_018CD818->faction(FACTION_SCIPII);
    Faction* brutii = lbl_018CD818->faction(FACTION_BRUTII);
    Faction* senate = lbl_018CD818->faction(FACTION_SENATE);
    bool julii_allied = lbl_018CD818->m_diplomacy.stance(FACTION_SENATE, FACTION_JULII) < 100;
    bool scipii_allied = lbl_018CD818->m_diplomacy.stance(FACTION_SENATE, FACTION_SCIPII) < 100;
    bool brutii_allied = lbl_018CD818->m_diplomacy.stance(FACTION_SENATE, FACTION_BRUTII) < 100;
    for (i = 0; i < NUM_FACTIONS; i++) {
        int faction_id = i;
        SenateView* view = &m_views[i];
        if (faction_id == FACTION_SENATE) {
            view->m_plan = SENATE_PLAN_SENATE;
            continue;
        }
        if (view->m_4) {
            view->m_plan = SENATE_PLAN_ASSASSINATE;
            continue;
        }
        if (lbl_018CD818->is_faction_alive(faction_id)) {
            Faction* faction = lbl_018CD818->faction(faction_id);
            // Once one of them is near it or borders it, the later ones' armies count too.
            bool borders = false;
            bool in_reach = false;
            int nearby_armies = 0;
            int turns = 0;
            if (julii && julii_allied) {
                if (fn_0051E515(julii, faction, &turns) && turns <= 3)
                    in_reach = true;
                if (border_length(FACTION_JULII, faction_id) > 0)
                    borders = true;
                if (in_reach || borders)
                    nearby_armies += army_strength(FACTION_JULII);
            }
            if (scipii && scipii_allied) {
                if (fn_0051E515(scipii, faction, &turns) && turns <= 3)
                    in_reach = true;
                if (border_length(FACTION_SCIPII, faction_id) > 0)
                    borders = true;
                if (in_reach || borders)
                    nearby_armies += army_strength(FACTION_SCIPII);
            }
            if (brutii && brutii_allied) {
                if (fn_0051E515(brutii, faction, &turns) && turns <= 3)
                    in_reach = true;
                if (border_length(FACTION_BRUTII, faction_id) > 0)
                    borders = true;
                if (in_reach || borders)
                    nearby_armies += army_strength(FACTION_BRUTII);
            }
            if (fn_0051E515(senate, faction, &turns) && turns <= 3)
                in_reach = true;
            if (border_length(FACTION_SENATE, faction_id) > 0)
                borders = true;
            if (in_reach || borders)
                nearby_armies += army_strength(FACTION_SENATE);
            bool hostile = m_views[i].m_hostile;
            int own_armies = army_strength(faction_id) + 1;
            float ratio = (float)nearby_armies / (float)own_armies;
            switch (view->m_state) {
            case SENATE_VIEW_ENEMY_OF_SENATE:
                view->m_plan = SENATE_PLAN_ENEMY_OF_SENATE;
                continue;
            case SENATE_VIEW_ENEMY:
                if (borders || in_reach) {
                    if (faction->m_settlements.size() < 3 && faction->m_characters.size() < 3 && ratio > 3.0f) {
                        view->m_plan = SENATE_PLAN_EXTERMINATE;
                        continue;
                    }
                    if (ratio > 2.0f && capital_order > 0) {
                        view->m_plan = SENATE_PLAN_ENSLAVE;
                        continue;
                    }
                }
                if (faction->m_settlements.size() < 4 && faction->m_characters.size() < 5 && ratio > 1.5f
                    && !lbl_018CD818->m_diplomacy.unk_005A0FB1(faction_id)) {
                    view->m_plan = SENATE_PLAN_SUBJUGATE;
                    continue;
                }
                if (ratio < 0.3f && !faction->unk_a7() && !is_family_enemy(faction_id)
                    && !given_recently(MISSION_BROKER_PEACE, faction_id, 10)) {
                    view->m_plan = SENATE_PLAN_BROKER_PEACE;
                    continue;
                }
                if (view->m_at_war && faction->m_ports.size() > 0 && fleets > fleet_strength(faction_id)
                    && !given_recently(MISSION_BLOCKADE_PORT, faction_id, 5)) {
                    view->m_plan = SENATE_PLAN_BLOCKADE;
                    continue;
                }
                view->m_plan = SENATE_PLAN_OCCUPY;
                continue;
            case SENATE_VIEW_HOSTILE:
                if (borders || in_reach) {
                    if (ratio > 3.0f && roman_ratio > 1.2f
                        && !given_recently(MISSION_ANNEX_CITY, faction_id, SecureInt(Scrambled(0xfbd6aaf4)).value())) {
                        view->m_plan = SENATE_PLAN_ANNEX;   // not given in the last 5 turns
                        continue;
                    }
                    if (faction->m_settlements.size() < 5 && ratio > 1.8f && roman_ratio <= 1.2f
                        && !given_recently(MISSION_SUBJUGATE, faction_id, SecureInt(Scrambled(0xfbd6aaf4)).value())) {
                        view->m_plan = SENATE_PLAN_SUBJUGATE;   // 5 turns
                        continue;
                    }
                    if (view->m_targeted) {
                        view->m_plan = SENATE_PLAN_TARGETED;
                        continue;
                    }
                }
                if (ratio > 1.5f
                    && !given_recently(MISSION_EXACT_TRIBUTE, faction_id, SecureInt(Scrambled(0xfbd6aaf1)).value())) {
                    view->m_plan = SENATE_PLAN_EXACT_TRIBUTE;   // 10 turns
                    continue;
                }
                view->m_plan = SENATE_PLAN_ASSASSINATE;
                continue;
            case SENATE_VIEW_FRIEND:
                view->m_plan = SENATE_PLAN_FRIEND;
                continue;
            case SENATE_VIEW_5:
                view->m_plan = SENATE_PLAN_VIEW_5;
                continue;
            case SENATE_VIEW_NEUTRAL:
                if (borders || in_reach) {
                    if (hostile && roman_ratio < 0.5f) {
                        view->m_plan = SENATE_PLAN_APPEASE;
                        continue;
                    }
                    if (borders
                        && (m_player_family == 0
                            || !lbl_02921AC0->m_diplomacy.unk_005A0ABE(m_player_family->faction()->id(), faction_id))) {
                        view->m_plan = SENATE_PLAN_TRADE;
                        continue;
                    }
                    if (faction->m_settlements.size() > rome / 2
                        && !given_recently(MISSION_GET_MAP_INFO, faction_id, 20)) {
                        view->m_plan = SENATE_PLAN_GET_MAP_INFO;
                        continue;
                    }
                    view->m_plan = SENATE_PLAN_ASSASSINATE;
                    continue;
                }
                view->m_plan = SENATE_PLAN_NONE;
                continue;
            }
        }
        view->m_plan = SENATE_PLAN_NONE;
    }
}

void AISenateFaction::roman_strength(int* armies, int* fleets)
{
    // NONMATCHING - stack slot order differs
    *armies = army_strength(FACTION_SENATE);
    *fleets = fleet_strength(FACTION_SENATE);
    if (!view(FACTION_JULII).m_at_war_with_senate) {
        *armies += army_strength(FACTION_JULII);
        *fleets += fleet_strength(FACTION_JULII);
    }
    if (!view(FACTION_SCIPII).m_at_war_with_senate) {
        *armies += army_strength(FACTION_SCIPII);
        *fleets += fleet_strength(FACTION_SCIPII);
    }
    if (!view(FACTION_BRUTII).m_at_war_with_senate) {
        *armies += army_strength(FACTION_BRUTII);
        *fleets += fleet_strength(FACTION_BRUTII);
    }
}

unsigned int AISenateFaction::rome_settlements()
{
    int count = m_senate->m_settlements.size();
    if (family_faction(FACTION_JULII))
        count += family_faction(FACTION_JULII)->m_settlements.size();
    if (family_faction(FACTION_SCIPII))
        count += family_faction(FACTION_SCIPII)->m_settlements.size();
    if (family_faction(FACTION_BRUTII))
        count += family_faction(FACTION_BRUTII)->m_settlements.size();
    return count;
}

bool AISenateFaction::is_family_enemy(int faction)
{
    for (unsigned int i = 0; i < 3; i++) {
        if (family_faction(i) && family_enemy(i) == faction)
            return true;
    }
    return false;
}

// The family's senate standing: 50, then its share of Rome's military strength, its popularity, the human's missions,
// its leader's SenateStanding, and the senate's appreciation less its displeasure.
void AISenateFaction::update_senate_standing(SenateFamily* family)
{
    // NONMATCHING - stack slot order differs, and the original keeps the clamps' bounds and the faction in temporaries
    if (family->m_faction->stance(FACTION_SENATE) == STANCE_AT_WAR || family->m_faction->m_178 >= 2) {
        if (family->m_senate_standing != 0)
            family->m_senate_rising = family->m_senate_standing < 0;
        family->m_senate_standing = 0;
        return;
    }
    int armies = 0;
    int fleets = 0;
    roman_strength(&armies, &fleets);
    int total = armies + fleets;
    int strength = army_strength(family->m_faction->m_id) + fleet_strength(family->m_faction->m_id);
    int strength_term = 0;
    if (total != 0) {
        int share = strength * 100 / total;
        if (share < 33)
            strength_term = 0;
        else if (share >= 33 && share < 40)
            strength_term = 1;
        else if (share >= 40 && share < 45)
            strength_term = 2;
        else if (share >= 45 && share < 50)
            strength_term = 3;
        else if (share >= 50 && share < 57)
            strength_term = -1;
        else if (share >= 57 && share < 65)
            strength_term = -2;
        else if (share >= 65)
            strength_term = -3;
        strength_term *= 8;
    }
    int popularity = family->m_popularity;
    int popularity_term = 0;
    if (popularity < 30)
        popularity_term = 0;
    else if (popularity >= 30 && popularity < 39)
        popularity_term = 1;
    else if (popularity >= 39 && popularity < 48)
        popularity_term = 2;
    else if (popularity >= 48 && popularity < 57)
        popularity_term = 1;
    else if (popularity >= 57 && popularity < 66)
        popularity_term = -1;
    else if (popularity >= 66 && popularity < 75)
        popularity_term = -2;
    else if (popularity >= 75)
        popularity_term = -3;
    popularity_term *= 6;
    int missions_term = 0;
    int missions = m_missions_succeeded + m_missions_failed;
    if (missions > 0)
        missions_term = m_missions_succeeded * 8 / missions - 4;
    int leader_term = 0;
    if (family->m_faction->m_leader)
        leader_term = family->m_faction->m_leader->attribute(ATTR_SENATE_STANDING);
    leader_term = min_int(max_int(leader_term, 0), 4);
    ScrambledInt base(0xfbd6aac9);  // 50
    int standing = base.value() + strength_term + popularity_term + missions_term + leader_term
                 + family->m_appreciation - family->m_displeasure;
    standing = min_int(max_int(standing, 0), 99);
    if (family->m_senate_standing != standing)
        family->m_senate_rising = family->m_senate_standing < standing;
    family->m_senate_standing = standing;
}

SecureInt::SecureInt(int value)
{
    // NONMATCHING - the original reaches the member through a temporary, and reserves 8 more bytes of stack
    ScrambledInt scrambled(value, 0);
    m_value.m_scrambled = scrambled.m_scrambled;
}

SecureInt& SecureInt::operator+=(SecureInt& other)
{
    // NONMATCHING - the original reaches the member through temporaries, and reserves 0x10 more bytes of stack
    ScrambledInt b(other.m_value.value(), 0);
    unsigned int add = 0xa8b35b36;
    m_value.m_scrambled = apply(add, m_value, b).m_scrambled;
    return *this;
}

SecureInt SecureInt::operator-(SecureInt& other)
{
    // NONMATCHING - the original builds the result in the return slot; this copies it there
    ScrambledInt b(other.m_value.value(), 0);
    unsigned int subtract = 0xa8b35b37;
    return SecureInt(apply(subtract, m_value, b).value());
}

// Sorts the items from first to last (both included) so that none is lower than the next, by picking the highest
// left each time.
template <class It, class Lower>
void selection_sort(It first, It last, Lower lower)
{
    // NONMATCHING - the original advances the inner iterator through a temporary, and compares the other way round
    It end = last;
    ++end;
    for (It i = first; i != last; ++i) {
        It best = i;
        It j = i;
        for (++j; j != end; ++j) {
            if (!lower(*j, *best))
                best = j;
        }
        if (best != i) {
            OfficeCandidate swap = *i;
            *i = *best;
            *best = swap;
        }
    }
}

struct LowerWeight {
    bool operator()(const OfficeCandidate& a, const OfficeCandidate& b) const { return a.m_weight < b.m_weight; }
};

// The offices each turn: the terms that end today end, and so do those of the dead and of men whose faction is at
// war with a family or the senate. Then each office whose term ended today goes to one of the candidates, at random
// by their weights.
void AISenateFaction::update_offices()
{
    // NONMATCHING - stack slot order differs, and the original reserves 0xdc more bytes of stack
    unsigned int ending = 0;
    bool appointed_any = false;
    for (LinkedList<SenateOffice>::iterator it = s_offices.head(); it != LinkedList<SenateOffice>::iterator(); ++it) {
        SenateOffice& office = *it;
        if (office.m_holder != 0
            && (!office.m_holder->m_alive || office.m_date == lbl_018CD818->m_date
                || lbl_018CD818->senate_at_war_with(office.m_holder->m_faction))) {
            office.vacate();
            ending++;
        } else if (office.m_date == lbl_018CD818->m_date) {
            ending++;
        }
    }
    if (ending == 0)
        return;

    LinkedList<SenateOffice*> appointed;
    bool only_player = m_appointment_promised && !m_offices_withheld && m_player_family != 0;
    bool not_player = m_offices_withheld && !m_appointment_promised && m_player_family != 0;
    m_offices_withheld = false;
    m_appointment_promised = false;
    for (LinkedList<SenateOffice>::iterator it2 = s_offices.last(); it2 != LinkedList<SenateOffice>::iterator();
         --it2) {
        SenateOffice* office = &*it2;
        if (!(office->m_date == lbl_018CD818->m_date))
            continue;

        // The candidates: the three with the most weight from each family, or else the senate's own men.
        static Array<OfficeCandidate> candidates;
        candidates.clear();
        SecureInt total(Scrambled(0xfbd6aafb));     // 0
        for (SenateFamily* family = m_families; family != m_families + 3; family++) {
            Faction* faction = family->m_faction;
            if (faction == 0)
                continue;
            bool skip_others = only_player && m_player_family != 0 && family != m_player_family;
            bool skip_player = not_player && m_player_family != 0 && family == m_player_family;
            if (lbl_018CD818->senate_at_war_with(faction) || skip_others || skip_player)
                continue;
            static Array<OfficeCandidate> family_candidates;
            family_candidates.clear();
            for (unsigned int i = 0; i < faction->m_characters.size(); i++) {
                Character* character = faction->m_characters[i];
                if (character->type() == CHARACTER_NAMED && office->eligible(character)) {
                    int weight = (family->m_popularity >> 3) + character->m_record->attribute_at(ATTR_ELECTABILITY)
                               + character->m_record->attribute_at(ATTR_INFLUENCE);
                    if (weight > 0) {
                        family_candidates.add();
                        family_candidates.back().m_character = character;
                        family_candidates.back().m_weight = weight;
                    }
                }
            }
            unsigned int count = 3;
            LowerWeight lower;
            if (family_candidates.size() > count)
                selection_sort(family_candidates.begin(), family_candidates.last(), lower);
            for (OfficeCandidate* candidate = family_candidates.data(); count > 0; count--) {
                if (candidate == family_candidates.data() + family_candidates.size())
                    break;
                candidates.push_back(*candidate);
                total += SecureInt((int)candidate->m_weight);
                candidate++;
            }
        }
        if (candidates.size() == 0) {
            for (unsigned int i = 0; i < m_senate->m_characters.size(); i++) {
                Character* character = m_senate->m_characters[i];
                if (character->type() == CHARACTER_NAMED && office->out_of_office(character)) {
                    int weight = character->m_record->attribute_at(ATTR_ELECTABILITY) + 12;
                    weight = min_int(max_int(weight, 1), 20);
                    candidates.add();
                    candidates.back().m_character = character;
                    candidates.back().m_weight = weight;
                    total += SecureInt(weight);
                }
            }
        }

        SecureInt zero(Scrambled(0xfbd6aafb));
        if (total.value() > zero.value()) {
            SecureInt one(Scrambled(0xfbd6aa30));
            int pick = lbl_018CD818->m_random.range(0, (total - one).value());
            for (unsigned int i = 0; i < candidates.size(); i++) {
                pick -= candidates[i].m_weight;
                if (pick <= 0) {
                    office->appoint(candidates[i].m_character->m_record);
                    appointed_any = true;
                    if (m_player_family != 0
                        && office->m_holder->m_faction->m_id == m_player_family->m_faction->m_id)
                        appointed.push_back(office);
                    break;
                }
            }
        } else {
            Character* character = fn_008E24E2();
            if (character != 0) {
                office->appoint(character->m_record);
                appointed_any = true;
            }
        }
    }
    if (appointed_any && m_player_family != 0 && lbl_018CD818->m_turn > 1)
        fn_00CBB01C(&appointed);
}

// The missions step: what the senate makes of last turn's events, the missions it gives the human's family, and the
// one it picks at random from the candidates its plans for the other factions give.
void AISenateFaction::unk_00530A0D()
{
    // NONMATCHING - stack slot order differs, the original reserves 0x134 more bytes of stack, and it walks the faction
    // events' list once more for each access
    if (m_player_family->at_war_with_senate())
        return;
    SenateFamily* family = m_player_family;
    Faction* faction = family->m_faction;
    int faction_id = faction->m_id;
    faction->unk_004D13E8(this);

    // Last turn's faction events that the senate hasn't judged yet.
    LinkedList<FactionEventRecord*>* events = &lbl_014D5F70;
    if (events->size() > 0) {
        Date last_turn = lbl_018CD818->m_date;
        last_turn.previous_turn();
        unsigned int i = events->size() - 1;
        while (i > 0 && (*events)[i]->m_date >= last_turn)
            i--;
        if ((*events)[i]->m_date != last_turn)
            i++;
        while (i != (unsigned int)events->size()) {
            if ((*events)[i]->m_judged)
                break;
            (*events)[i]->judge(this, family);
            FactionEventRecord* event = (*events)[i];
            event->m_judged = true;
            if (!event->release())
                i++;
        }
    }

    // Leave_Region: an army of the family's in a region of a faction the senate likes.
    unsigned int num_armies = faction->m_armies.size();
    unsigned int oldest = lbl_018CD818->m_turn - 3;
    LinkedList<LeaveRegionRequest>::iterator request = m_leave_requests.head();
    while (request != LinkedList<LeaveRegionRequest>::iterator()) {
        if ((*request).m_turn < oldest)
            request = m_leave_requests.erase(request);
        else
            ++request;
    }
    unsigned int i;
    for (i = 0; i < num_armies; i++) {
        Army* army = faction->m_armies[i];
        if (army == 0)
            continue;
        Region* region = army->unk_007199A3();
        int region_id = region->id();
        Faction* owner = region->m_owner;
        if (owner == 0)
            continue;
        LinkedList<LeaveRegionRequest>::iterator asked = m_leave_requests.head();
        for (; asked != LinkedList<LeaveRegionRequest>::iterator(); ++asked) {
            if ((*asked).m_region == region_id)
                break;
        }
        if (asked != LinkedList<LeaveRegionRequest>::iterator())
            continue;
        int state = m_views[owner->m_id].m_state;
        if (state != SENATE_VIEW_FRIEND && state != SENATE_VIEW_5)
            continue;
        if (lbl_018CD818->m_diplomacy.m_factions[faction_id].m_relations[owner->m_id].m_flags & 2)
            continue;
        offer(GivenMission(MISSION_LEAVE_REGION, owner->m_id),
              fn_00547F88(SenateMissionRequest(this, faction, family->standing_level(), m_player_family->penalty(), 2),
                          region));
        LeaveRegionRequest asked_now;
        asked_now.m_region = region_id;
        asked_now.m_turn = lbl_018CD818->m_turn;
        m_leave_requests.append(asked_now);
    }

    // Demand_Suicide, with the Outlaw penalty.
    if (family->standing_level() == 0 && unk_44()->unk_18(faction_id) && faction->m_heir != 0
        && !given_recently(MISSION_DEMAND_SUICIDE, faction_id, 3)) {
        offer(GivenMission(MISSION_DEMAND_SUICIDE, faction_id),
              fn_00547F56(SenateMissionRequest(this, faction, family->standing_level(), fn_0054610E(1, faction), 0)));
    }

    // The first turn's Occupy_City for each family.
    if (lbl_018CD818->m_turn == 0 && !lbl_018CD818->m_prologue) {
        String* name = &fn_008F60BF(faction_id)->m_name;
        if (*name == String("romans_julii")) {
            offer(GivenMission(MISSION_OCCUPY_CITY, faction_id),
                  fn_005480AC(SenateMissionRequest(this, faction, family->standing_level(), m_player_family->reward(), 10),
                              lbl_018CD818->unk_004543F1("Segesta")));
        } else if (*name == String("romans_brutii")) {
            offer(GivenMission(MISSION_OCCUPY_CITY, faction_id),
                  fn_005480AC(SenateMissionRequest(this, faction, family->standing_level(), m_player_family->reward(), 10),
                              lbl_018CD818->unk_004543F1("Apollonia")));
        } else if (*name == String("romans_scipii")) {
            offer(GivenMission(MISSION_OCCUPY_CITY, faction_id),
                  fn_005480AC(SenateMissionRequest(this, faction, family->standing_level(), m_player_family->reward(), 10),
                              lbl_018CD818->unk_004543F1("Syracuse")));
        }
    }

    // No other mission the turn after one fails, or while the family has one under way.
    if (m_mission_failed) {
        m_mission_failed = false;
        return;
    }
    if (faction->m_missions.size() > 0 || lbl_018CD818->m_prologue)
        return;

    LinkedList<SenateCandidate*> candidates;
    for (i = 0; i < NUM_FACTIONS; i++) {
        Faction* other = lbl_018CD818->faction(i);
        if (other == 0)
            continue;
        Faction* target = other;
        if (target->m_id == faction->m_id)
            continue;
        switch (m_views[i].m_plan) {
        case SENATE_PLAN_BLOCKADE: {
            // A port of theirs that a fleet of the family's, or a port of its own, can reach in under 5 turns.
            unsigned int num_ports = target->m_ports.size();
            unsigned int num_own_ports = faction->m_ports.size();
            unsigned int num_fleets = faction->m_armies.size();
            for (unsigned int j = 0; j < num_ports; j++) {
                SettlementUnk184* port = target->m_ports[j];
                if (port == 0 || port->m_3c <= 0 || port->unk_0049E979())
                    continue;
                unsigned int k;
                for (k = 0; k < num_fleets; k++) {
                    Army* fleet = faction->m_armies[k];
                    if (fleet == 0 || !fleet->unk_ac())
                        continue;
                    SeaRoute* route = sea_routes()->unk_0049060E(1, fleet->unk_007216D2(0, 10, port->position()));
                    if (route == 0 || route->m_turns >= 5)
                        continue;
                    int weight = 15 - route->m_turns;
                    if (i == family->m_enemy)
                        weight += 5;
                    if (weight <= 0)
                        continue;
                    int difficulty = fn_0054F8AD(port, fleet, route->m_turns);
#line 4122
                    candidates.push_back(NEW BlockadePortCandidate(difficulty, weight, port));
                }
                for (k = 0; k < num_own_ports; k++) {
                    SettlementUnk184* own_port = faction->m_ports[k];
                    if (own_port == 0)
                        continue;
                    SeaRoute* route = sea_routes()->unk_004908F6(1, 3, own_port->position(), 0, 10, port->position());
                    if (route == 0 || route->m_turns >= 5)
                        continue;
                    int weight = 15 - route->m_turns;
                    if (i == family->m_enemy)
                        weight += 5;
                    if (weight <= 0)
                        continue;
                    int difficulty = fn_0054F8B4(port, own_port, route->m_turns);
#line 4148
                    candidates.push_back(NEW BlockadePortCandidate(difficulty, weight, port));
                }
            }
            break;
        }
        case SENATE_PLAN_ASSASSINATE: {
            LinkedList<Character*> characters;
            target->unk_004D6EC0(&characters, CHARACTER_NAMED);
            for (LinkedList<Character*>::iterator it = characters.head(); it != LinkedList<Character*>::iterator(); ++it) {
                Character* character = *it;
                int difficulty = fn_0054C9A1(faction, character->m_record);
                if (difficulty == 3)
                    continue;
                int weight = 19;
                if (target->m_id == family->m_enemy)
                    weight += 5;
                weight -= difficulty;
                if (weight <= 0)
                    continue;
#line 4197
                candidates.push_back(NEW AssassinationCandidate(difficulty, weight, character->m_record));
            }
            break;
        }
        case SENATE_PLAN_EXACT_TRIBUTE: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_005572F2(faction, target, &cost);
            if (difficulty == 3)
                break;
            int weight = 20 - cost - difficulty;
            if (weight <= 0)
                break;
#line 4232
            candidates.push_back(NEW ExactTributeCandidate(difficulty, weight, target));
            break;
        }
        case SENATE_PLAN_SUBJUGATE: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_005609FC(faction, target, &cost);
            if (difficulty == 3)
                break;
            int weight = 20 - cost - difficulty;
            if (weight <= 0)
                break;
#line 4255
            candidates.push_back(NEW SubjugateCandidate(difficulty, weight, target));
            break;
        }
        case SENATE_PLAN_ANNEX: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_0054A550(faction, target, &cost);
            if (difficulty == 3)
                break;
            // The best is never lowered, so the city is the settlement of the last of their regions.
            float best = FLT_MAX;
            Settlement* city = 0;
            Array<Region*> own_regions;
            Array<Region*> their_regions;
            faction->unk_004D0905(&own_regions);
            target->unk_004D0905(&their_regions);
            for (Array<Region*>::iterator own = own_regions.begin(); own != own_regions.end(); ++own) {
                Region* from = *own;
                for (Array<Region*>::iterator their = their_regions.begin(); their != their_regions.end(); ++their) {
                    Region* to = *their;
                    Array<int> path;
                    float route_cost = 0.0f;
                    lbl_029219C4->unk_004B13A2(1, from->id(), to->id(), &path, &route_cost, 0);
                    if (route_cost < best)
                        city = to->settlement();
                }
            }
            int weight = 20 - cost - difficulty;
            if (weight > 0) {
#line 4311
                candidates.push_back(NEW AnnexCityCandidate(difficulty, weight, city));
            }
            break;
        }
        case SENATE_PLAN_GET_MAP_INFO: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_00558B3F(faction, target, &cost);
            if (difficulty == 3)
                break;
            int weight = 20 - cost - difficulty;
            if (weight <= 0)
                break;
#line 4334
            candidates.push_back(NEW GetMapInfoCandidate(difficulty, weight, target));
            break;
        }
        case SENATE_PLAN_TRADE: {
            // Only with a faction whose region borders the family's, and that it doesn't trade with yet.
            if (target->m_description->unk_a7())
                break;
            if (lbl_02921AC0->m_diplomacy.unk_005A0ABE(faction->m_id, i))
                break;
            Array<int>* borders = &faction->m_bordering_regions;
            for (unsigned int j = 0; j < borders->size(); j++) {
                Faction* owner = lbl_018CD80C->m_regions[(*borders)[j]].m_owner;
                if (owner == 0 || owner->m_id != target->m_id)
                    continue;
                int cost;
                int difficulty = fn_0055DE42(faction, target, &cost);
                if (difficulty != 3) {
                    int weight = 20 - cost - difficulty;
                    if (weight > 0) {
#line 4369
                        candidates.push_back(NEW TradeAgreementCandidate(difficulty, weight, target));
                    }
                }
                break;
            }
            break;
        }
        case SENATE_PLAN_APPEASE: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_0054B3ED(faction, target, &cost);
            if (difficulty == 3)
                break;
            int weight = 20 - cost - difficulty;
            if (weight <= 0)
                break;
#line 4395
            candidates.push_back(NEW AppeaseCandidate(difficulty, weight, target));
            break;
        }
        case SENATE_PLAN_BROKER_PEACE: {
            if (target->m_description->unk_a7())
                break;
            int cost;
            int difficulty = fn_005516FE(faction, target, &cost);
            if (difficulty == 3)
                break;
            int weight = 20 - cost - difficulty;
            if (weight <= 0)
                break;
#line 4424
            candidates.push_back(NEW BrokerPeaceCandidate(difficulty, weight, target));
            break;
        }
        case SENATE_PLAN_EXTERMINATE:
        case SENATE_PLAN_ENSLAVE:
        case SENATE_PLAN_OCCUPY: {
            // Each of their settlements that a region of the family's is near: by a land route of fewer than 3 steps,
            // or 2 turns' march.
            Array<Region*> own_regions;
            Array<int> path;
            int region_id = 0;
            int extra = 0;
            float best = 0.0f;
            float turn_cost = 0.0f;
            lbl_029219C4->unk_004ACEB2(&turn_cost);
            faction->unk_004D0905(&own_regions);
            unsigned int num_settlements = target->m_regions.size();
            for (unsigned int j = 0; j < num_settlements; j++) {
                extra = -1;
                best = FLT_MAX;
                region_id = target->m_settlements[j]->region_id();
                Region* region = target->m_settlements[j]->unk_b4();
                for (Array<Region*>::iterator own = own_regions.begin(); own != own_regions.end(); ++own) {
                    int from = (*own)->id();
                    float route_cost = 0.0f;
                    float march_cost = 0.0f;
                    int turns = 0;
                    bool by_route = lbl_029219C4->unk_004B13A2(1, from, region_id, &path, &route_cost, 0);
                    bool by_march = fn_0051E6C7(*own, region, &turns);
                    if (by_march)
                        march_cost = (float)turns * turn_cost;
                    if (by_route && route_cost < best && path.size() < 3) {
                        best = route_cost;
                        extra = path.size() * 2;
                    }
                    if (by_march && march_cost < best && turns <= 2) {
                        best = march_cost;
                        extra = turns;
                    }
                }
                if (best >= FLT_MAX)
                    continue;
                Settlement* city = lbl_018CD80C->m_regions[region_id].settlement();
                int difficulty = fn_005621DA(faction, city, 1);
                if (difficulty == 3)
                    continue;
                int weight = 15 - extra - difficulty * 2;
                if (target->m_id == family->m_enemy)
                    weight += 5;
                weight -= difficulty;
                if (weight <= 0)
                    continue;
                switch (m_views[i].m_plan) {
                case SENATE_PLAN_OCCUPY:
#line 4536
                    candidates.push_back(NEW OccupyCityCandidate(difficulty, weight, city));
                    break;
                case SENATE_PLAN_ENSLAVE:
#line 4539
                    candidates.push_back(NEW EnslaveCityCandidate(difficulty, weight, city));
                    break;
                case SENATE_PLAN_EXTERMINATE:
#line 4542
                    candidates.push_back(NEW ExterminateCityCandidate(difficulty, weight, city));
                    break;
                }
            }
            break;
        }
        }
    }

    if (candidates.head() != 0) {
        // One of them, each as likely as its weight.
        int total = 0;
        LinkedList<SenateCandidate*>::iterator it;
        for (it = candidates.head(); it != LinkedList<SenateCandidate*>::iterator(); ++it)
            total += (*it)->m_weight;
        int roll = lbl_018CD818->m_random.up_to(total);
        for (it = candidates.head(); it != LinkedList<SenateCandidate*>::iterator(); ++it) {
            roll -= (*it)->m_weight;
            if (roll <= 0) {
                (*it)->give(faction, family->standing_level(), this);
                break;
            }
        }
    } else if (!given_recently(MISSION_HELP_PLAYER, faction_id, 20) && lbl_018CD818->m_turn > 10
               && m_player_family->standing_level() > 3) {
        int column = 0;
        int code = fn_0055B89C(family->standing_level(), column);
        if (code != 0) {
            offer(GivenMission(MISSION_HELP_PLAYER, faction_id),
                  fn_00547F48(SenateMissionRequest(this, faction, family->standing_level(), fn_0054610E(code, faction), 0)));
        }
    }
    for (LinkedList<SenateCandidate*>::Node* node = candidates.tail(); node != 0; node = node->m_prev) {
#line 4600
        CA_DELETE(node->m_data);
    }
}

// The senate gives the family a mission (if its builder made one) and remembers that it did.
void AISenateFaction::offer(const GivenMission& given, SenateMissionImpl* mission)
{
    if (mission) {
        m_given_missions.append(given);
        mission->unk_24();
        mission->unk_00();
    }
}

// Whether it gave the family a mission of this type fewer than this many turns ago.
bool AISenateFaction::given_recently(int type, int family, unsigned int turns)
{
    for (LinkedList<GivenMission>::iterator it = m_given_missions.head(); it != LinkedList<GivenMission>::iterator(); ++it) {
        GivenMission& mission = *it;
        if (mission.m_type == type && mission.m_family == family &&
            lbl_018CD818->m_date.turns_since(mission.m_date) < turns)
            return true;
    }
    return false;
}

// What the missions' tables call its attitude: floor(standing * 0.07).
int SenateFamily::standing_level()
{
    // NONMATCHING - stack layout: the original has one more int temporary for the result
    float scaled = 0.07f * m_senate_standing;
    int level = float_to_int(floor(scaled));
    return level;
}

bool SenateFamily::at_war_with_senate()
{
    // NONMATCHING - the original has no slot for status: it keeps the stance in the inline's slot
    int family = m_faction->m_id;
    int senate = FACTION_SENATE;
    int status = lbl_018CD818->m_diplomacy.stance(senate, family);
    return status >= STANCE_AT_WAR;
}

// What the candidates give: the mission, unless its table's reward code at this level and difficulty is 0 (none).
void BlockadePortCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - the original keeps the owner in one more temporary
    int code = fn_0054F8C2(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->unk_0049E4B4()->m_id;
        senate->offer(GivenMission(MISSION_BLOCKADE_PORT, owner),
                      fn_00547F9A(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void OccupyCityCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    int code = fn_005621C5(level, m_difficulty);
    if (code != 0) {
        senate->offer(GivenMission(MISSION_OCCUPY_CITY, m_target->owner_id()),
                      fn_005480AC(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void EnslaveCityCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    int code = fn_005621C5(level, m_difficulty);
    if (code != 0) {
        senate->offer(GivenMission(MISSION_ENSLAVE_CITY, m_target->owner_id()),
                      fn_005480BE(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void ExterminateCityCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    int code = fn_005621C5(level, m_difficulty);
    if (code != 0) {
        senate->offer(GivenMission(MISSION_EXTERMINATE_CITY, m_target->owner_id()),
                      fn_005480D0(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void AssassinationCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_0054CADD(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_faction->m_id;
        senate->offer(GivenMission(MISSION_ASSASSINATION, owner),
                      fn_00547FAC(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void ExactTributeCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_0055730D(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_EXACT_TRIBUTE, owner),
                      fn_0054800A(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void SubjugateCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_00560A17(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_SUBJUGATE, owner),
                      fn_0054801C(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void GetMapInfoCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_00558B5A(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_GET_MAP_INFO, owner),
                      fn_00548040(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void AppeaseCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_0054B408(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_APPEASE, owner),
                      fn_00548064(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void TradeAgreementCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_0055DE5D(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_MAKE_TRADE_AGREEMENT, owner),
                      fn_00548052(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void BrokerPeaceCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    // NONMATCHING - stack slot order differs (the owner's index)
    int code = fn_00551719(level, m_difficulty);
    if (code != 0) {
        int owner = m_target->m_id;
        senate->offer(GivenMission(MISSION_BROKER_PEACE, owner),
                      fn_00548076(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

void AnnexCityCandidate::give(Faction* faction, int level, AISenateFaction* senate)
{
    int code = fn_0054A56B(level, m_difficulty);
    if (code != 0) {
        senate->offer(GivenMission(MISSION_ANNEX_CITY, m_target->owner_id()),
                      fn_0054802E(SenateMissionRequest(senate, faction, level, fn_0054610E(code, faction), 0), m_target));
    }
}

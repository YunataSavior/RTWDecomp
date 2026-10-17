#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\Ai\\ai_senate_faction.cpp"
// The senate: descr_senate.txt, the Roman families' senate standing and popularity, and the senate's turn. (The
// original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"

#include "core_linked_list.h"
#include "../character.h"
#include "../character_record.h"
#include "../landmark.h"
#include "ai_senate_faction.h"

enum {
    FACTION_JULII = 0,
    FACTION_BRUTII = 1,
    FACTION_SCIPII = 2,
    FACTION_SENATE = 3,
    NUM_FACTIONS = 21,
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
inline int army_strength(int faction)
{
    return lbl_012D0F74[faction].m_strength;
}

inline int fleet_strength(int faction)
{
    return lbl_012D0F68[faction].m_strength;
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
    : m_title(title), m_text(text), m_rank(rank), m_c(0), m_date(lbl_018CD818->m_date), m_type(type),
      m_duration(duration), m_sittings(sittings), m_restrictions(restrictions), m_benefits(benefits)
{
    // NONMATCHING - the original keeps the campaign's date in a temporary, read before the other members are set
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
    m_18 = 0x10;
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
    unk_0052FFC8();
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

// The families' standings each turn, and the advisor's words when the human's family's change.
void AISenateFaction::update_families()
{
    // NONMATCHING - stack slot order differs, and the original indexes each family with a constant it loads into a
    // register
    Faction* julii = m_families[FACTION_JULII].m_faction;
    Faction* scipii = m_families[FACTION_SCIPII].m_faction;
    Faction* brutii = m_families[FACTION_BRUTII].m_faction;
    for (unsigned int i = 0; i < NUM_FACTIONS; i++)
        m_views[i].reset();
    if (julii)
        unk_0052E7D2(julii->m_ai);
    if (scipii)
        unk_0052E7D2(scipii->m_ai);
    if (brutii)
        unk_0052E7D2(brutii->m_ai);
    unk_0052E911();
    for (unsigned int j = 0; j < NUM_FACTIONS; j++)
        m_views[j].update_state(s_tolerance[j]);
    if (julii)
        m_families[FACTION_JULII].m_enemy = choose_enemy(julii);
    if (scipii)
        m_families[FACTION_SCIPII].m_enemy = choose_enemy(scipii);
    if (brutii)
        m_families[FACTION_BRUTII].m_enemy = choose_enemy(brutii);
    unk_0052EDA6();
    if (julii) {
        m_families[FACTION_JULII].update_popularity();
        update_senate_standing(&m_families[FACTION_JULII]);
        if (&m_families[FACTION_JULII] != m_player_family)
            m_families[FACTION_JULII].check_gift();
    }
    if (scipii) {
        m_families[FACTION_SCIPII].update_popularity();
        update_senate_standing(&m_families[FACTION_SCIPII]);
        if (&m_families[FACTION_SCIPII] != m_player_family)
            m_families[FACTION_SCIPII].check_gift();
    }
    if (brutii) {
        m_families[FACTION_BRUTII].update_popularity();
        update_senate_standing(&m_families[FACTION_BRUTII]);
        if (&m_families[FACTION_BRUTII] != m_player_family)
            m_families[FACTION_BRUTII].check_gift();
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

void AISenateFaction::roman_strength(int* armies, int* fleets)
{
    // NONMATCHING - the original indexes the families' views with a constant it loads into a register
    *armies = army_strength(FACTION_SENATE);
    *fleets = fleet_strength(FACTION_SENATE);
    if (!at_war_with_senate(FACTION_JULII)) {
        *armies += army_strength(FACTION_JULII);
        *fleets += fleet_strength(FACTION_JULII);
    }
    if (!at_war_with_senate(FACTION_SCIPII)) {
        *armies += army_strength(FACTION_SCIPII);
        *fleets += fleet_strength(FACTION_SCIPII);
    }
    if (!at_war_with_senate(FACTION_BRUTII)) {
        *armies += army_strength(FACTION_BRUTII);
        *fleets += fleet_strength(FACTION_BRUTII);
    }
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

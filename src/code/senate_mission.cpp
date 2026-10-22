#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\senate_mission.cpp"
// The senate's missions: the rewards and penalties they end with, descr_senate.txt's mission part, and the base
// classes every mission type (code\sm_*.cpp) builds on.
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"
#include "unit_db.h"
#include "core_strings.h"
#include "campaign_triggers.h"
#include "faction.h"
#include "faction_economics.h"
#include "romans_utils.h"
#include "version_registry.h"
#include "world.h"
#include "map_constants.h"
#include "hash_map.h"
#include "maths_random.h"
#include "character.h"
#include "character_record.h"
#include "senate_mission.h"

enum {
    CHARACTER_SPY = 0,
    CHARACTER_ASSASSIN = 1,
    CHARACTER_DIPLOMAT = 2,
    CHARACTER_GENERAL = 4,
    CHARACTER_NAMED = 5,
};

enum {
    MISSION_CANCELLED = 2,              // over with neither outcome (a guess)
};

// The finance record's line the senate's fines and cash rewards are booked under.
enum {
    FINANCE_SENATE = 6,
};

// The texts the senate's fines and cash rewards are told with (what fn_00D926DB gives; not identified).
class SenateMoneyTexts {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
    virtual void unk_20();
    virtual void unk_24();
    virtual UniString major_fine(int amount);       // +0x28
    virtual UniString fine(int amount);             // +0x2c
    virtual UniString minor_fine(int amount);       // +0x30
    virtual UniString cash(int amount);             // +0x34
};

// What a mission's end shows: the mission, the message's kind, and its texts.
struct SenateMissionNote {
    SenateMissionNote(void* title, int kind, void* name, void* text)
        : m_title(title), m_kind(kind), m_name(name), m_text(text)
    {
    }

    void* m_title;
    int m_kind;
    void* m_name;
    void* m_text;
};

struct SenateMissionVerdict : SenateMissionNote {
    SenateMissionVerdict(void* title, int kind, void* name, void* text, UniString* payback)
        : SenateMissionNote(title, kind, name, text), m_payback(payback)
    {
    }

    UniString* m_payback;               // +0x10: what its reward or penalty did, or 0
};

typedef SenateMissionImpl* (*MissionLoader)(SaveStream& stream);

// The mission types' parts of descr_senate.txt that aren't only a table, and their tables.
class DemandSuicideBuilder {
public:
    static SenateMissionImpl* load(SaveStream& stream);
    static SenateRewardTable s_rewards;
};

extern "C" {
void* fn_00D93A0C(int text);            // a senate text by its number (a guess)
int fn_008F649F();                      // the index of the faction "romans_senate"
SenateMoneyTexts* fn_00D926DB();
void fn_00CB639F(Faction* faction, SenateMissionInfo* info, int);       // a mission's message
void fn_00CB634F(Faction* faction, SenateMissionNote* note, int);       // a mission's end with neither outcome
void fn_00CB6486(Faction* faction, SenateMissionVerdict* verdict, RewardPlace place);  // a mission's success
void fn_00CB656B(Faction* faction, SenateMissionVerdict* verdict, int); // a mission's failure
void* fn_00413901(void* saved);         // a saved pointer's object
void fn_00553C9B();
void fn_005508A3();
void fn_00554A08();
void fn_00553C83();
void fn_0055088B();
void fn_005549F0();
void fn_0056218B();

bool fn_0054C825(ScriptReader& reader);  // Assassinate (code\sm_assassination.cpp)
bool fn_005507B0(ScriptReader& reader);  // Break_Alliance
bool fn_00553BA8(ScriptReader& reader);  // Cease_Hostilities
bool fn_00554915(ScriptReader& reader);  // Declare_War
bool fn_0054F877(ScriptReader& reader);  // Blockade_Port
bool fn_005531DD(ScriptReader& reader);  // Capture_Rebel_City
bool fn_005620F2(ScriptReader& reader);  // Take_City
extern SenateRewardTable lbl_012D8BA0;  // Help_Player's
extern SenateRewardTable lbl_012D7B40;  // Attack_Outlaw_Faction's
extern SenateRewardTable lbl_012D8988;  // Give_Back_City's
extern SenateRewardTable lbl_012D8640;  // Exact_Tribute's
extern SenateRewardTable lbl_012D9300;  // Subjugate's
extern SenateRewardTable lbl_012D7EE0;  // Broker_Peace's
extern SenateRewardTable lbl_012D8F68;  // Make_Trade_Agreement's
extern SenateRewardTable lbl_012D87F0;  // Get_Map_Info's
extern SenateRewardTable lbl_012D7800;  // Appease's
extern SenateRewardTable lbl_012D8DB8;  // Make_Alliance's
extern SenateRewardTable lbl_012D8A08;  // Give_Cash's
extern SenateRewardTable lbl_012D7788;  // Annex_City's
extern SenateRewardTable lbl_012D8530;  // Destroy_Rebel_City's

// Each mission type's builder from a save, by its type.
SenateMissionImpl* fn_005506AC(SaveStream& stream);
SenateMissionImpl* fn_0055310B(SaveStream& stream);
SenateMissionImpl* fn_00555F9F(SaveStream& stream);
SenateMissionImpl* fn_0055996C(SaveStream& stream);
SenateMissionImpl* fn_0054EB5D(SaveStream& stream);
SenateMissionImpl* fn_00553AA4(SaveStream& stream);
SenateMissionImpl* fn_00554809(SaveStream& stream);
SenateMissionImpl* fn_0055B7DA(SaveStream& stream);
SenateMissionImpl* fn_0054C512(SaveStream& stream);
SenateMissionImpl* fn_00561842(SaveStream& stream);
SenateMissionImpl* fn_0055F10D(SaveStream& stream);
SenateMissionImpl* fn_0055FF48(SaveStream& stream);
SenateMissionImpl* fn_005568C1(SaveStream& stream);
SenateMissionImpl* fn_005580F9(SaveStream& stream);
SenateMissionImpl* fn_00552559(SaveStream& stream);
SenateMissionImpl* fn_0054DB35(SaveStream& stream);
SenateMissionImpl* fn_0055C5BC(SaveStream& stream);
SenateMissionImpl* fn_0054F724(SaveStream& stream);
SenateMissionImpl* fn_0055720C(SaveStream& stream);
SenateMissionImpl* fn_00560916(SaveStream& stream);
SenateMissionImpl* fn_0054A471(SaveStream& stream);
SenateMissionImpl* fn_00558A59(SaveStream& stream);
SenateMissionImpl* fn_0055DD5C(SaveStream& stream);
SenateMissionImpl* fn_0054B307(SaveStream& stream);
SenateMissionImpl* fn_00551618(SaveStream& stream);
}

inline const UniString& senate_text(int text)
{
    return *(const UniString*)fn_00D93A0C(text);
}

// descr_senate.txt's fines (each divides the faction's net income) and cash rewards (in denari).
static unsigned int s_major_fine;       // 0x012D76B4
static unsigned int s_moderate_fine;    // 0x012D76B8
static unsigned int s_minor_fine;       // 0x012D76BC
static unsigned int s_major_reward;     // 0x012D76C0
static unsigned int s_moderate_reward;  // 0x012D76C4
static unsigned int s_minor_reward;     // 0x012D76C8
// descr_senate.txt's bounds on the ratio of strengths that makes taking a city a mission of difficulty E, M or D
// (code\sm_take_city.cpp's 0x005621DA, for the city missions).
static float s_rebel_city_lower_boundary;   // 0x012D76CC
static float s_rebel_city_higher_boundary;  // 0x012D76D0
static float s_take_city_lower_boundary;    // 0x012D76D4
static float s_take_city_higher_boundary;   // 0x012D76D8

// All four give rebel_city_lower_boundary, so the other three bounds are read but never used, and M is given only at
// exactly that ratio.
float rebel_city_lower_boundary()
{
    return s_rebel_city_lower_boundary;
}

float rebel_city_higher_boundary()
{
    return s_rebel_city_lower_boundary;
}

float take_city_lower_boundary()
{
    return s_rebel_city_lower_boundary;
}

float take_city_higher_boundary()
{
    return s_rebel_city_lower_boundary;
}

// The rewards and penalties by name.
typedef HashMap<TextKey, SenateReward*, TextHash, TextEqual> RewardMap;
static RewardMap* s_reward_map;         // 0x012D76DC
static char s_deleted_key;              // 0x012D76E0
static char s_unused_key;               // 0x012D76E4

static RewardMap* rewards();

// A reward or penalty, by its index and name text.
template <int INDEX, int TITLE>
class Payback : public SenateReward {
public:
    Payback(const char* name) : SenateReward(name) {}
    virtual void* title() { return fn_00D93A0C(TITLE); }
    virtual SaveStream& save(SaveStream& stream)
    {
        if (!stream.unk_00F3D990())
            return stream;
        int index = INDEX;
        stream.write(&index, 4);
        return stream;
    }
};

template <int INDEX, int TITLE>
class Penalty : public Payback<INDEX, TITLE> {
public:
    Penalty(const char* name) : Payback<INDEX, TITLE>(name) {}
    virtual bool is_benefit() { return false; }
};

template <int INDEX, int TITLE>
class Benefit : public Payback<INDEX, TITLE> {
public:
    Benefit(const char* name) : Payback<INDEX, TITLE>(name) {}
    virtual bool is_benefit() { return true; }
};

#define SENATE_PENALTY(cls, index, title, name)                                                                     \
    class cls : public Penalty<index, title> {                                                                      \
    public:                                                                                                         \
        cls() : Penalty<index, title>(name) {}                                                                      \
        virtual UniString* punish(AISenateFaction* senate, Faction* faction, UniString& text);                      \
    }

#define SENATE_BENEFIT(cls, index, title, name)                                                                     \
    class cls : public Benefit<index, title> {                                                                      \
    public:                                                                                                         \
        cls() : Benefit<index, title>(name) {}                                                                      \
        virtual UniString* reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place);   \
    }

// The senate declares war on the faction.
SENATE_PENALTY(OutlawPenalty, REWARD_OUTLAW, 0xe1, "outlaw");
// One of the family's men, not the leader, is killed (when it has more than one).
SENATE_PENALTY(CensorFamilyPenalty, REWARD_CENSOR_INVESTIGATES_FAMILY, 0xe3, "censor_investigates_family");
// One of its spies, assassins, diplomats or generals is killed.
SENATE_PENALTY(CensorFactionPenalty, REWARD_CENSOR_INVESTIGATES_FACTION, 0xe7, "censor_investigates_faction");
// Fines: a share of the turn's net income, by major_fine, moderate_fine and minor_fine.
SENATE_PENALTY(QuaestorCorruptionPenalty, REWARD_QUAESTOR_CORRUPTION, 0xe5, "quaestor_investigates_for_corruption");
SENATE_PENALTY(QuaestorFinancialPenalty, REWARD_QUAESTOR_FINANCIAL, 0xe9,
               "quaestor_investigates_for_financial_irregularities");
SENATE_PENALTY(QuaestorAccountingPenalty, REWARD_QUAESTOR_ACCOUNTING, 0xeb,
               "quaestor_investigates_for_accounting_irregularities");
// The senate's displeasure starts (its date is only kept).
SENATE_PENALTY(DispleasurePenalty, REWARD_SENATE_DISPLEASURE, 0xed, "senate_displeasure");
// The SenateExposure event, for the leader's triggers.
SENATE_PENALTY(ExposurePenalty, REWARD_SECRET_INFORMATION_EXPOSURE, 0xee, "secret_information_exposure");
// The family is left out of the next appointments.
SENATE_PENALTY(OfficesWithheldPenalty, REWARD_OFFICES_WITHHELD, 0xf0, "offices_witheld");
// Units.
SENATE_BENEFIT(MajorExoticUnitReward, REWARD_MAJOR_EXOTIC_UNIT, 0xf4, "major_exotic_unit");
SENATE_BENEFIT(TwoMinorExoticUnitsReward, REWARD_TWO_MINOR_EXOTIC_UNITS, 0xf5, "two_minor_exotic_units");
SENATE_BENEFIT(TwoBestBuildableUnitsReward, REWARD_TWO_BEST_BUILDABLE_UNITS, 0xf6, "two_best_buildable_units");
SENATE_BENEFIT(OneBestBuildableUnitReward, REWARD_ONE_BEST_BUILDABLE_UNIT, 0xf7, "one_best_buildable_unit");
SENATE_BENEFIT(OneMinorExoticUnitReward, REWARD_ONE_MINOR_EXOTIC_UNIT, 0xf8, "one_minor_exotic_unit");
SENATE_BENEFIT(OneBuildableUnitReward, REWARD_ONE_BUILDABLE_UNIT, 0xf9, "one_buildable_unit");
// Cash, by major_reward, moderate_reward and minor_reward.
SENATE_BENEFIT(MajorCashReward, REWARD_MAJOR_CASH, 0xff, "major_reward");
SENATE_BENEFIT(ModerateCashReward, REWARD_MODERATE_CASH, 0x100, "moderate_reward");
SENATE_BENEFIT(MinorCashReward, REWARD_MINOR_CASH, 0x101, "minor_reward");
// Only the family is considered at the next appointments.
SENATE_BENEFIT(AppointmentReward, REWARD_PROMISE_OF_APPOINTMENT, 0x103, "promise_of_appointment");
// The senate's appreciation starts (its date is only kept).
SENATE_BENEFIT(GratitudeReward, REWARD_SENATE_GRATITUDE, 0xfc, "senate_gratitude");
// Games, for free, in the faction's settlement with the lowest public order.
SENATE_BENEFIT(GamesReward, REWARD_THROW_GAMES, 0xfe, "throw_games");
// Nothing (its name text is throw_games's).
class NoReward : public Benefit<REWARD_NONE, 0xfe> {
public:
    NoReward() : Benefit<REWARD_NONE, 0xfe>("null_payback") {}
};
SENATE_BENEFIT(OneTriariiReward, REWARD_ONE_TRIARII, 0x4ed, "one_triarii");
SENATE_BENEFIT(TwoTriariiReward, REWARD_TWO_TRIARII, 0x4ee, "two_triarii");

static OutlawPenalty s_outlaw;                                  // 0x012D7654
static CensorFamilyPenalty s_censor_investigates_family;        // 0x012D75B0
static CensorFactionPenalty s_censor_investigates_faction;      // 0x012D7678
static QuaestorCorruptionPenalty s_quaestor_corruption;         // 0x012D75C4
static QuaestorFinancialPenalty s_quaestor_financial;           // 0x012D7664
static QuaestorAccountingPenalty s_quaestor_accounting;         // 0x012D76AC
static DispleasurePenalty s_senate_displeasure;                 // 0x012D75B8
static ExposurePenalty s_secret_information_exposure;           // 0x012D7660
static OfficesWithheldPenalty s_offices_withheld;               // 0x012D7694
static MajorExoticUnitReward s_major_exotic_unit;               // 0x012D75A8
static TwoMinorExoticUnitsReward s_two_minor_exotic_units;      // 0x012D76A0
static TwoBestBuildableUnitsReward s_two_best_buildable_units;  // 0x012D75C0
static OneBestBuildableUnitReward s_one_best_buildable_unit;    // 0x012D76B0
static OneMinorExoticUnitReward s_one_minor_exotic_unit;        // 0x012D7658
static OneBuildableUnitReward s_one_buildable_unit;             // 0x012D7684
static MajorCashReward s_major_cash;                            // 0x012D7674
static ModerateCashReward s_moderate_cash;                      // 0x012D768C
static MinorCashReward s_minor_cash;                            // 0x012D7698
static AppointmentReward s_promise_of_appointment;              // 0x012D765C
static GratitudeReward s_senate_gratitude;                      // 0x012D7670
static GamesReward s_throw_games;                               // 0x012D7680
NoReward s_null_payback;                                        // 0x012D75B4
static OneTriariiReward s_one_triarii;                          // 0x012D75C8
static TwoTriariiReward s_two_triarii;                          // 0x012D75D0

SenateReward* const s_rewards[NUM_REWARDS] = {
    &s_outlaw,
    &s_censor_investigates_family,
    &s_censor_investigates_faction,
    &s_quaestor_corruption,
    &s_quaestor_financial,
    &s_quaestor_accounting,
    &s_senate_displeasure,
    &s_secret_information_exposure,
    &s_offices_withheld,
    &s_major_exotic_unit,
    &s_two_minor_exotic_units,
    &s_two_best_buildable_units,
    &s_one_best_buildable_unit,
    &s_one_minor_exotic_unit,
    &s_one_buildable_unit,
    &s_major_cash,
    &s_moderate_cash,
    &s_minor_cash,
    &s_promise_of_appointment,
    &s_senate_gratitude,
    &s_throw_games,
    &s_null_payback,
    &s_one_triarii,
    &s_two_triarii,
};

SenateReward::SenateReward(const char* name)
{
    rewards()->insert(TextKey(name), this);
}

UniString* SenateReward::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    return 0;
}

UniString* SenateReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    return 0;
}

// The last reward or penalty chosen: a code that picks at random never gives it twice running.
static int s_last_reward = REWARD_NONE;  // 0x010E47A4

// Codes 2 to 4 (p3, p2 and p1, the harshest first) and 5 to 7 (r1, r2 and r3, the best last) pick at random among
// theirs. A faction that's losing money always gets cash for a reward; for r1, one with a settlement at 70% public
// order or less gets games. two_triarii is never chosen.
SenateReward* choose_reward(int code, Faction* faction)
{
    // NONMATCHING - its locals' stack slots are in another order
    int chosen = s_last_reward;
    bool losing_money = faction->m_finance.net() < 0;
    bool unruly = faction->unk_004CF466();
    switch (code) {
    case 1:
        chosen = REWARD_OUTLAW;
        break;
    case 2:
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(1)) {
            case 0:
                chosen = REWARD_CENSOR_INVESTIGATES_FAMILY;
                break;
            case 1:
                chosen = REWARD_QUAESTOR_CORRUPTION;
                break;
            }
        }
        break;
    case 3:
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(1)) {
            case 0:
                chosen = REWARD_CENSOR_INVESTIGATES_FACTION;
                break;
            case 1:
                chosen = REWARD_QUAESTOR_FINANCIAL;
                break;
            }
        }
        break;
    case 4:
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(3)) {
            case 0:
                chosen = REWARD_QUAESTOR_ACCOUNTING;
                break;
            case 1:
                chosen = REWARD_SENATE_DISPLEASURE;
                break;
            case 2:
                chosen = REWARD_SECRET_INFORMATION_EXPOSURE;
                break;
            case 3:
                chosen = REWARD_OFFICES_WITHHELD;
                break;
            }
        }
        break;
    case 5:
        if (losing_money) {
            chosen = REWARD_MINOR_CASH;
            break;
        }
        if (unruly) {
            chosen = REWARD_THROW_GAMES;
            break;
        }
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(4)) {
            case 0:
                chosen = REWARD_ONE_BUILDABLE_UNIT;
                break;
            case 1:
                chosen = REWARD_MINOR_CASH;
                break;
            case 2:
                chosen = REWARD_PROMISE_OF_APPOINTMENT;
                break;
            case 3:
                chosen = REWARD_SENATE_GRATITUDE;
                break;
            case 4:
                chosen = REWARD_THROW_GAMES;
                break;
            }
        }
        break;
    case 6:
        if (losing_money) {
            chosen = REWARD_MODERATE_CASH;
            break;
        }
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(4)) {
            case 0:
                chosen = REWARD_ONE_BEST_BUILDABLE_UNIT;
                break;
            case 1:
                chosen = REWARD_ONE_MINOR_EXOTIC_UNIT;
                break;
            case 2:
                chosen = REWARD_MODERATE_CASH;
                break;
            default:
                chosen = REWARD_ONE_TRIARII;
                break;
            }
        }
        break;
    case 7:
        if (losing_money) {
            chosen = REWARD_MAJOR_CASH;
            break;
        }
        while (s_last_reward == chosen) {
            switch (lbl_018CD818->m_random.up_to(3)) {
            case 0:
                chosen = REWARD_MAJOR_EXOTIC_UNIT;
                break;
            case 1:
                chosen = REWARD_TWO_MINOR_EXOTIC_UNITS;
                break;
            case 2:
                chosen = REWARD_TWO_BEST_BUILDABLE_UNITS;
                break;
            case 3:
                chosen = REWARD_MAJOR_CASH;
                break;
            }
        }
        break;
    case 0:
        chosen = REWARD_NONE;
        break;
    }
    s_last_reward = chosen;
    return s_rewards[chosen];
}

// The Duration line, then a code for each "attitude difficulty" line ("-3 E   Outlaw", ...) to the End; the codes
// not given are 0 (none). Any other line is skipped.
bool SenateRewardTable::load(ScriptReader& reader)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (!reader.is_open())
        return false;
    while (!reader.at_end() && !reader.match("Duration"))
        reader.next_line();
    if (reader.at_end())
        return false;
    if (reader.match("Duration")) {
        reader.skip_word();
        m_duration = reader.read_int();
        reader.next_line();
    }
    for (unsigned int level = 0; level < 7; level++) {
        for (unsigned int difficulty = 0; difficulty < 3; difficulty++)
            m_codes[level][difficulty] = 0;
    }
    while (!reader.at_end() && !reader.match("End")) {
        int level = reader.read_int() + 3;
        reader.skip_word();
        int difficulty;
        switch (*reader.cur()) {
        case 'E':
            difficulty = 0;
            break;
        case 'M':
            difficulty = 1;
            break;
        case 'D':
            difficulty = 2;
            break;
        default:
            return false;
        }
        reader.skip_word();
        if (reader.match("Outlaw"))
            m_codes[level][difficulty] = 1;
        else if (reader.match("p3"))
            m_codes[level][difficulty] = 2;
        else if (reader.match("p2"))
            m_codes[level][difficulty] = 3;
        else if (reader.match("p1"))
            m_codes[level][difficulty] = 4;
        else if (reader.match("r1"))
            m_codes[level][difficulty] = 5;
        else if (reader.match("r2"))
            m_codes[level][difficulty] = 6;
        else if (reader.match("r3"))
            m_codes[level][difficulty] = 7;
        reader.next_line();
    }
    return true;
}

SenateMissionBase::SenateMissionBase(const SenateMissionState& state)
    // NONMATCHING - the original keeps the senate in a second temporary
    : m_senate((unsigned int)saved_version("SENATE_MISSION_IMPL") >= 1
                   ? (AISenateFaction*)fn_00413901((void*)state.m_senate)
                   : lbl_018CD818->m_senate),
      m_faction(lbl_018CD818->faction(state.m_faction)), m_reward(s_rewards[state.m_reward]),
      m_result(state.m_result), m_date(state.m_date), m_duration(state.m_duration)
{
}

SenateMissionBase::SenateMissionBase(AISenateFaction* senate, Faction* faction, SenateReward* reward, int duration)
    : m_senate(senate), m_faction(faction), m_reward(reward), m_result(0), m_date(lbl_018CD818->m_date),
      m_duration(duration)
{
}

SenateMissionBase::~SenateMissionBase()
{
}

void SenateMissionBase::unk_24()
{
    m_faction->add_mission(this);
}

void SenateMissionBase::unk_00()
{
    // NONMATCHING - the original loads this before the vtable for its virtual calls, as for temporaries
    void* mission_title = title();
    void* target = target_name();
    void* mission_text = text();
    int duration = m_duration;
    void* payback_kind = fn_00D93A0C(m_reward->is_benefit() ? 0xf2 : 0xdf);
    void* payback = m_reward->title();
    SenateMissionInfo info(mission_title, 0x3b, target, mission_text, duration, payback_kind, payback, 0);
    fn_00CB639F(m_faction, &info, unk_0c());
}

void SenateMissionBase::unk_28()
{
}

int SenateMissionBase::turns_left()
{
    return m_duration - lbl_018CD818->m_date.turns_since(m_date);
}

bool SenateMissionBase::reward_title(void** title)
{
    *title = m_reward->title();
    return m_reward->is_benefit();
}

bool SenateMissionBase::unk_18()
{
    return true;
}

Faction* SenateMissionBase::faction()
{
    return m_faction;
}

int SenateMissionBase::result()
{
    return m_result;
}

// The date, the senate, the faction's index, the result, the reward's index and the duration.
bool SenateMissionBase::save(SaveStream& stream)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (!stream.unk_00F3D990())
        return false;
    int faction = m_faction->id();
    AISenateFaction* senate = m_senate;
    stream.write(&m_date, 8);
    stream.write(&senate, 4);
    stream.write(&faction, 4);
    stream.write(&m_result, 4);
    SaveStream& rest = payback()->save(stream);
    rest.write(&m_duration, 4);
    return rest.unk_00F3D990();
}

static VersionRegistrar s_mission_version("SENATE_MISSION_IMPL", 2);

// Version 1 has no reward or duration, and the first has no senate.
SaveStream& load_mission_state(SaveStream& stream, SenateMissionState& state)
{
    stream.read(&state.m_date, 8);
    int unused;
    switch (saved_version("SENATE_MISSION_IMPL")) {
    case 1:
        stream.read(&state.m_senate, 4);
        stream.read(&state.m_faction, 4);
        stream.read(&state.m_result, 4);
        break;
    case 2: {
        stream.read(&state.m_senate, 4);
        stream.read(&state.m_faction, 4);
        stream.read(&state.m_result, 4);
        stream.read(&state.m_reward, 4);
        int duration;
        stream.read(duration);
        state.m_duration = duration;
        break;
    }
    default:
        stream.read(&state.m_faction, 4);
        stream.read(&unused, 4);
        stream.read(&state.m_result, 4);
        break;
    }
    return stream;
}

void SenateMissionOffer::report_accepted(void* name, void* text)
{
    // NONMATCHING - the original loads this before the vtable for its virtual calls, as for temporaries, and its locals' stack slots are in another order
    UniString payback_text;
    TilePosition position;
    RewardPlace place = &position;
    UniString* payback = m_reward->reward(m_senate, m_faction, payback_text, place);
    void* mission_title = title();
    SenateMissionVerdict verdict(mission_title, 0x3d, name, text, payback);
    fn_00CB6486(m_faction, &verdict, place);
    m_result = MISSION_SUCCEEDED;
}

void SenateMissionOffer::report_cancelled(void* name, void* text)
{
    // NONMATCHING - the original loads this before the vtable for its virtual calls, as for temporaries
    void* mission_title = title();
    SenateMissionNote note(mission_title, 0x3a, name, text);
    fn_00CB634F(m_faction, &note, 0);
    m_result = MISSION_CANCELLED;
}

void SenateMissionOffer::report_declined(void* name, void* text)
{
    // NONMATCHING - the original loads this before the vtable for its virtual calls, as for temporaries, and its locals' stack slots are in another order
    UniString payback_text;
    void* mission_title = title();
    UniString* payback = m_reward->punish(m_senate, m_faction, payback_text);
    SenateMissionVerdict verdict(mission_title, 0x3c, name, text, payback);
    fn_00CB656B(m_faction, &verdict, 0);
    m_result = MISSION_FAILED;
}

SenateMissionState::SenateMissionState()
    : m_senate(0), m_faction(21), m_reward(REWARD_NONE), m_result(0), m_duration(10)
{
}

SenateMissionOffer::SenateMissionOffer(AISenateFaction* senate, Faction* faction, SenateReward* reward, int duration)
    : SenateMissionBase(senate, faction, reward, duration)
{
}

SenateMissionOffer::SenateMissionOffer(const SenateMissionState& state) : SenateMissionBase(state)
{
}

void SenateMissionOffer::unk_24()
{
    faction()->add_offer(this);
}

void SenateMissionOffer::unk_00()
{
    // NONMATCHING - the original gets payback() twice before the rest, and evaluates the fields in another order
    SenateMissionInfo info(title(), 0x3b, target_name(), text(), duration(),
                           fn_00D93A0C(payback()->is_benefit() ? 0xf2 : 0xdf), payback()->title(),
                           (SenateMissionAnswer*)this);
    fn_00CB639F(faction(), &info, unk_0c());
}

void SenateMissionOffer::accepted()
{
    faction()->accept_offer(this);
}

void SenateMissionOffer::declined()
{
    faction()->drop_offer(this);
}

// No mission (type 10's place in the loaders).
static SenateMissionImpl* load_no_mission(SaveStream& stream)
{
    return 0;
}

static RewardMap* rewards()
{
    if (!s_reward_map)
        s_reward_map = new RewardMap(TextKey(&s_unused_key), TextKey(&s_deleted_key), 0x18);
    return s_reward_map;
}

SenateMissionImpl::~SenateMissionImpl()
{
}

int SenateMissionImpl::unk_1c()
{
    return 0x24;
}

int SenateMissionImpl::unk_20()
{
    return 0;
}

SenateMissionImpl* SenateMissionImpl::build()
{
    return this;
}

// Skips lines to the one that starts with key, and fails at the end of the file.
#define FIND_SECTION(reader, key)                                                                                       while (!(reader).at_end() && !(reader).match(key))                                                                      (reader).next_line();                                                                                           if ((reader).at_end())                                                                                                  return false

// descr_senate.txt's missions: the fines and cash rewards, then each mission type's table (or its own lines), in
// this order.
bool load_senate_missions(ScriptReader& reader)
{
    if (!reader.is_open())
        return false;
    FIND_SECTION(reader, "major_fine");
    reader.skip_word();
    s_major_fine = reader.read_uint();
    FIND_SECTION(reader, "moderate_fine");
    reader.skip_word();
    s_moderate_fine = reader.read_uint();
    FIND_SECTION(reader, "minor_fine");
    reader.skip_word();
    s_minor_fine = reader.read_uint();
    FIND_SECTION(reader, "major_reward");
    reader.skip_word();
    s_major_reward = reader.read_uint();
    FIND_SECTION(reader, "moderate_reward");
    reader.skip_word();
    s_moderate_reward = reader.read_uint();
    FIND_SECTION(reader, "minor_reward");
    reader.skip_word();
    s_minor_reward = reader.read_uint();
    FIND_SECTION(reader, "Help_Player");
    reader.next_line();
    if (!lbl_012D8BA0.load(reader))
        return false;
    FIND_SECTION(reader, "Demand_Suicide");
    reader.next_line();
    if (!DemandSuicideBuilder::s_rewards.load(reader))
        return false;
    FIND_SECTION(reader, "Attack_Outlaw_Faction");
    reader.next_line();
    if (!lbl_012D7B40.load(reader))
        return false;
    FIND_SECTION(reader, "Assassinate");
    reader.next_line();
    if (!fn_0054C825(reader))
        return false;
    FIND_SECTION(reader, "Break_Alliance");
    reader.next_line();
    if (!fn_005507B0(reader))
        return false;
    FIND_SECTION(reader, "Cease_Hostilities");
    reader.next_line();
    if (!fn_00553BA8(reader))
        return false;
    FIND_SECTION(reader, "Declare_War");
    reader.next_line();
    if (!fn_00554915(reader))
        return false;
    FIND_SECTION(reader, "Give_Back_City");
    reader.next_line();
    if (!lbl_012D8988.load(reader))
        return false;
    FIND_SECTION(reader, "Exact_Tribute");
    reader.next_line();
    if (!lbl_012D8640.load(reader))
        return false;
    FIND_SECTION(reader, "Subjugate");
    reader.next_line();
    if (!lbl_012D9300.load(reader))
        return false;
    FIND_SECTION(reader, "Broker_Peace");
    reader.next_line();
    if (!lbl_012D7EE0.load(reader))
        return false;
    FIND_SECTION(reader, "Make_Trade_Agreement");
    reader.next_line();
    if (!lbl_012D8F68.load(reader))
        return false;
    FIND_SECTION(reader, "Get_Map_Info");
    reader.next_line();
    if (!lbl_012D87F0.load(reader))
        return false;
    FIND_SECTION(reader, "Appease");
    reader.next_line();
    if (!lbl_012D7800.load(reader))
        return false;
    FIND_SECTION(reader, "Make_Alliance");
    reader.next_line();
    if (!lbl_012D8DB8.load(reader))
        return false;
    FIND_SECTION(reader, "Give_Cash");
    reader.next_line();
    if (!lbl_012D8A08.load(reader))
        return false;
    FIND_SECTION(reader, "Annex_City");
    reader.next_line();
    if (!lbl_012D7788.load(reader))
        return false;
    FIND_SECTION(reader, "Blockade_Port");
    reader.next_line();
    if (!fn_0054F877(reader))
        return false;
    FIND_SECTION(reader, "Capture_Rebel_City");
    reader.next_line();
    if (!fn_005531DD(reader))
        return false;
    FIND_SECTION(reader, "Destroy_Rebel_City");
    reader.next_line();
    if (!lbl_012D8530.load(reader))
        return false;
    FIND_SECTION(reader, "rebel_city_lower_boundary");
    reader.skip_word();
    s_rebel_city_lower_boundary = reader.read_float();
    FIND_SECTION(reader, "rebel_city_higher_boundary");
    reader.skip_word();
    s_rebel_city_higher_boundary = reader.read_float();
    FIND_SECTION(reader, "Take_City");
    reader.next_line();
    if (!fn_005620F2(reader))
        return false;
    FIND_SECTION(reader, "take_city_lower_boundary");
    reader.skip_word();
    s_take_city_lower_boundary = reader.read_float();
    FIND_SECTION(reader, "take_city_higher_boundary");
    reader.skip_word();
    s_take_city_higher_boundary = reader.read_float();
    return true;
}

void unload_senate_missions()
{
    fn_00553C9B();
    fn_005508A3();
    fn_00554A08();
    delete s_reward_map;
}

extern "C" void fn_00547ED3()
{
    fn_00553C83();
    fn_0055088B();
    fn_005549F0();
    fn_0056218B();
}

static MissionLoader s_mission_loaders[] = {
    fn_005506AC, fn_0055310B, fn_00555F9F, fn_0055996C, fn_0054EB5D, fn_00553AA4, fn_00554809,
    fn_0055B7DA, fn_0054C512, fn_00561842, load_no_mission, DemandSuicideBuilder::load,
    fn_0055F10D, fn_0055FF48, fn_005568C1, fn_005580F9, fn_00552559, fn_0054DB35, fn_0055C5BC,
    fn_0054F724, fn_0055720C, fn_00560916, fn_0054A471, fn_00558A59, fn_0055DD5C, fn_0054B307,
    fn_00551618,
};

// A mission (or a builder) from a save: its type, then what that type's loader reads.
SenateMissionImpl* load_senate_mission(SaveStream& stream)
{
    int type;
    stream.read(&type, 4);
    return s_mission_loaders[type](stream);
}

SenateMissionRequest::SenateMissionRequest(AISenateFaction* senate, Faction* faction, int level, SenateReward* reward,
                                           int unk)
{
    m_senate = senate;
    m_faction = faction;
    m_level = level;
    m_reward = reward;
    m_10 = unk;
}

UniString* OutlawPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    // NONMATCHING - its locals' stack slots are in another order
    int id = faction->m_id;
    int senate_faction;
    if (lbl_018CD818->m_2c8.size() != 0)
        senate_faction = lbl_018CD818->m_2c8[0].m_faction;
    else
        senate_faction = 21;
    lbl_018CD818->m_diplomacy.declare_war(senate_faction, id);
    return &(text = senate_text(0xe2));
}

UniString* CensorFamilyPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (senate->censor_protects(faction)) {
        text = senate_text(0x412);
        return &text;
    }
    LinkedList<Character*> family;
    unsigned int count = faction->unk_004D6EC0(&family, CHARACTER_NAMED);
    if (count > 1) {
        Character* victim = 0;
        do
            victim = family[lbl_018CD818->m_random.up_to(family.size() - 1)];
        while (victim->record()->m_leader);
        victim->kill();
        victim->unk_00580C1E()->m_898.set_14(fn_008F649F());
        text = uni_format(UniString("%S (%S)"), *(UniString*)fn_00D93A0C(0xe4), victim->m_record->m_name);
        return &text;
    }
    return 0;
}

UniString* CensorFactionPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    if (senate->censor_protects(faction)) {
        text = senate_text(0x412);
        return &text;
    }
    LinkedList<Character*> agents;
    unsigned int count = faction->unk_004D6EC0(&agents, CHARACTER_SPY) +
                         faction->unk_004D6EC0(&agents, CHARACTER_ASSASSIN) +
                         faction->unk_004D6EC0(&agents, CHARACTER_DIPLOMAT) +
                         faction->unk_004D6EC0(&agents, CHARACTER_GENERAL);
    if (count > 0) {
        Character* victim = agents[lbl_018CD818->m_random.up_to(agents.size() - 1)];
        victim->kill();
        victim->unk_00580C1E()->m_898.set_14(fn_008F649F());
        text = uni_format(UniString("%S %S"), *(UniString*)fn_00D93A0C(0xe8), victim->m_record->m_name);
        return &text;
    }
    return 0;
}

// The fine: the turn's net income (its first four incomes less its first two costs, whether it's a gain or a loss)
// divided by descr_senate.txt's figure, so a higher figure is a smaller fine.
UniString* QuaestorCorruptionPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (senate->quaestor_protects(faction)) {
        text = senate_text(0x413);
        return &text;
    }
    FinanceRecord& record = faction->m_finance.m_history.current();
    int net = record.unk_004D7E1C() - record.unk_004D7E74();
    unsigned int amount = (unsigned int)(net < 0 ? -net : net) / s_major_fine;
    faction->m_finance.spend(amount, FINANCE_SENATE);
    fire_triggers(QuaestorInvestigationMajorEvent(faction->leader()));
    return &(text = fn_00D926DB()->major_fine(amount));
}

UniString* QuaestorFinancialPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (senate->quaestor_protects(faction)) {
        text = senate_text(0x413);
        return &text;
    }
    FinanceRecord& record = faction->m_finance.m_history.current();
    int net = record.unk_004D7E1C() - record.unk_004D7E74();
    unsigned int amount = (unsigned int)(net < 0 ? -net : net) / s_moderate_fine;
    faction->m_finance.spend(amount, FINANCE_SENATE);
    fire_triggers(QuaestorInvestigationEvent(faction->leader()));
    return &(text = fn_00D926DB()->fine(amount));
}

UniString* QuaestorAccountingPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    // NONMATCHING - its locals' stack slots are in another order
    if (senate->quaestor_protects(faction)) {
        text = senate_text(0x413);
        return &text;
    }
    FinanceRecord& record = faction->m_finance.m_history.current();
    int net = record.unk_004D7E1C() - record.unk_004D7E74();
    unsigned int amount = (unsigned int)(net < 0 ? -net : net) / s_minor_fine;
    faction->m_finance.spend(amount, FINANCE_SENATE);
    fire_triggers(QuaestorInvestigationMinorEvent(faction->leader()));
    return &(text = fn_00D926DB()->minor_fine(amount));
}

UniString* DispleasurePenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    senate->start_displeasure();
    return 0;
}

UniString* ExposurePenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    fire_triggers(SenateExposureEvent(faction->leader()));
    return &(text = senate_text(0xef));
}

UniString* OfficesWithheldPenalty::punish(AISenateFaction* senate, Faction* faction, UniString& text)
{
    senate->withhold_offices();
    return &(text = senate_text(0xf1));
}

// The unit rewards tell of the first unit, and where it is.
UniString* MajorExoticUnitReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                         RewardPlace& place)
{
    RewardUnit unit;
    faction->give_major_exotic_units(1, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

UniString* TwoMinorExoticUnitsReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                             RewardPlace& place)
{
    RewardUnit unit;
    faction->give_minor_exotic_units(2, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0x46f) + *unit.m_unit->unk_50()->m_name);
}

UniString* TwoBestBuildableUnitsReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                               RewardPlace& place)
{
    RewardUnit unit;
    faction->give_best_buildable_units(2, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0x46f) + *unit.m_unit->unk_50()->m_name);
}

UniString* OneBestBuildableUnitReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                              RewardPlace& place)
{
    RewardUnit unit;
    faction->give_best_buildable_units(1, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

UniString* OneMinorExoticUnitReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                            RewardPlace& place)
{
    RewardUnit unit;
    faction->give_minor_exotic_units(1, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

UniString* OneBuildableUnitReward::reward(AISenateFaction* senate, Faction* faction, UniString& text,
                                          RewardPlace& place)
{
    RewardUnit unit;
    faction->give_buildable_units(1, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

UniString* MajorCashReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    faction->m_finance.earn(s_major_reward, FINANCE_SENATE);
    place = 0;
    return &(text = fn_00D926DB()->cash(s_major_reward));
}

UniString* ModerateCashReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    faction->m_finance.earn(s_moderate_reward, FINANCE_SENATE);
    place = 0;
    return &(text = fn_00D926DB()->cash(s_moderate_reward));
}

UniString* MinorCashReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    faction->m_finance.earn(s_minor_reward, FINANCE_SENATE);
    place = 0;
    return &(text = fn_00D926DB()->cash(s_minor_reward));
}

UniString* AppointmentReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    senate->promise_appointment();
    place = 0;
    return &(text = senate_text(0x104));
}

UniString* GratitudeReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    senate->start_appreciation();
    place = 0;
    return &(text = senate_text(0xfd));
}

UniString* GamesReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    place = 0;
    faction->unk_004CF4E5()->unk_005BC88A(1);
    return &(text = senate_text(0xfe));
}

UniString* OneTriariiReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    RewardUnit unit;
    faction->give_triarii(1, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

UniString* TwoTriariiReward::reward(AISenateFaction* senate, Faction* faction, UniString& text, RewardPlace& place)
{
    RewardUnit unit;
    faction->give_triarii(2, &unit);
    if (!unit.m_unit)
        return &(text = senate_text(0xfa));
    *place = *unit.m_place->position();
    return &(text = *(UniString*)fn_00D93A0C(0xfb) + *unit.m_unit->unk_50()->m_name);
}

void* SenateMissionOffer::accept_text()
{
    return fn_00D93A0C(0x223);
}

void* SenateMissionOffer::decline_text()
{
    return fn_00D93A0C(0x405);
}

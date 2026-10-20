#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\sm_demand_suicide.cpp"
// The senate's Demand_Suicide mission: it asks the faction leader to kill himself.
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"
#include "character.h"
#include "character_record.h"
#include "senate_mission.h"

extern "C" {
void* fn_00D93A0C(int text);                // a senate text by its number (a guess)
int fn_008F649F();                          // the index of the faction "romans_senate"
}

// The builder of the mission, as a save keeps it.
class DemandSuicideBuilder : public SenateMissionBuilder {
public:
    DemandSuicideBuilder(const SenateMissionState& state);

    static SenateMissionImpl* load(SaveStream& stream);
    static SenateMissionImpl* create(const SenateMissionRequest& request);

    virtual void* title();
    virtual SenateMissionImpl* build();

    static SenateRewardTable s_rewards;     // descr_senate.txt's Demand_Suicide table

    SenateMissionState m_state;             // +0x04
};

// The mission: the human's faction leader is to fall on his sword now (its Duration is 0).
class DemandSuicideMission : public SenateMissionOffer {
public:
    DemandSuicideMission(AISenateFaction* senate, Faction* faction, SenateReward* reward);
    DemandSuicideMission(const DemandSuicideBuilder& builder);

    virtual int run_out() { return 0; }
    virtual void* title() { return fn_00D93A0C(0xdb); }
    virtual void unk_08(int* a, int* b)
    {
        *a = 0;
        *b = 0;
    }
    virtual int unk_0c() { return 0; }
    virtual void* text() { return fn_00D93A0C(0xdc); }
    virtual void* target_name() { return faction()->display_name(); }

    virtual bool save(SaveStream& stream);
    virtual bool possible(AISenateFaction* senate);
    virtual void accepted();
    virtual void declined();
};

// 0x005555E6
SenateMissionImpl* DemandSuicideBuilder::load(SaveStream& stream)
{
    SenateMissionState state;
    load_mission_state(stream, state);
#line 46
    return NEW DemandSuicideBuilder(state);
}

DemandSuicideBuilder::DemandSuicideBuilder(const SenateMissionState& state) : m_state(state)
{
}

void* DemandSuicideBuilder::title()
{
    return fn_00D93A0C(0xdb);
}

// NONMATCHING - its initializer: the original reserves 0x28 bytes of stack it doesn't use
SenateRewardTable DemandSuicideBuilder::s_rewards;

// The mission, unless its penalty (by default the table's, at the faction's level) is none.
SenateMissionImpl* DemandSuicideBuilder::create(const SenateMissionRequest& request)
{
    // NONMATCHING - the original computes the table's column offset (0) first, without folding it
    SenateReward* reward = request.m_reward;
    int difficulty = 0;
    if (!reward)
        reward = choose_reward(s_rewards.m_codes[request.m_level][difficulty], request.m_faction);
    if (is_reward(reward))
#line 81
        return NEW DemandSuicideMission(request.m_senate, request.m_faction, reward);
    return 0;
}

DemandSuicideMission::DemandSuicideMission(AISenateFaction* senate, Faction* faction, SenateReward* reward)
    : SenateMissionOffer(senate, faction, reward, 0)
{
}

SenateMissionImpl* DemandSuicideBuilder::build()
{
#line 98
    SenateMissionImpl* mission = NEW DemandSuicideMission(*this);
    CA_DELETE(this);
    return mission;
}

DemandSuicideMission::DemandSuicideMission(const DemandSuicideBuilder& builder) : SenateMissionOffer(builder.m_state)
{
}

bool DemandSuicideMission::save(SaveStream& stream)
{
    if (!stream.unk_00F3D990())
        return false;
    int type = 11;
    stream.write(&type, 4);
    if (!SenateMissionOffer::save(stream))
        return false;
    return stream.unk_00F3D990();
}

bool DemandSuicideMission::possible(AISenateFaction* senate)
{
    return true;
}

// The leader dies, killed by the senate.
void DemandSuicideMission::accepted()
{
    this->faction()->leader()->m_character->kill();
    this->faction()->m_898.set_14(fn_008F649F());
    report_accepted(this->faction()->display_name(), fn_00D93A0C(0xde));
    SenateMissionOffer::declined();
}

void DemandSuicideMission::declined()
{
    report_declined(this->faction()->display_name(), fn_00D93A0C(0xdd));
    SenateMissionOffer::declined();
}

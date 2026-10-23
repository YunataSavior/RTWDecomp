// What each faction knows of a map object: its level of information on it, and what that lets it see.
#include "unsorted.h"
#include "spying_info.h"
#include "faction.h"
#include "world.h"

SpyingInfo::SpyingInfo()
{
    for (unsigned int i = 0; i < 21; i++)
        m_level[i] = -1;
    m_random.Random::Random(0);
}

// The seed is drawn from the campaign's generator the first time it's asked for.
int CharacterSpyingInfo::seed()
{
    if (m_seed == -1) {
        m_seed = lbl_018CD818->m_random.next();
        int high = m_seed << 16;
        m_seed = high | lbl_018CD818->m_random.next();
    }
    return m_seed;
}

// FACTION's level rises to LEVEL (at most 3), and never falls.
int CharacterSpyingInfo::raise(int faction, int level)
{
    if (level > 3)
        level = 3;
    if (level > m_level[faction])
        m_level[faction] = level;
    else
        level = m_level[faction];
    return level;
}

void CharacterSpyingInfo::reset(int faction)
{
    seed();
    m_level[faction] = -1;
}

// Its own faction sees everything; another sees what its level allows, where a spy or an assassin that is hidden
// (unk_14 or unk_18) is an agent rather than a character.
bool CharacterSpyingInfo::can_see(int faction, unsigned int item)
{
    if (this->faction()->id() == faction)
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    int agent;
    if ((type() == 0 || type() == 1) && (unk_14() || unk_18()))
        agent = SpyingInfoDB::AGENT;
    else
        agent = SpyingInfoDB::CHARACTER;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(agent)[level].test(item))
        return true;
    else
        return false;
}

int SettlementSpyingInfo::seed()
{
    if (m_seed == -1) {
        m_seed = lbl_018CD818->m_random.next();
        int high = m_seed << 16;
        m_seed = high | lbl_018CD818->m_random.next();
    }
    return m_seed;
}

void SettlementSpyingInfo::raise(int faction, int level)
{
    if (level > 3)
        level = 3;
    if (level > m_level[faction])
        m_level[faction] = level;
    else
        level = m_level[faction];
}

void SettlementSpyingInfo::reset(int faction)
{
    seed();
    m_level[faction] = -1;
}

bool SettlementSpyingInfo::can_see(int faction, unsigned int item)
{
    if (this->faction()->id() == faction)
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(SpyingInfoDB::SETTLEMENT)[level].test(item))
        return true;
    else
        return false;
}

void BuildingSpyingInfo::raise(int faction, int level)
{
    if (level > 3)
        level = 3;
    if (level > m_level[faction])
        m_level[faction] = level;
    else
        level = m_level[faction];
}

bool BuildingSpyingInfo::can_see(int faction, unsigned int item)
{
    if (this->faction()->id() == faction)
        return true;
    if (unk_08() || unk_0c())
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(SpyingInfoDB::BUILDING)[level].test(item))
        return true;
    else
        return false;
}

int FortSpyingInfo::seed()
{
    if (m_seed == -1) {
        m_seed = lbl_018CD818->m_random.next();
        int high = m_seed << 16;
        m_seed = high | lbl_018CD818->m_random.next();
    }
    return m_seed;
}

void FortSpyingInfo::raise(int faction, int level)
{
    if (level > 3)
        level = 3;
    if (level > m_level[faction])
        m_level[faction] = level;
    else
        level = m_level[faction];
}

void FortSpyingInfo::reset(int faction)
{
    seed();
    m_level[faction] = -1;
}

bool FortSpyingInfo::can_see(int faction, unsigned int item)
{
    if (this->faction()->id() == faction)
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(SpyingInfoDB::FORT)[level].test(item))
        return true;
    else
        return false;
}

int ArmySpyingInfo::seed()
{
    if (m_seed == -1) {
        m_seed = lbl_018CD818->m_random.next();
        int high = m_seed << 16;
        m_seed = high | lbl_018CD818->m_random.next();
    }
    return m_seed;
}

void ArmySpyingInfo::reseed()
{
    seed();
    m_random.Random::Random(m_seed);
}

// A random value from VALUE * spread to VALUE / spread, where spread is 0.35 * ACCURACY + 0.3, kept between 0.1 and 1:
// so an ACCURACY of 2 or more gives VALUE itself.
int ArmySpyingInfo::estimate(float accuracy, int value)
{
    float spread = accuracy * 0.35f + 0.3f;
    float top = 1.0f;
    float bottom = 0.1f;
    spread = min_of(max_of(spread, bottom), top);
    int lower = chop_to_int(value * spread);
    int upper = chop_to_int(value / spread);
    return m_random.range(lower, upper);
}

void ArmySpyingInfo::reset(int faction)
{
    seed();
    m_level[faction] = -1;
}

bool ArmySpyingInfo::can_see(int faction, unsigned int item)
{
    if (this->faction()->id() == faction)
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(SpyingInfoDB::ARMY)[level].test(item))
        return true;
    else
        return false;
}

int UnitSpyingInfo::raise(int faction, int level)
{
    if (level > 3)
        level = 3;
    if (level == 0 && unk_14())
        level = 1;
    if (level > m_level[faction])
        m_level[faction] = level;
    else
        level = m_level[faction];
    return level;
}

bool UnitSpyingInfo::can_see(int faction, unsigned int item)
{
    if (faction_id() == faction)
        return true;
    SpyingInfoDB* info = &lbl_01589168;
    char level = m_level[faction];
    if (level < 0)
        level = 0;
    if (info->kind(SpyingInfoDB::UNIT)[level].test(item))
        return true;
    else
        return false;
}

#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\ai\\ai_long_term_goal_director.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "ai_long_term_goal_director.h"

#include "unsorted.h"
#include "ai_analyser.h"
#include "ai_cheat_manager.h"
#include "ai_faction_base.h"
#include "../faction.h"
#include "../faction_db.h"
#include "../faction_diplomacy.h"
#include "../settlement.h"
#include "../world.h"
#include "../world_map.h"

// The diplomatic stances (DiplomaticRelation::m_stance).
enum {
    STANCE_ALLIED = 0,
    STANCE_AT_WAR = 600
};

inline float ratio(const int& a, const int& b)
{
    float x = (float)a;
    float y = (float)b;
    return x / y;
}

// The AI analyser's figures (codeii_analyser.cpp).
inline int ai_value(const int& faction)
{
    return lbl_012D0F78[faction].m_value;
}

inline int army_strength(const int& faction)
{
    return lbl_012D0F74[faction].m_strength;
}

inline int settlement_value(const int& faction)
{
    return lbl_012D0F54[faction].m_value;
}

inline int neighbour_wars(const int& faction)
{
    return lbl_012D0F7C[faction].m_value;
}

inline int unk_012D0F84(const int& faction)
{
    return lbl_012D0F84[faction].m_value;
}

inline int border_strength(const int& faction, const int& other)
{
    return lbl_012D154C[faction * 21 + other].m_value;
}

inline bool allies_border(const int& faction, const int& other)
{
    return lbl_012D1550[faction * 21 + other].m_set;
}

inline bool unk_012D1551(const int& faction, const int& other)
{
    return lbl_012D1551[faction * 21 + other].m_set;
}

inline bool is_strongest_neighbour(const int& faction, const int& other)
{
    return lbl_012D1552[faction * 21 + other].m_strongest_neighbour;
}

AIFactionGoal::AIFactionGoal()
{
    m_stance = 1;
    m_stance_priority = 0;
    m_war_goal = 5;
    m_war_priority = 0;
    m_at_war = false;
    m_11 = false;
    m_12 = false;
    m_13 = false;
    m_14 = false;
    m_18 = 0;
    m_1c = 0;
    m_20 = 0;
    m_24 = 0;
    m_28 = 0;
}

AILongTermGoalDirectorBase::AILongTermGoalDirectorBase(FactionAI* ai)
    : m_ai(ai), m_faction(ai->faction()), m_10(0), m_14(0.0f), m_18(false), m_1c(-1), m_20(-1), m_24(0), m_3c4(3), m_3c8(0)
{
}

// How the faction stands towards each other faction, from their strength along their border, the AI's threat
// values and their settlements: m_stance 5 (and the highest priorities) for an enemy it's at war with and much
// stronger than, down to 0 for one it's allied with or at peace with and far from.
void AILongTermGoalDirectorBase::update_stances()
{
    // NONMATCHING - stack slot order differs, and this keeps two more temporaries
    int me = m_ai->m_faction_id;
    int war_border = 0;
    int peace_border = 0;
    for (unsigned int i = 0; i < 21; i++) {
        int other = i;
        if (lbl_01181378 == other)
            continue;
        Faction* faction = lbl_018CD818->faction(other);
        AIFactionGoal* goal = &m_goals.m_goals[i];
        int stance = lbl_018CD818->m_diplomacy.stance(me, other);
        if (other == me || faction == 0)
            continue;
        // A faction that doesn't border it only counts while they're at war.
        if (!faction->neighbours().contains(me) && m_ai->faction()->m_settlements.size() > 0) {
            if (stance == STANCE_AT_WAR) {
                goal->m_at_war = true;
                if (!m_c.contains(other))
                    goal->m_11 = faction->shadowing() != me;
            }
            continue;
        }
        int theirs = lbl_012D154C[other * 21 + me].m_value;
        if (theirs <= 0)
            theirs = 1;
        int their_value = lbl_012D0F78[other].m_value;
        if (their_value <= 0)
            their_value = 1;
        int mine = lbl_012D154C[me * 21 + other].m_value;
        if (mine <= 0)
            mine = 1;
        int my_value = lbl_012D0F78[me].m_value;
        if (my_value <= 0)
            my_value = 1;
        float strength = ratio(mine, theirs);
        float value = ratio(my_value, their_value);
        float wealth = ratio(settlement_value(me), settlement_value(other));
        if (stance == STANCE_AT_WAR) {
            goal->m_at_war = true;
            if (AICheatManager::flag_war(me, other))
                goal->m_11 = true;
            war_border += border_strength(other, me);
            if (fn_004E99ED(me, other) && strength < 0.3f && value < 0.25f && wealth < 0.2f) {
                goal->m_stance = 5;
                goal->m_stance_priority = 900;
                goal->m_11 = faction->shadowing() != me;
                goal->m_13 = faction->shadowing() != me;
                goal->m_18 = 7;
                continue;
            }
            if (fn_004E99ED(me, other) && strength < 0.5f && value < 0.5f) {
                goal->m_stance = 5;
                goal->m_stance_priority = 850;
                goal->m_11 = faction->shadowing() != me;
                goal->m_18 = 7;
                continue;
            }
            if (strength < 0.5f && value > 1.0f) {
                goal->m_stance = 5;
                goal->m_stance_priority = 800;
                goal->m_18 = 7;
                continue;
            }
            if (strength > 1.0f && value < 0.7f && wealth > 1.0f) {
                goal->m_stance = 2;
                goal->m_stance_priority = 750;
                goal->m_18 = 4;
                continue;
            }
            goal->m_stance = 3;
            goal->m_stance_priority = 700;
            continue;
        }
        peace_border += border_strength(other, me);
        if (strength < 0.5f && value > 1.0f) {
            goal->m_stance = 5;
            goal->m_stance_priority = 650;
            continue;
        }
        if (strength < 0.5f && value < 1.0f) {
            goal->m_stance = 4;
            goal->m_stance_priority = 600;
            continue;
        }
        if (!unk_012D1551(other, me) && stance == STANCE_ALLIED) {
            goal->m_stance = 0;
            goal->m_stance_priority = 550;
            continue;
        }
        if (unk_012D1551(other, me) && neighbour_wars(other) == 0
            && neighbour_wars(me) == 0) {
            goal->m_stance = 3;
            goal->m_stance_priority = 500;
            continue;
        }
        if (neighbour_wars(other) == 0 && neighbour_wars(me) == 0) {
            goal->m_stance = 1;
            goal->m_stance_priority = 450;
            continue;
        }
        goal->m_stance = 0;
        goal->m_stance_priority = 400;
    }
    if (m_ai->faction()->neighbours().contains(lbl_01181378)) {
        AIFactionGoal* rebels = &m_goals.m_goals[lbl_01181378];
        rebels->m_at_war = true;
        rebels->m_stance = 1;
        rebels->m_stance_priority = 300;
        war_border += border_strength(lbl_01181378, me);
    }
}

// Which neighbours the faction should go to war with, and how much: a goal (m_war_goal) and priority for each,
// from their strength along the border and their settlements. m_18 is then set when the faction has few targets,
// a threat value over 1000 and isn't a superfaction or the rebels. A faction under another (bit 3 of their
// relation's flags) does none of this.
void AILongTermGoalDirectorBase::choose_war_targets()
{
    // NONMATCHING - stack slot order differs, and this keeps one more temporary
    int me = m_ai->m_faction_id;
    if (unk_012D0F84(me) != 21) {
        m_18 = false;
        return;
    }
    FactionSet neighbours = m_ai->faction()->neighbours();
    unsigned int targets = 0;
    unsigned int wars = 0;
    bool at_war = false;
    for (unsigned int i = 0; i < 21; i++) {
        int other = i;
        if (lbl_01181378 == other)
            continue;
        Faction* faction = lbl_018CD818->faction(other);
        AIFactionGoal* goal = &m_goals.m_goals[i];
        if (other == me || faction == 0 || !neighbours.contains(other))
            continue;
        if (is_fellow_family(other) && !lbl_018CD818->unk_0045428B(me))
            continue;
        if (!AICheatManager::may_target(me, other))
            continue;
        if (lbl_018CD818->m_diplomacy.unk_005A0F3E(other, me) || protected_by_family(other))
            continue;
        targets++;
        int theirs = lbl_012D154C[other * 21 + me].m_value;
        if (theirs <= 0)
            theirs = 1;
        int mine = lbl_012D154C[me * 21 + other].m_value;
        if (mine <= 0)
            mine = 1;
        int stance = lbl_018CD818->m_diplomacy.stance(me, other);
        float strength = ratio(mine, theirs);
        float wealth = ratio(settlement_value(me), settlement_value(other));
        if (stance == STANCE_AT_WAR) {
            at_war = true;
            if (strength > 5.0f && stronger_armies(other) && wealth > 1.0f) {
                goal->m_war_goal = 1;
                goal->m_war_priority = 800;
                goal->m_14 = faction->shadowing() != me;
                wars++;
                continue;
            }
            if (strength > 2.0f && wealth < 1.0f && neighbour_wars(other) > 1) {
                goal->m_war_goal = 1;
                goal->m_war_priority = 750;
                wars++;
                continue;
            }
            if (strength > 2.0f && wealth > 1.0f && allies_border(me, other)) {
                goal->m_war_goal = 1;
                goal->m_war_priority = 700;
                wars++;
                continue;
            }
            if (strength > 1.0f && wealth > 1.0f) {
                goal->m_war_goal = 0;
                goal->m_war_priority = 650;
                wars++;
                continue;
            }
            if (strength > 1.0f && wealth < 1.0f) {
                goal->m_war_goal = 2;
                goal->m_war_priority = 600;
                goal->m_18 = 10;
                wars++;
                continue;
            }
            goal->m_war_goal = 3;
            goal->m_war_priority = 550;
            goal->m_18 = 8;
            wars++;
            continue;
        }
        // A new war, only while the faction isn't fighting another neighbour (or just one).
        if (neighbour_wars(me) == 0 && strength > 3.0f && stronger_armies(other)) {
            goal->m_war_goal = 1;
            goal->m_war_priority = 500;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 1 && strength > 3.0f && stronger_than_its_enemies(other)) {
            goal->m_war_goal = 1;
            goal->m_war_priority = 450;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && strength > 2.0f && allies_border(me, other)
            && weaker_than_allies(other)) {
            goal->m_war_goal = 1;
            goal->m_war_priority = 400;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && strength > 1.5f && wealth > 1.5f) {
            goal->m_war_goal = 0;
            goal->m_war_priority = 350;
            goal->m_18 = 2;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && strength > 1.5f && allies_border(me, other)
            && weaker_than_allies(other) && is_strongest_neighbour(me, other)) {
            goal->m_war_goal = 3;
            goal->m_war_priority = 300;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && strength > 2.0f && wealth < 1.0f
            && is_strongest_neighbour(me, other) && neighbour_wars(other) > 0) {
            goal->m_war_goal = 2;
            goal->m_war_priority = 250;
            goal->m_18 = 6;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && wealth > 1.0f && is_strongest_neighbour(me, other)) {
            goal->m_war_goal = 4;
            goal->m_war_priority = 200;
            goal->m_18 = 1;
            wars++;
            continue;
        }
        if (neighbour_wars(me) == 0 && wealth > 1.0f && is_top_1c(other)) {
            goal->m_war_goal = 4;
            goal->m_war_priority = 150;
            goal->m_18 = 1;
            wars++;
            continue;
        }
        // (The same test as two above, so it never passes.)
        if (neighbour_wars(me) == 0 && wealth > 1.0f && is_strongest_neighbour(me, other)) {
            goal->m_war_goal = 4;
            goal->m_war_priority = 100;
            goal->m_18 = 1;
            wars++;
            continue;
        }
    }
    if (m_ai->faction()->neighbours().contains(lbl_01181378)) {
        AIFactionGoal* rebels = &m_goals.m_goals[lbl_01181378];
        rebels->m_war_goal = 3;
        rebels->m_war_priority = 850;
        wars++;
        targets++;
    }
    // A long war with the player: its strongest neighbour gets 800 more priority, or a new goal at 900.
    for (unsigned int j = 0; j < 21; j++) {
        int other = j;
        if (lbl_01181378 == other)
            continue;
        AIFactionGoal* goal = &m_goals.m_goals[j];
        if (other == me || !lbl_018CD818->is_faction_alive(other) || !neighbours.contains(other))
            continue;
        if (is_fellow_family(other))
            continue;
        if (AICheatManager::punish_long_war(me, other)) {
            if (goal->m_war_goal != 5) {
                goal->m_war_priority += 800;
            } else {
                goal->m_war_goal = 0;
                goal->m_war_priority = 900;
                goal->m_18 = 10;
                wars++;
            }
        }
    }
    m_18 = false;
    bool can_start = false;
    if (m_ai->faction()->description()->m_dc != 0)
        can_start = targets < 10;
    else
        can_start = targets < 4 && wars < 1 ? true : false;
    if (can_start && ai_value(me) > 1000 && m_ai->unk_c()->m_8 != 0 && m_ai->m_faction_id != lbl_01181378
        && !m_ai->faction()->has_subfactions())
        m_18 = true;
}

// Whether the faction's armies are stronger than the other's.
bool AILongTermGoalDirectorBase::stronger_armies(int faction)
{
    // NONMATCHING - stack slot order differs
    int me = m_ai->m_faction_id;
    int mine = lbl_012D0F74[me].m_strength;
    int theirs = lbl_012D0F74[faction].m_strength;
    float strength = ratio(mine, theirs);
    return strength > 1.0f ? 1 : 0;
}

// Whether the faction's armies are stronger than the other's and its other enemies' together.
bool AILongTermGoalDirectorBase::stronger_than_its_enemies(int faction)
{
    // NONMATCHING - stack slot order differs
    int total = 0;
    int me = m_ai->m_faction_id;
    for (unsigned int i = 0; i < 21; i++) {
        int other = i;
        if (lbl_01181378 == other)
            continue;
        if (lbl_018CD818->has_faction(other) && other != faction
            && lbl_018CD818->m_diplomacy.stance(me, other) == STANCE_AT_WAR)
            total += army_strength(other);
    }
    total += army_strength(faction);
    return total < army_strength(me);
}

// Whether the other faction's armies are weaker than the faction's and those of its allies that border the other
// and aren't allied to it.
bool AILongTermGoalDirectorBase::weaker_than_allies(int faction)
{
    // NONMATCHING - stack slot order differs
    int me = m_ai->m_faction_id;
    Faction* target = lbl_018CD818->faction(faction);
    FactionSet neighbours = target->neighbours();
    int total = army_strength(me);
    for (unsigned int i = 0; i < 21; i++) {
        int other = i;
        if (lbl_01181378 == other)
            continue;
        if (lbl_018CD818->has_faction(other) && neighbours.contains(other)
            && lbl_018CD818->m_diplomacy.stance(me, other) == STANCE_ALLIED
            && lbl_018CD818->m_diplomacy.stance(faction, other) != STANCE_ALLIED)
            total += army_strength(other);
    }
    return army_strength(faction) < total;
}

// Whether the other faction's goal has the highest m_1c (the rebels' aside).
bool AILongTermGoalDirectorBase::is_top_1c(int faction)
{
    int best = 0;
    for (unsigned int i = 0; i < 21; i++) {
        if (lbl_01181378 == (int)i)
            continue;
        if (m_goals.m_goals[i].m_1c > best)
            best = m_goals.m_goals[i].m_1c;
    }
    return m_goals.m_goals[faction].m_1c == best;
}

// Whether the faction is the other's superfaction, or they have the same one.
bool AILongTermGoalDirectorBase::shares_superfaction(int faction)
{
    Faction* other = lbl_018CD818->faction(faction);
    if (other) {
        Faction* mine = m_ai->faction();
        if (mine->has_subfactions() && mine->id() == other->superfaction()
            || mine->superfaction() != 21 && mine->superfaction() == other->superfaction())
            return true;
    }
    return false;
}

// Whether the other faction is one of the faction's superfaction's (the Roman families), while the senate's
// unk_004542E9 holds for neither of them.
bool AILongTermGoalDirectorBase::is_fellow_family(int faction)
{
    if (!lbl_018CD818->faction(faction))
        return false;
    if (shares_superfaction(faction) && !lbl_018CD818->unk_004542E9(m_ai->faction_id())
        && !lbl_018CD818->unk_004542E9(faction))
        return true;
    return false;
}

// Whether the other faction has bit 3 of the relation's flags (perhaps a protectorate) with the faction's
// superfaction or another of the superfaction's factions.
bool AILongTermGoalDirectorBase::protected_by_family(int faction)
{
    if (!lbl_018CD818->faction(faction))
        return false;
    int superfaction = m_ai->faction()->superfaction();
    if (superfaction != 21) {
        if (lbl_018CD818->m_diplomacy.unk_005A0F3E(faction, superfaction))
            return true;
        Faction* super = lbl_018CD818->faction(superfaction);
        for (unsigned int i = 0; i < 21; i++) {
            int other = i;
            if (other != m_ai->faction_id() && super->is_superfaction_of(other)
                && lbl_018CD818->m_diplomacy.unk_005A0F3E(faction, other))
                return true;
        }
    }
    return false;
}

// The region to invade by sea: the best target_score of the regions that the coastal regions of the faction's
// largest region group can reach by sea. The target's owner gets a war goal for it (1 at 725, or 3 at 100).
void AILongTermGoalDirectorBase::choose_sea_target()
{
    // NONMATCHING - stack slot order differs
    int me = m_ai->m_faction_id;
    int from = -1;
    int target = -1;
    int best = -1;
    AIRegionGroup* group = fn_004E94E5(me, 0);
    if (group) {
        int value = group_value(group);
        for (unsigned int i = 1; i < fn_004E9547(me); i++) {
            AIRegionGroup* other = fn_004E94E5(me, i);
            int other_value = group_value(other);
            if (other_value > value) {
                group = other;
                value = other_value;
            }
        }
        for (unsigned int i = 0; i < group->m_regions.size(); i++) {
            int region_id = group->m_regions.m_data[i].m_region;
            Region* region = lbl_018CD80C->region(region_id);
            if (region->m_coast.size() != 0) {
                Array<AIRegionTarget>* targets = (Array<AIRegionTarget>*)region->m_sea_targets;
                for (unsigned int k = 0; k < targets->size(); k++) {
                    int score = target_score(targets->m_data[k], region_id);
                    if (score > best) {
                        from = region_id;
                        target = targets->m_data[k].m_region;
                        best = score;
                    }
                }
            }
        }
    }
    if (from > -1) {
        m_1c = target;
        m_20 = from;
        int owner = lbl_018CD80C->m_regions[target].m_owner->id();
        if (m_faction->description()->m_dc != 0) {
            m_goals.m_goals[owner].m_war_goal = 1;
            m_goals.m_goals[owner].m_war_priority = 725;
        } else {
            m_goals.m_goals[owner].m_war_goal = 3;
            m_goals.m_goals[owner].m_war_priority = 100;
        }
    } else {
        m_18 = false;
    }
}

// A region group's value: 100 for each of its regions, or -1 without its unk_4c.
int AILongTermGoalDirectorBase::group_value(AIRegionGroup* group)
{
    if (!group->unk_4c())
        return -1;
    return group->m_regions.size() * 100;
}

// A region's score as a target: -1 when it's the faction's own, has no coast, borders it by land, is a fellow
// family's or borders one; otherwise 100, plus 1200 when it's very near (or 5 for each step under 240), 500 when
// fn_004E99BE holds for it (or 400 for a rebel region), less 400 for a capital, 150 more when it isn't where FROM is
// (by the regions' m_54), the analyser's four region values, and 5000 against the player on very hard.
int AILongTermGoalDirectorBase::target_score(const AIRegionTarget& target, int from)
{
    // NONMATCHING - stack slot order differs
    int region_id = target.m_region;
    Region* region = lbl_018CD80C->region(region_id);
    if (region->m_owner == m_faction)
        return -1;
    if (region->m_coast.size() == 0)
        return -1;
    if (region->m_coast.size() == 0)
        return -1;
    int me = m_ai->m_faction_id;
    if (!region->has_one_neighbour() && region->m_owner->neighbours().contains(me))
        return -1;
    int owner = region->m_owner->id();
    if (is_fellow_family(owner))
        return -1;
    if (borders_fellow_family(region_id))
        return -1;
    if (borders_region(region_id))
        return -1;
    int score = 100;
    int distance = float_to_int(target.m_distance - 30.0f);
    if (distance < 0) {
        score += 1200;
    } else {
        if (distance > 240)
            return -1;
        score += (240 - distance) * 5;
    }
    if (fn_004E99BE(region_id))
        score += 500;
    else if (lbl_01181378 == owner)
        score += 400;
    if (region->settlement()->is_capital())
        score -= 400;
    if (region->unk_54() != lbl_02921A94->region(from)->unk_54())
        score += 150;
    score += fn_004E4AC6(region_id);
    score += fn_004E4B16(region_id);
    score += fn_004E4BE6(region_id);
    score += fn_004E4C70(region_id);
    if (AICheatManager::target_player_bonus(me, owner))
        score += 5000;
    return score;
}

// Whether a region beside the region is a fellow family's.
bool AILongTermGoalDirectorBase::borders_fellow_family(int region)
{
    Region* here = lbl_018CD80C->region(region);
    for (unsigned int i = 0; i < here->num_neighbours(); i++) {
        Faction* owner = lbl_018CD80C->m_regions[here->m_neighbours[i].m_region].m_owner;
        if (owner && is_fellow_family(owner->id()))
            return true;
    }
    return false;
}

// Whether the region, or a region beside it, is one of the other factions' regions beside the faction's own.
bool AILongTermGoalDirectorBase::borders_region(int region)
{
    Array<int>& bordering = m_ai->faction()->m_bordering_regions;
    for (unsigned int i = 0; i < bordering.size(); i++) {
        if (bordering.m_data[i] == region)
            return true;
    }
    Region* here = lbl_018CD80C->region(region);
    for (unsigned int j = 0; j < here->num_neighbours(); j++) {
        int neighbour = here->m_neighbours[j].m_region;
        for (unsigned int k = 0; k < bordering.size(); k++) {
            if (bordering.m_data[k] == neighbour)
                return true;
        }
    }
    return false;
}

AILongTermGoalDirector::AILongTermGoalDirector(FactionAI* ai) : AILongTermGoalDirectorBase(ai)
{
}

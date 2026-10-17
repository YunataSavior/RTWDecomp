// The strategy map's road planning.
#include "road.h"
#include "character.h"

#include "unsorted.h"

RoadPoint::RoadPoint(int index)
{
    x = index % lbl_018CD80C->width();
    y = index / lbl_018CD80C->width();
}

inline bool has_aerial_map()
{
    return lbl_015FC11C != 0;
}

inline bool in_bounds(const RoadPoint& point)
{
    return point.x < lbl_018CD86C->width() && point.y < lbl_018CD86C->height();
}

inline unsigned char direction_bit(const RoadPoint& from, const RoadPoint& to)
{
    return (lbl_010D0A00 + 1)[(to.y - from.y + 1) * 3 + (to.x - from.x)];
}

// Whether the terrain stops a step from one tile into the next (tile): its blocked directions, the sea, or a river
// that can't be crossed there.
bool PathFinder::is_blocked(const RoadPoint& from, const RoadPoint& to, Tile* tile)
{
    // NONMATCHING - the original has a movzx eax, al after each of two bool calls
    if (tile->m_25 == 0xff || (tile->m_25 & direction_bit(from, to)))
        return true;
    Tile* from_tile = lbl_018CD80C->tile(from);
    bool by_land = true;
    if (m_c == 3) {
        by_land = false;
        if (character() && from_tile->m_coast)
            return true;
    }
    bool sea = tile->is_sea();
    if (lbl_012D0604 == 6 || lbl_012D0604 == 7)
        return sea == (lbl_012D0604 == 6);
    if (sea) {
        if (character()) {
            if (m_c != 3 && unk_004AB253(from, to))
                return false;
        } else if (m_c == 3 && from_tile->m_coast) {
            if (lbl_018CD80C->m_regions[from_tile->m_region].m_port_position != from)
                return true;
        }
        return by_land;
    }
    bool shore = from_tile->is_sea() && tile->m_coast;
    if (tile->is_river()) {
        if (lbl_012D0604 == 3)
            return !lbl_018CD80C->can_cross_river(from.x, from.y, to.x, to.y);
        return !tile->is_bridge() && !tile->m_road_level;
    }
    if (character() && is_mode_3() && shore && lbl_012D0604 != 2) {
        if (unk_004AB325(character()))
            return by_land;
    }
    if (character() && shore) {
        if (unk_004AADAE(character()->position()) && unk_004A7D84(character()->unk_00580C1E(), to))
            return true;
    }
    if (!character() && m_c == 3 && shore)
        return by_land;
    return !by_land;
}

// Collects the spies in a settlement that aren't its owner's: every spy on its tile whose faction isn't the owner,
// whether the owner can see it or not, and whatever the two factions' relations.
void PathFinder::foreign_spies(Settlement* settlement, Array<Character*>* spies)
{
    StrategyObject* object = lbl_018CD80C->tile(*settlement->position())->m_objects;
    while (object) {
        Character* character = object->unk_28();
        if (character && character->faction() != settlement->unk_9c() && character->type() == 0)
            spies->push_back(character);
        object = object->m_next;
    }
}

// Plans a road from one tile to another into m_path, goal first. Returns 0, or 1 when the goal is off the map or
// blocked, or 2 when there is no path.
int PathFinder::find_road_path(const RoadPoint& from, const RoadPoint& to)
{
    lbl_012D0604 = 3;
    if (!in_bounds(to) || unk_004A605B(to)) {
        m_status = 1;
        lbl_012D0604 = 0;
        return m_status;
    }
    m_path.clear();
    set_character(0);
    PathNode* node = lbl_012D07C0->search(2, from, to, true);
    if (node) {
        while (node) {
            m_path.push_back(node->m_point);
            node = node->m_parent;
        }
        return 0;
    }
    m_status = 2;
    return m_status;
}

// Lays region's roads to its neighbours. A road a neighbour already planned towards it is finished (its pending part
// drawn, or extended to the settlement); otherwise a road is planned from settlement to settlement, refused if it
// crosses a third region. Then the port road.
void PathFinder::plan_roads(int region_id, unsigned char level)
{
    RoadPoint source;
    RoadPoint to;
    Region* region = &lbl_018CD80C->m_regions[region_id];
    lbl_018CD80C->unk_00482B16(region_id);
    if (level) {
        region->set_road_level(level);
    } else {
        region->raise_road_level();
        level = region->m_road_level;
    }
    int i = 0;
    for (i = 0; i < region->num_neighbours(); i++) {
        int neighbour = region->m_neighbours[i].m_region;
        if (!lbl_018CD80C->is_sea_region(neighbour)) {
            Region* target = &lbl_018CD80C->m_regions[neighbour];
            source = region->m_settlement_position;
            to = lbl_018CD80C->m_regions[region->m_neighbours[i].m_region].m_settlement_position;
            Road* road = 0;
            for (unsigned int j = 0; j < target->m_neighbours.size(); j++) {
                if (target->m_neighbours[j].m_road && target->m_neighbours[j].m_road->other() == region_id) {
                    road = target->m_neighbours[j].m_road;
                    break;
                }
            }
            if (road) {
                if (road->m_pending.size()) {
                    campaign_roads().upgrade(road);
                } else {
                    Array<RoadPoint>& path = road->m_drawn;
                    RoadPoint tip = *path.last();
                    RoadPoint settlement = region->m_settlement_position;
                    int blocked = find_road_path(tip, settlement);
                    for (unsigned int k = 0; k < m_path.size(); k++) {
                        Tile* tile = lbl_018CD80C->tile(m_path[k]);
                        if (!lbl_018CD80C->is_sea_region(tile->m_region) && tile->m_region != region_id
                            && tile->m_region != neighbour) {
                            blocked = 1;
                            break;
                        }
                        if (tile->m_region != region_id && !target->road_level()) {
                            m_path.erase(k);
                            k--;
                        } else {
                            break;
                        }
                    }
                    if (!blocked) {
                        m_path.pop_back();
                        campaign_roads().upgrade(road);
                    }
                }
            } else {
                int blocked = find_road_path(source, to);
                int border = -1;
                for (unsigned int k = 0; k < m_path.size(); k++) {
                    Tile* tile = lbl_018CD80C->tile(m_path[k]);
                    if (!lbl_018CD80C->is_sea_region(tile->m_region) && tile->m_region != region_id
                        && tile->m_region != neighbour) {
                        blocked = 1;
                        break;
                    }
                    if (tile->m_region != region_id && !target->road_level())
                        border = k;
                    else
                        break;
                }
                if (!blocked) {
                    campaign_roads().create(region_id, neighbour, border);
                } else {
                    for (unsigned int k = 0; k < target->m_neighbours.size(); k++)
                        if (target->m_neighbours[k].m_region == region_id)
                            break;
                }
            }
        }
    }
    lbl_018CD80C->unk_0047FC44(region_id);
    for (i = 0; i < region->num_neighbours(); i++)
        lbl_018CD80C->unk_0047FC44(i);
    if ((region->port() && region->port()->unk_184())
        || (region->m_settlement_position != RoadPoint() && region->m_port_position != RoadPoint())) {
        source = region->m_settlement_position;
        to = region->m_port_position;
        int blocked = find_road_path(source, to);
        if (blocked)
            return;
        campaign_roads().create_port_road(region_id);
    }
}

// Builds a region's roads at a level: plans them if it has none yet, otherwise raises the level of its road tiles.
void PathFinder::build_roads(int region_id, unsigned char level)
{
    Region* region = &lbl_018CD80C->m_regions[region_id];
    if (!region->road_level()) {
        plan_roads(region_id, level);
    } else {
        region->raise_road_level();
        for (unsigned int i = 0; i < region->m_tiles.size(); i++) {
            int index = region->m_tiles[i];
            if (lbl_018CD80C->m_tiles[index].m_road_level)
                lbl_018CD80C->tile(index)->m_road_level = region->road_level();
        }
    }
    if (region->port())
        campaign_roads().update_connected_regions(region);
    lbl_018CD80C->unk_0048257A(region_id);
    if (has_aerial_map())
        fn_009CAB2E(region_id, region->road_level());
}

// Sets the character whose path is planned next: its kind and faction decide what it can enter. Roads have none.
void PathFinder::set_character(Character* character)
{
    m_watched.set(character);
    if (character) {
        m_c = character->type();
        m_faction = character->unk_00580C1E();
    } else {
        m_c = 6;
        m_faction = 0;
    }
}

// Whether the path can step from one tile to the next: 1 when it can, 0 when it can't, 2 when the step would end a
// move the character can't make there. Besides the terrain (is_blocked), it checks what stands on the tile: the
// goal can always be entered, and other tiles can't hold another faction's structure or character.
int PathFinder::can_move(const RoadPoint& from, const RoadPoint& to, const RoadPoint& goal)
{
    // NONMATCHING - stack slot order differs, and the original keeps the structure's owner in a temporary
    Character* mover = character();
    bool hidden = false;
    FactionUnk180* seen = 0;
    if (lbl_018CD818->unk_00452977() && mover && lbl_012D0604 != 2) {
        seen = mover->unk_00580C1E()->m_180;
        hidden = seen->at(to) < 2 && !seen->unk_00493794(to);
    }
    bool open = false;
    Tile* target = lbl_018CD80C->tile(to);
    if (hidden || to == goal) {
        open = true;
    } else if (!mover && !m_faction) {
        open = true;
    } else if (target->m_24_2) {
        TileStructure* structure = unk_004A76B7(to);
        if (structure->m_30->unk_a0() == m_faction)
            open = true;
    } else if (!unk_004A7CA9(m_faction, to)) {
        open = true;
    }
    if (to != goal) {
        TileUnk84* occupant = unk_004A74BA(from);
        if (occupant && (bool)occupant->m_84)
            open = false;
    }
    if (open) {
        bool by_sea = lbl_012D0604 != 2 && ((mover && mover->unk_0058D0C5()) || (!mover && m_c == 3));
        if (!by_sea && !(!mover && m_c == 6) && to == goal && !unk_004AB135(goal, 0)) {
            if (mover) {
                if (unk_004A6DA0(to, mover)) {
                    bool from_flag = lbl_018CD80C->tile(from)->m_24_5;
                    bool to_flag = target->m_24_5;
                    if (m_c != 3 && from_flag && !to_flag)
                        return 0;
                }
                return 1;
            }
            if (unk_004A7097(to, m_c, 1))
                return 1;
            return 0;
        }
        if (!is_blocked(from, to, target)) {
            if (lbl_018CD86C->contains(to)) {
                if (mover && m_c != 3 && to != goal && !by_sea) {
                    bool left_flag = lbl_018CD80C->tile(from)->m_24_5;
                    bool entered_flag = target->m_24_5;
                    if (left_flag && !entered_flag)
                        return 0;
                }
                bool blocker_flag = false;
                Character* blocker = 0;
                if (!hidden) {
                    blocker = unk_004A95F3(m_faction, to, &blocker_flag);
                    if (blocker && blocker_flag && lbl_012D0604 != 2) {
                        if (!unk_004AC9E5(m_faction, blocker))
                            blocker = 0;
                    }
                    if (to.unk_004B61C2(goal)) {
                        if (blocker || target->m_24_0 || target->m_24_1 || target->m_landmark) {
                            Tile* goal_tile = lbl_018CD80C->tile(goal);
                            if (goal_tile->m_24_6 || goal_tile->m_24_5 || goal_tile->m_24_0 || goal_tile->m_24_1
                                || goal_tile->m_24_2) {
                                if (!unk_004AA85D(to))
                                    return 0;
                            }
                        }
                    }
                    if (mover && lbl_012D08A4 == 0 && to != goal) {
                        if ((unsigned int)(target->m_28 ^ mover->unk_78()) > 0) {
                            static Array<int> s_tiles;
                            s_tiles.clear();
                            unk_004B68A4(mover, to, &s_tiles, 0);
                            StrategyObject* object = mover->unk_00586278(&s_tiles);
                            if (object) {
                                if (!seen || (seen && seen->at(*object->position()) >= 2)) {
                                    int found = unk_004A7F3C(m_faction, goal);
                                    if (!found || !mover->unk_00585CF4(&s_tiles, found))
                                        return 2;
                                }
                            }
                        }
                    }
                }
                if (!blocker)
                    return 1;
                if (to != goal) {
                    if (!blocker_flag || !blocker->army())
                        return 1;
                } else
                    return 1;
            }
        }
    }
    return 0;
}

// The cost of a step into the next tile: its ground, scaled by its movement factor, and by the road on it.
float PathFinder::step_cost(const RoadPoint& from, const RoadPoint& to)
{
    // NONMATCHING - one table index is loaded with push 3; pop eax, and stack slot order differs
    if (from != to) {
        Tile* dest = lbl_018CD86C->tile(to);
        float cost = lbl_010D01A0[dest->m_10];
        if (from.x == to.x || from.y == to.y)
            cost *= lbl_00FE60A0;
        else
            cost *= lbl_012D05F0;
        cost *= dest->m_movement_factor;
        if (dest->m_flags21)
            cost *= lbl_00FE60A4;
        if (lbl_012D0604 != 3) {
            cost *= lbl_00FE60C0[dest->m_road_level];
        } else if (!dest->m_road) {
            if (dest->m_river)
                cost *= lbl_00FE60AC;
            else
                cost *= lbl_00FE60A8;
        } else {
            cost *= lbl_00FE60C0[3];
        }
        return cost >= lbl_00FE609C ? cost : lbl_00FE609C;
    }
    return 0.0f;
}

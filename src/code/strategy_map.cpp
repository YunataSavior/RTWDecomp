// The strategy map's road planning.
#include "road.h"

#include "unsorted.h"

inline bool has_aerial_map()
{
    return lbl_015FC11C != 0;
}

inline bool in_bounds(const RoadPoint& point)
{
    return point.x < lbl_018CD86C->width() && point.y < lbl_018CD86C->height();
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
    unk_004B64C3(0);
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
        region->unk_00483E16(level);
    } else {
        region->unk_00483E6B();
        level = region->m_road_level;
    }
    int i = 0;
    for (i = 0; i < region->num_neighbours(); i++) {
        int neighbour = region->m_neighbours[i].m_region;
        if (!lbl_018CD80C->unk_00481149(neighbour)) {
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
                        if (!lbl_018CD80C->unk_00481149(tile->m_region) && tile->m_region != region_id
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
                    if (!lbl_018CD80C->unk_00481149(tile->m_region) && tile->m_region != region_id
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
        region->unk_00483E6B();
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

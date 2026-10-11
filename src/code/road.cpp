#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\road.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "road.h"

#include "unsorted.h"

inline bool has_aerial_map()
{
    return lbl_015FC11C != 0;
}

Road::Road()
{
    m_planner = 0;
    m_other = 0;
    m_traffic = 0;
    m_split = false;
    m_id = 0;
    m_display = -1;
}

Road::Road(int planner, int other)
{
    m_planner = planner;
    m_other = other;
    m_traffic = 0;
    m_split = false;
    m_id = 0;
    m_display = -1;
}

// Takes a path from the pathfinder (goal first): the tiles past the border go to the pending list, the rest are drawn.
void Road::set_path(Array<RoadPoint>& path, int border)
{
    for (int i = path.size() - 1; i >= 0; i--) {
        if (!m_split || i > border) {
            m_drawn.push_back(path[i]);
        } else {
            m_pending.push_back(path[i]);
            path.erase(i);
        }
    }
}

void Road::reveal()
{
    for (unsigned int i = 0; i < m_pending.size(); i++)
        m_drawn.push_back(m_pending[i]);
    m_pending.clear();
}

void Road::clear_drawn()
{
    m_drawn.clear();
}

void Road::set_road_level(int x, int y, int level)
{
    lbl_018CD80C->tile(x, y)->m_road_level = level;
}

void Road::add_traffic(int amount)
{
    float alpha;
    m_traffic += amount;
    if (m_traffic < 1)
        m_traffic = 1;
    alpha = k_4000 / (float)m_traffic;
    if (alpha < 6.0f)
        alpha = 6.0f;
    if (has_aerial_map())
        fn_009CAB82(m_display, alpha);
}

void Road::reset_traffic()
{
    m_traffic = 0;
    if (has_aerial_map())
        fn_009CAB82(m_display, 0.0f);
}

bool Road::unk_004A1DFD()
{
    bool found = false;
    Array<RoadPoint> points;
    for (Array<RoadPoint>::iterator it = m_drawn.data(); it != m_drawn.end(); ++it) {
        Settlement* source = lbl_018CD80C->m_regions[m_planner].m_settlement;
        Settlement* target = lbl_018CD80C->m_regions[m_other].m_settlement;
        int source_hit = 0;
        int target_hit = 0;
        if (source)
            source_hit = lbl_029219E0->unk_004A7EA9(source, &*it);
        if (target)
            target_hit = lbl_029219E0->unk_004A7EA9(target, &*it);
        if (source_hit || target_hit) {
            found = true;
            points.push_back(*it);
        }
    }
    if (found) {
        if (has_aerial_map())
            fn_009CABA1(m_display, points);
        return true;
    }
    return false;
}

static VersionRegistrar s_road_version("ROAD", 2);

bool Road::load(SaveStream& stream)
{
    if (!stream.is_open())
        return false;
    unsigned int version = VersionRegistry::instance()->version("ROAD");
    int old_this = stream.read_int();
    fn_0041369B(this, old_this);
    fn_0041393B(&stream, 0);
    m_planner = stream.read_int();
    m_other = stream.read_int();
    m_traffic = stream.read_int();
    m_id = stream.read_int();
    unsigned int size = stream.read_int();
    unsigned int i;
    for (i = 0; i < size; i++) {
        RoadPoint point;
        stream.read(&point, sizeof(point));
        m_drawn.push_back(point);
    }
    if (version >= 2) {
        size = stream.read_int();
        for (i = 0; i < size; i++) {
            RoadPoint point;
            stream.read(&point, sizeof(point));
            m_pending.push_back(point);
        }
    }
    stream.read(&m_split, 1);
    fn_0041393B(&stream, 0);
    return true;
}

bool Road::save(SaveStream& stream)
{
    if (!stream.unk_00F3D990())
        return false;
    Road* road = this;
    stream.write(&road, 4);
    fn_0041391C(&stream);
    stream.write(&m_planner, 4);
    stream.write(&m_other, 4);
    stream.write(&m_traffic, 4);
    stream.write(&m_id, 4);
    unsigned int size = m_drawn.size();
    stream.write(&size, 4);
    unsigned int i;
    for (i = 0; i < size; i++)
        stream.write_value(m_drawn[i]);
    size = m_pending.size();
    stream.write(&size, 4);
    for (i = 0; i < size; i++)
        stream.write_value(m_pending[i]);
    stream.write(&m_split, 1);
    fn_0041391C(&stream);
    return true;
}

void Road::unk_004A2229()
{
}

RoadManager::RoadManager()
{
    m_c = false;
}

void RoadManager::create(int planner, int other, int border)
{
#line 253
    Road* road = NEW Road(planner, other);
    road->m_split = border != -1 ? true : false;
    road->set_path(lbl_029219E0->m_path, border);
    register_tiles(road);
    road->m_id = free_id();
    m_roads.push_back(road);
    lbl_018CD80C->add_road(planner, other, road);
    if (has_aerial_map()) {
        int handle = fn_009CA89F(road->m_planner, road);
        road->m_display = handle;
    }
}

void RoadManager::create_port_road(int region)
{
#line 275
    Road* road = NEW Road(region, region);
    road->m_id = free_id();
    m_roads.push_back(road);
    road->set_path(lbl_029219E0->m_path, -1);
    register_tiles(road);
    lbl_018CD80C->region(region)->set_port_road(road);
    if (has_aerial_map()) {
        int handle = fn_009CA89F(road->m_planner, road);
        road->m_display = handle;
    }
}

void RoadManager::upgrade(Road* road)
{
    road->m_split = false;
    unsigned int waiting = road->m_pending.size();
    if (waiting)
        road->reveal();
    else
        road->set_path(lbl_029219E0->m_path, -1);
    register_tiles(road);
    int source = road->m_planner;
    int target = road->m_other;
    lbl_018CD80C->add_road(target, source, road);
    if (has_aerial_map())
        fn_009CAAA4(road->m_display, road);
}

void RoadManager::show()
{
    for (unsigned int i = 0; i < m_roads.size(); i++) {
        Region* source = &lbl_02921A94->m_regions[m_roads[i]->m_planner];
        Region* target = &lbl_02921A94->m_regions[m_roads[i]->m_other];
        if (m_roads[i]->m_split) {
            int handle = fn_009CA89F(m_roads[i]->m_planner, m_roads[i]);
            m_roads[i]->m_display = handle;
            fn_009CAB2E(m_roads[i]->m_planner, source->road_level());
        } else {
            int handle = fn_009CA94C(m_roads[i]->m_planner, m_roads[i]);
            m_roads[i]->m_display = handle;
            fn_009CAB2E(m_roads[i]->m_planner, source->road_level());
            fn_009CAB2E(m_roads[i]->m_other, target->road_level());
        }
    }
}

void RoadManager::hide()
{
    for (unsigned int i = 0; i < m_roads.size(); i++)
        m_roads[i]->m_display = -1;
}

// Marks the drawn tiles with the road and their region's road level.
void RoadManager::register_tiles(Road* road)
{
    for (Array<RoadPoint>::iterator it = road->m_drawn.begin(); it != road->m_drawn.end(); ++it) {
        Tile* tile = lbl_018CD80C->tile(it->x, it->y);
        if (tile->m_road_level != 4) {
            tile->m_road_level = lbl_018CD80C->m_regions[tile->m_region].road_level();
            if (tile->m_10 >= 7 || tile->m_10 <= 8)
                lbl_029219E0->unk_004AC914(*it);
            tile->m_road = road;
        }
    }
}

int RoadManager::free_id()
{
    int in_use = 0;
    for (unsigned int i = 0; i < m_roads.size(); i++)
        in_use += 1 << m_roads[i]->m_id;
    int mask = 1;
    int id = 0;
    while (mask & in_use) {
        mask <<= 1;
        id++;
    }
    return id;
}

void RoadManager::clear()
{
    while (m_roads.size() != 0) {
#line 392
        CA_DELETE(m_roads.back());
        m_roads.pop_back();
    }
}

void RoadManager::update_connected_regions(Region* region)
{
    static Array<int> regions;
    regions.clear();
    regions.push_back(region->id());
    unsigned int i;
    for (i = 0; i < m_roads.size(); i++) {
        if (region->id() == m_roads[i]->m_planner || region->id() == m_roads[i]->m_other) {
            regions.add_unique(m_roads[i]->m_planner);
            regions.add_unique(m_roads[i]->m_other);
        }
    }
    for (i = 0; i < regions.size(); i++) {
        Region* connected = &lbl_018CD80C->m_regions[regions[i]];
        connected->unk_00484763();
    }
}

void RoadManager::reset_connected_traffic(Region* region)
{
    static Array<int> regions;
    regions.clear();
    regions.push_back(region->id());
    unsigned int i;
    for (i = 0; i < m_roads.size(); i++) {
        if (region->id() == m_roads[i]->m_planner || region->id() == m_roads[i]->m_other) {
            regions.add_unique(m_roads[i]->m_planner);
            regions.add_unique(m_roads[i]->m_other);
            m_roads[i]->reset_traffic();
        }
    }
    for (i = 0; i < regions.size(); i++) {
        Region* connected = &lbl_018CD80C->m_regions[regions[i]];
        connected->unk_00484763();
    }
}

void RoadManager::unk_004A2AC7()
{
}

static VersionRegistrar s_road_manager_version("ROAD_MANAGER", 1);

bool RoadManager::load(SaveStream& stream)
{
    if (!stream.is_open())
        return false;
    switch (saved_version("ROAD_MANAGER")) {
    case 1: {
        unsigned short i = 0;
        RoadPoint unused;
        unsigned short num_roads = 0;
        clear();
        num_roads = stream.read_ushort();
        for (i = 0; i < num_roads; i++) {
#line 483
            Road* road = NEW Road();
            road->load(stream);
            m_roads.push_back(road);
        }
        return true;
    }
    }
    stream.fail();
    return false;
}

bool RoadManager::save(SaveStream& stream)
{
    if (!stream.unk_00F3D990())
        return false;
    unsigned short i = 0;
    unsigned short num_roads = m_roads.size();
    stream.write(&num_roads, 2);
    for (i = 0; i < num_roads; i++)
        m_roads[i]->save(stream);
    return true;
}

void RoadManager::unk_004A2C6D()
{
}

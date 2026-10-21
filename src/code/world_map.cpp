// The campaign map's tiles.
#include "unsorted.h"
#include "port.h"
#include "rebel_faction_db.h"
#include "strategy_map.h"
#include "world_map.h"
#include "map_constants.h"

void unk_009CA004(int x, int y, int width, int height, int);  // 0x009CA004: on the aerial map

bool Tile::is_river()
{
    return m_river || m_river_source;
}

int Tile::unk_0048A97F()
{
    return m_20_3 ? 0 : (m_10 == 8 ? 1 : (m_10 == 7 ? 3 : 0));
}

// The next famine threat, 4 at most.
int& next_famine_threat(int& threat)
{
    threat = threat == 4 ? 4 : threat + 1;
    return threat;
}

// Whether a road being planned can step from one tile to the next, a river tile. A bridge can be entered only along
// it, and only when no tile on either side of it has a road; a tile with no model only when the tiles before and after
// it along the step don't share its m_flags15.
bool StrategyMap::can_cross_river(int from_x, int from_y, int to_x, int to_y)
{
    Tile* to_tile = tile(to_x, to_y);
    int tile_model = to_tile->m_model;
    if (from_x != to_x && from_y == to_y) {
        if (tile_model != 0xffff) {
            if (tile_model == 0x6e || tile_model == 0x6f) {
                if (in_bounds(to_x, to_y + 1) == true && tile(to_x, to_y + 1)->m_road_level)
                    return false;
                if (in_bounds(to_x, to_y - 1) == true && tile(to_x, to_y - 1)->m_road_level)
                    return false;
            } else
                return false;
        } else if (to_tile->m_flags15) {
            if (in_bounds(to_x + 1, to_y) == true && tile(to_x + 1, to_y)->m_flags15)
                return false;
            if (in_bounds(to_x - 1, to_y) == true && tile(to_x - 1, to_y)->m_flags15)
                return false;
        }
    } else if (from_x == to_x && from_y != to_y) {
        if (tile_model != 0xffff) {
            if (tile_model == 0x6c || tile_model == 0x6d) {
                if (in_bounds(to_x + 1, to_y) == true && tile(to_x + 1, to_y)->m_road_level)
                    return false;
                if (in_bounds(to_x - 1, to_y) == true && tile(to_x - 1, to_y)->m_road_level)
                    return false;
            } else
                return false;
        } else if (to_tile->m_flags15) {
            if (in_bounds(to_x, to_y + 1) == true && tile(to_x, to_y + 1)->m_flags15)
                return false;
            if (in_bounds(to_x, to_y - 1) == true && tile(to_x, to_y - 1)->m_flags15)
                return false;
        }
    } else
        return false;
    return true;
}

// Reads one tile's road state from a saved game: the address its road had (fixed up later) and its road bits.
bool StrategyMap::load_tile_road(SaveStream& stream)
{
    // NONMATCHING - the original copies the stream to a temporary before calling is_open
    TileRoadRecord record;
    stream.read(&record, sizeof(record));
    if (!stream.is_open())
        return false;
    Tile* loaded = tile(record.m_x, record.m_y);
    loaded->m_road = record.m_road;
    loaded->m_road_level = record.m_road_level;
    loaded->m_flags15 = record.m_flags15;
    loaded->m_flags18 = record.m_flags18;
    loaded->m_flags21 = record.m_flags21;
    loaded->m_flags24 = record.m_flags24;
    loaded->m_flags27 = record.m_flags27;
    loaded->m_1c_30 = record.m_1c_30;
    loaded->m_flags20 = record.m_flags20;
    loaded->m_20_3 = record.m_20_3;
    return true;
}

// Builds each region's list of neighbours, and of the tiles on its border with each, by scanning the map from the
// north-west: each tile is compared with the tiles east, north-east, south and south-east of it. It also marks the
// tiles on borders between land regions and on coasts, then places each land region's settlement and port.
void StrategyMap::build_neighbours()
{
    // NONMATCHING - stack slot order differs, and the original reserves 0x10 more bytes of stack
    int row = 0;
    for (row = 0; row < (int)height(); row++) {
        for (int x = 0; x < (int)width(); x++) {
            Tile* cleared = tile(x, row);
            cleared->m_region_border = 0;
            cleared->m_sea_coast = 0;
            cleared->m_coast = 0;
        }
    }
    for (unsigned int i = 0; i < m_region_list.m_count; i++)
        m_regions[i].m_neighbours.clear();
    int x = 0;
    int last_x = (int)width() - 1;
    int rows = m_height;
    for (row = 0; row < rows; row++) {
        for (x = 0; x < last_x; x++) {
            int y = rows - row - 1;
            Tile* here = tile(x, y);
            int here_index = x + y * (int)width();
            Tile* east = tile(x + 1, y);
            int east_index = x + 1 + y * (int)width();
            Tile* north_east = 0;
            int north_east_index = 0;
            if (row > 0) {
                north_east = tile(x + 1, y + 1);
                north_east_index = x + 1 + (y + 1) * (int)width();
            }
            Tile* south = 0;
            int south_index = 0;
            Tile* south_east = 0;
            int south_east_index = 0;
            if (row < rows - 1) {
                south = tile(x, y - 1);
                south_index = x + (y - 1) * (int)width();
                south_east = tile(x + 1, y - 1);
                south_east_index = x + 1 + (y - 1) * (int)width();
            }
            if (here->m_region != east->m_region) {
                RegionNeighbour* to_east = m_regions[here->m_region].neighbour(east->m_region);
                to_east->add_border_tile(here_index);
                to_east = m_regions[east->m_region].neighbour(here->m_region);
                to_east->add_border_tile(east_index);
                if (m_regions[here->m_region].m_sea && !m_regions[east->m_region].m_sea) {
                    east->m_coast = 1;
                    here->m_sea_coast = 1;
                } else if (!m_regions[here->m_region].m_sea && m_regions[east->m_region].m_sea) {
                    here->m_coast = 1;
                    east->m_sea_coast = 1;
                    m_regions[here->m_region].m_coast.add_unique(CoastTile(here_index));
                } else {
                    here->m_region_border = 1;
                    east->m_region_border = 1;
                }
            }
            if (row > 0 && here->m_region != north_east->m_region) {
                RegionNeighbour* to_north_east = m_regions[here->m_region].neighbour(north_east->m_region);
                to_north_east->add_border_tile(here_index);
                to_north_east = m_regions[north_east->m_region].neighbour(here->m_region);
                to_north_east->add_border_tile(north_east_index);
                if (m_regions[here->m_region].m_sea && !m_regions[north_east->m_region].m_sea) {
                    north_east->m_coast = 1;
                    here->m_sea_coast = 1;
                } else if (!m_regions[here->m_region].m_sea && m_regions[north_east->m_region].m_sea) {
                    here->m_coast = 1;
                    north_east->m_sea_coast = 1;
                    m_regions[here->m_region].m_coast.add_unique(CoastTile(here_index));
                } else {
                    here->m_region_border = 1;
                    north_east->m_region_border = 1;
                }
            }
            if (row < rows - 1) {
                if (here->m_region != south->m_region) {
                    RegionNeighbour* to_south = m_regions[here->m_region].neighbour(south->m_region);
                    to_south->add_border_tile(here_index);
                    to_south = m_regions[south->m_region].neighbour(here->m_region);
                    to_south->add_border_tile(south_index);
                    if (m_regions[here->m_region].m_sea && !m_regions[south->m_region].m_sea) {
                        south->m_coast = 1;
                        here->m_sea_coast = 1;
                    } else if (!m_regions[here->m_region].m_sea && m_regions[south->m_region].m_sea) {
                        here->m_coast = 1;
                        south->m_sea_coast = 1;
                        m_regions[here->m_region].m_coast.add_unique(CoastTile(here_index));
                    } else {
                        here->m_region_border = 1;
                        south->m_region_border = 1;
                    }
                }
                if (here->m_region != south_east->m_region) {
                    RegionNeighbour* to_south_east = m_regions[here->m_region].neighbour(south_east->m_region);
                    to_south_east->add_border_tile(here_index);
                    to_south_east = m_regions[south_east->m_region].neighbour(here->m_region);
                    to_south_east->add_border_tile(south_east_index);
                    if (m_regions[here->m_region].m_sea && !m_regions[south_east->m_region].m_sea) {
                        south_east->m_coast = 1;
                        here->m_sea_coast = 1;
                    } else if (!m_regions[here->m_region].m_sea && m_regions[south_east->m_region].m_sea) {
                        here->m_coast = 1;
                        south_east->m_sea_coast = 1;
                        m_regions[here->m_region].m_coast.add_unique(CoastTile(here_index));
                    } else {
                        here->m_region_border = 1;
                        south_east->m_region_border = 1;
                    }
                }
            }
            Tile* west = 0;
            if (x > 0)
                west = tile(x - 1, y);
            Tile* north = 0;
            if (row > 0)
                north = tile(x, y + 1);
            Tile* north_west = 0;
            if (x > 0 && row > 0)
                north_west = tile(x - 1, y + 1);
        }
    }
    unsigned int region = 0;
    for (region = 0; region < m_region_list.m_count; region++) {
        if (!m_regions[region].is_sea()) {
            m_regions[region].m_settlement_position == RoadPoint();
            int settlement = lbl_029219E0->unk_004A5231(&m_regions[region].m_settlement_position, 0);
            if (!(m_regions[region].m_port_position == RoadPoint())) {
                int port = lbl_029219E0->unk_004A5879(&m_regions[region].m_port_position);
            }
        }
        unk_0047FC44(region);
    }
    for (region = 0; region < m_region_list.m_count; region++) {
        if (!m_regions[region].is_sea()) {
            m_regions[region].unk_00483CBF();
            m_regions[region].unk_00483D33();
        }
    }
}

// Adds a tile to the border with this neighbour, once.
void RegionNeighbour::add_border_tile(int tile)
{
    for (unsigned int i = 0; i < m_18.size(); i++)
        if (m_18[i] == tile)
            return;
    m_18.push_back(tile);
}

bool StrategyMap::is_sea_region(int region)
{
    return lbl_018CD80C->m_regions[region].is_sea();
}

// Records the road a region planned towards a neighbour, in its entry for that neighbour.
void StrategyMap::add_road(int region, int other, Road* road)
{
    for (unsigned int i = 0; i < m_regions[region].num_neighbours(); i++) {
        if (m_regions[region].m_neighbours[i].m_region == other) {
            m_regions[region].m_neighbours[i].m_road = road;
            break;
        }
    }
}

void StrategyMap::unk_00481FA4(int region)
{
    m_regions[region].m_1ac = true;
}

void StrategyMap::unk_004826A1(TilePosition* position)
{
    Tile* marked = tile(*position);
    marked->m_1c_30 = 1;
    Region* region = unk_00482A48(position);
    int tile_index = position->x + position->y * m_width;
    region->unk_00486079(tile_index);
    unk_009CA004(position->x - 1, position->y - 1, 3, 3, 1);
}

Region* StrategyMap::unk_004829FD(TilePosition* position)
{
    Tile* t = tile(*position);
    return &m_regions[t->m_region];
}

Region* StrategyMap::unk_00482A48(TilePosition* position)
{
    Tile* t = tile(*position);
    return &m_regions[t->m_region];
}

void Region::set_road_level(unsigned char level)
{
    if (level >= 3)
        level = 2;
    m_road_level = level;
}

void Region::raise_road_level()
{
    if (m_road_level < 3)
        m_road_level++;
}

void Region::set_farming_level(unsigned int level)
{
    if (level > 5)
        level = 5;
    m_farming_level = level;
}

void Region::raise_farming_level()
{
    if (m_farming_level < 4)
        m_farming_level++;
}

void Region::set_famine_threat(int threat)
{
    unsigned char num_threats = 5;
    if (threat >= num_threats)
        threat = num_threats - 1;
    m_famine_threat = threat;
}

void Region::raise_famine_threat()
{
    if (m_famine_threat < 4)
        next_famine_threat(m_famine_threat);
}

void Region::unk_00486079(int tile)
{
    // NONMATCHING - the original reserves 12 more bytes of stack
    m_a4.add_unique(tile);
}

int Region::unk_00483F17()
{
    return m_port->unk_184();
}

int Region::unk_00483F36()
{
    return m_port->unk_184();
}

RebelType* Region::rebel_type()
{
    RebelType* type = fn_00903C7E(m_rebel_type);
    return type;
}

bool Region::has_rebel_type()
{
    if (!m_rebel_type.is_set())
        return false;
    RebelType* type = fn_00903C7E(m_rebel_type);
    return type != 0;
}

RegionNeighbour* Region::neighbour(int region)
{
    for (unsigned int i = 0; i < m_neighbours.size(); i++)
        if (m_neighbours[i].m_region == region)
            return &m_neighbours[i];
    RegionNeighbour* entry = m_neighbours.push_back(RegionNeighbour());
    entry->m_region = region;
    entry->m_other = &lbl_018CD80C->m_regions[region];
    return entry;
}

void Region::unk_00483CBF()
{
    for (Array<RegionNeighbour>::iterator it = m_neighbours.data(); it != m_neighbours.end(); ++it)
        it->unk_0048642E();
}

void Region::unk_00483D33()
{
    for (Array<RegionNeighbour>::iterator it = m_neighbours.data(); it != m_neighbours.end(); ++it)
        it->unk_00486561(m_id);
    for (Array<RegionNeighbour>::iterator next = m_neighbours.data(); next != m_neighbours.end(); ++next)
        next->unk_00487915();
}

void RegionNeighbour::unk_0048642E()
{
    // NONMATCHING - stack slot order differs
    for (Array<int>::iterator it = m_18.data(); it != m_18.end(); ++it) {
        int tile = *it;
        bool blocked = lbl_029219E0->unk_004A605B(RoadPoint(tile));
        bool all_blocked = true;
        if (!blocked) {
            int neighbours[8];
            unsigned int count = lbl_018CD80C->unk_00481334(tile, neighbours);
            for (unsigned int i = 0; i < count; i++) {
                if (lbl_018CD80C->m_tiles[neighbours[i]].m_region == m_region &&
                    !lbl_029219E0->unk_004A605B(RoadPoint(neighbours[i]))) {
                    all_blocked = false;
                    break;
                }
            }
        }
        if (!blocked && !all_blocked)
            m_48.push_back(tile);
    }
}

void RegionNeighbour::unk_00487915()
{
    // NONMATCHING - the original reserves 12 more bytes of stack
    for (unsigned int i = 0; i < m_24.size(); i++) {
        int neighbours[8];
        unsigned int count = lbl_018CD80C->unk_00481334(m_24[i], neighbours);
        for (unsigned int j = 0; j < count; j++) {
            RoadPoint position(neighbours[j]);
            if (lbl_018CD80C->unk_00471495(position))
                m_30.add_unique(neighbours[j]);
        }
    }
}

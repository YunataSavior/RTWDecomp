// The campaign map's A* search.
#include "unsorted.h"
#include "faction.h"
#include "strategy_map.h"
#include "strategy_map_pathfinding.h"
#include "world_map.h"
#include "character.h"
#include "heap.h"

inline RoadPoint operator+(const RoadPoint& point, const TileStep& step)
{
    RoadPoint result;
    result.x = (short)(point.x + step.x);
    result.y = (short)(point.y + step.y);
    return result;
}

// About the distance between two tiles: a square root from halving the exponent of the float's bits.
inline float fast_sqrt(unsigned int value)
{
    float f = (float)value;
    int bits = *(int*)&f;
    bits = ((bits - 0x3f800000) >> 1) + 0x3f800000;
    return *(float*)&bits;
}

inline unsigned int distance_squared(const RoadPoint& a, const RoadPoint& b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

void fill_directions()
{
    // NONMATCHING - zeroes are stored with and word ptr, 0 instead of xor ax, ax then mov
    lbl_012D08A8[0].x = 0;
    lbl_012D08A8[0].y = 1;
    lbl_012D08A8[1].x = 1;
    lbl_012D08A8[1].y = 1;
    lbl_012D08A8[2].x = 1;
    lbl_012D08A8[2].y = 0;
    lbl_012D08A8[3].x = 1;
    lbl_012D08A8[3].y = -1;
    lbl_012D08A8[4].x = 0;
    lbl_012D08A8[4].y = -1;
    lbl_012D08A8[5].x = -1;
    lbl_012D08A8[5].y = -1;
    lbl_012D08A8[6].x = -1;
    lbl_012D08A8[6].y = 0;
    lbl_012D08A8[7].x = -1;
    lbl_012D08A8[7].y = 1;
}

// Finds the cheapest path from one tile to another, and returns the goal's node (follow m_parent back to the start),
// or 0 when there is none within the search's limit. In mode 2 (roads) and 5 it enters only tiles of the start's and
// the goal's regions; orthogonal limits it to the 4 orthogonal directions.
PathNode* AStar::search(int mode, const RoadPoint& from, const RoadPoint& to, bool orthogonal)
{
    // NONMATCHING - stack slot order, evaluation order and inline temporaries differ
    reset();
    unsigned int limit;
    switch (mode) {
    case 0:
    case 1:
    case 2:
    case 3:
        limit = 1700;
        break;
    default:
        limit = 3400;
        break;
    }
    lbl_012D08A4 = mode;
    m_orthogonal = orthogonal;
    m_start_region = lbl_018CD80C->tile(from)->m_region;
    m_goal_region = lbl_018CD80C->tile(to)->m_region;
    lbl_012D08DC = lbl_012D08C8.alloc();
    PathNode* start = lbl_012D0890.alloc();
    start->m_g = 0.0f;
    start->m_f = start->m_g + start->m_h;
    start->m_h = lbl_00FE6154 * fast_sqrt(distance_squared(from, to));
    start->m_point = from;
    start->m_index = from.x + from.y * lbl_018CD80C->width();
    unsigned int expanded = 0;
    lbl_012D08E0.clear();
    lbl_012D08E0.push(start);
    for (;;) {
        PathNode* node = pop();
        if (!node)
            return 0;
        if (node->m_point == to)
            return node;
        expand(node, to);
        expanded++;
        if (expanded > limit)
            return 0;
    }
}

// Takes the cheapest node off the open list and closes it.
PathNode* AStar::pop()
{
    // NONMATCHING - stack slot order differs
    if (lbl_012D08E0.empty())
        return 0;
    PathNode* node = lbl_012D08E0.top();
    lbl_012D08E0.pop();
    m_open.remove(node->m_index);
    m_closed.insert(node->m_index, node);
    return node;
}

// Tries each step out of a node's tile that the mode allows.
void AStar::expand(PathNode* node, const RoadPoint& goal)
{
    // NONMATCHING - evaluation order and inline temporaries differ, and the cast to CharacterKind adds a null check
    RoadPoint next;
    const RoadPoint& point = node->m_point;
    unsigned int width = lbl_018CD80C->width();
    unsigned int height = lbl_018CD80C->height();
    for (int i = 0; i < 8; i += m_orthogonal + 1) {
        next = point + lbl_012D08A8[i];
        if (next.x < width && next.y < height) {
            switch (lbl_012D08A4) {
            case 0:
            case 1:
                if (lbl_029219E0->can_move(point, next, goal) == 1)
                    relax(node, next, goal);
                break;
            case 2:
            case 5: {
                int region = lbl_018CD80C->m_tiles[next.x + next.y * lbl_018CD80C->m_width].m_region;
                if (region == m_start_region || region == m_goal_region) {
                    if (lbl_029219E0->can_move(point, next, goal) == 1)
                        relax(node, next, goal);
                }
                break;
            }
            case 3: {
                int region = lbl_018CD80C->m_tiles[next.x + next.y * lbl_018CD80C->m_width].m_region;
                Faction* owner = lbl_018CD80C->m_regions[region].m_owner;
                if (lbl_029219E0->character()->unk_0058B8D9(owner)
                    || ((CharacterKind*)lbl_029219E0->character())->faction()->at_war_with(owner)) {
                    if (lbl_029219E0->can_move(point, next, goal) == 1)
                        relax(node, next, goal);
                }
                break;
            }
            }
        }
    }
}

// Reaches a tile from a node: a cheaper way to a node already open or closed, or a new open node.
void AStar::relax(PathNode* parent, const RoadPoint& point, const RoadPoint& goal)
{
    // NONMATCHING - stack slot order, evaluation order and inline temporaries differ
    float g = lbl_029219E0->step_cost(parent->m_point, point) + parent->m_g;
    int i = 0;
    int step = (m_orthogonal != 0) + 1;
    int index = point.x + point.y * lbl_018CD80C->width();
    PathNode* node = 0;
    PathNode** found = m_open.find(index);
    node = found ? *found : 0;
    if (node) {
        for (i = 0; i < 8; i += step)
            if (!parent->m_children[i])
                break;
        parent->m_children[i] = node;
        if (g < node->m_g) {
            node->m_parent = parent;
            node->m_g = g;
            node->m_f = g + node->m_h;
        }
        return;
    }
    found = m_closed.find(index);
    node = found ? *found : 0;
    if (node) {
        for (i = 0; i < 8; i += step)
            if (!parent->m_children[i])
                break;
        parent->m_children[i] = node;
        if (g < node->m_g) {
            node->m_parent = parent;
            node->m_g = g;
            node->m_f = g + node->m_h;
            propagate(node);
        }
        return;
    }
    PathNode* added = lbl_012D0890.alloc();
    added->m_parent = parent;
    added->m_g = g;
    added->m_h = lbl_00FE6154 * fast_sqrt(distance_squared(goal, point));
    added->m_f = g + added->m_h;
    added->m_index = index;
    added->m_point = point;
    PathNode* open = added;
    lbl_012D08E0.push(open);
    m_open.insert(open->m_index, open);
    for (i = 0; i < 8; i += step)
        if (!parent->m_children[i])
            break;
    parent->m_children[i] = added;
}

// A cheaper way to a closed node: updates the costs of the nodes reached through it, and of theirs, and so on.
void AStar::propagate(PathNode* node)
{
    PathNode* parent;
    PathNode* child;
    int i = 0;
    float g_value = 0.0f, delta = 0.0f;
    g_value = node->m_g;
    for (i = 0; i < 8; i += m_orthogonal + 1) {
        child = node->m_children[i];
        if (!child)
            break;
        delta = lbl_029219E0->step_cost(node->m_point, child->m_point);
        if (g_value + delta < child->m_g) {
            child->m_g = g_value + delta;
            child->m_f = child->m_g + child->m_h;
            child->m_parent = node;
            push_stack(child);
        }
    }
    while (lbl_012D08DC->m_next) {
        parent = pop_stack();
        for (i = 0; i < 8; i += m_orthogonal + 1) {
            child = parent->m_children[i];
            if (!child)
                break;
            delta = lbl_029219E0->step_cost(parent->m_point, child->m_point);
            if (delta + parent->m_g < child->m_g) {
                child->m_g = delta + parent->m_g;
                child->m_f = child->m_g + child->m_h;
                child->m_parent = parent;
                push_stack(child);
            }
        }
    }
}

// Stops in the debugger when the stack's head links to itself, directly or through one entry.
#define CHECK_PATH_STACK()                                                                                      if (lbl_012D08DC->m_next                                                                                        && (lbl_012D08DC->m_next == lbl_012D08DC || lbl_012D08DC->m_next->m_next == lbl_012D08DC)                   && fn_00F3F290())                                                                                           __asm int 3

void AStar::push_stack(PathNode* node)
{
    CHECK_PATH_STACK();
    PathNodeStackEntry* entry = lbl_012D08C8.alloc();
    entry->m_node = node;
    entry->m_next = lbl_012D08DC->m_next;
    lbl_012D08DC->m_next = entry;
    CHECK_PATH_STACK();
}

PathNode* AStar::pop_stack()
{
    CHECK_PATH_STACK();
    PathNodeStackEntry* entry = lbl_012D08DC->m_next;
    PathNode* popped = entry->m_node;
    lbl_012D08DC->m_next = entry->m_next;
    lbl_012D08C8.free(entry);
    CHECK_PATH_STACK();
    return popped;
}

void PathNodeHeap::push(PathNode* const& node)
{
    // NONMATCHING - the original expands the sift-up loop, where VC6 calls it here
    m_items.push_back(node);
    push_heap(m_items.m_data, m_items.m_data + m_items.m_size, m_greater);
}

void PathNodeHeap::pop()
{
    // NONMATCHING - the original stores the heap's start before its end and predicate
    pop_heap(m_items.m_data, m_items.m_data + m_items.m_size, m_greater);
    m_items.pop_back();
}

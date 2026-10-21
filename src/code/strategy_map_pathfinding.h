// The campaign map's A* search (code\strategy_map_pathfinding.cpp).
#ifndef STRATEGY_MAP_PATHFINDING_H
#define STRATEGY_MAP_PATHFINDING_H

#include "unsorted.h"
#include "strategy_map.h"

// A node of the A* search.
struct PathNode {
    float m_f;                          // +0x00: m_g + m_h
    float m_h;                          // +0x04: the estimate of the cost left
    float m_g;                          // +0x08: the cost from the start
    int m_index;                        // +0x0c: its tile's index in the map's grid
    RoadPoint m_point;                  // +0x10
    PathNode* m_parent;                 // +0x18
    PathNode* m_children[8];            // +0x1c
};

// The search's nodes by tile index (not identified).
class PathNodeTable {
public:
    void insert(int index, PathNode* node);  // 0x004B81B4
    PathNode** find(int index);         // 0x004B81CD
    void remove(int index);             // 0x004B81FE

    void* m_0;
    int m_4;
};

// Orders the open list's heap: the node with the higher f is lower.
struct PathNodeGreater {
    bool operator()(PathNode* a, PathNode* b) const { return a->m_f > b->m_f; }
};

// The open list: a binary heap of nodes, cheapest first.
class PathNodeHeap {
public:
    bool empty() const { return m_items.m_size == 0; }
    PathNode* top() const { return m_items.m_data[0]; }
    void clear() { m_items.m_size = 0; }
    void push(PathNode* const& node);   // 0x004BA44B
    void pop();                         // 0x004BA51D

    Array<PathNode*> m_items;
    PathNodeGreater m_greater;          // +0x0c
};

class PathNodePool {
public:
    PathNode* alloc();                  // 0x004BA1F5
};

// An entry of propagate's stack of nodes to update.
struct PathNodeStackEntry {
    PathNode* m_node;
    PathNodeStackEntry* m_next;
    PathNodeStackEntry* m_free;         // +0x08: the next free entry, while in the pool's free list
};

// A pool of stack entries, allocated 64 at a time (romans_utils.h).
class PathNodeStackPool {
public:
    PathNodeStackEntry* alloc();        // 0x004BA34B
    void free(PathNodeStackEntry* entry)
    {
        entry->m_free = m_free;
        m_free = entry;
        m_count--;
    }

    void* m_blocks;
    int m_num_blocks;                   // +0x04
    PathNodeStackEntry* m_free;         // +0x08
    int m_count;                        // +0x0c: entries in use
};

// A step to a neighbouring tile.
struct TileStep {
    short x;
    short y;
};

// The campaign map's A* search (code\strategy_map_pathfinding.cpp).
class AStar {
public:
    PathNode* search(int mode, const RoadPoint& from, const RoadPoint& to, bool diagonal);  // 0x004B84E5
    void reset();                       // 0x004B83E1
    PathNode* pop();                    // 0x004B8B27
    void expand(PathNode* node, const RoadPoint& goal);  // 0x004B8B85
    void relax(PathNode* parent, const RoadPoint& point, const RoadPoint& goal);  // 0x004B8DE0
    void propagate(PathNode* node);     // 0x004B9092
    void push_stack(PathNode* node);    // 0x004B920B
    PathNode* pop_stack();              // 0x004B92C2

    bool m_orthogonal;                  // +0x00: only the 4 orthogonal directions
    PathNodeTable m_open;               // +0x04
    PathNodeTable m_closed;             // +0x0c
    int m_start_region;                 // +0x14
    int m_goal_region;                  // +0x18
};

extern "C" {
extern AStar* lbl_012D07C0;
extern int lbl_012D08A4;            // the A* search's mode (2 roads, 5 region to region)
extern TileStep lbl_012D08A8[8];    // the 8 directions, clockwise from north
extern PathNodeHeap lbl_012D08E0;
extern PathNodePool lbl_012D0890;
extern PathNodeStackPool lbl_012D08C8;
extern PathNodeStackEntry* lbl_012D08DC;  // the head of propagate's stack
extern float lbl_012D05F0;          // the cost factor of a diagonal step
extern const float lbl_010D01A0[14];  // the cost of each ground type
extern const unsigned char lbl_010D0A00[9];  // the edge-mask bit of each direction, by (dy+1)*3 + dx+1
extern const float lbl_00FE60A0;    // 1.0
extern const float lbl_00FE60A4;    // 2.0
extern const float lbl_00FE60A8;    // 4.0
extern const float lbl_00FE60AC;    // 7.0
extern const float lbl_00FE609C;    // 0.4: the least a step costs
extern const float lbl_00FE60C0[8]; // the cost factor of each road level
extern const float lbl_00FE6154;    // 0.5
extern int lbl_012D0604;           // the movement rules in force (3 while planning roads)
}

#endif

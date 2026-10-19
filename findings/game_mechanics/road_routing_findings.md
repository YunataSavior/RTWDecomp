# How roads are routed on the campaign map

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the matching source in [`src/`](../../src/), or from in-game tests.

**Short answer:** roads aren't stored in `map.rwm`; the game plans them while a campaign runs. When a region first gets roads, it plans one road to each neighbouring region's settlement with an A* search, plus one to its port. The **region that plans a road first fixes its whole route for good**. If the other region has no roads yet, only the planner's part is drawn, and the rest waits, already planned, until the other region builds roads. Tiles that already carry a drawn road are cheap to cross, so later roads join earlier ones, and **the routes depend on the order in which regions built their roads**.

Sources:

- The decompiled [`src/code/road.cpp`](../../src/code/road.cpp) (the `Road` and `RoadManager` classes) and the road planning in [`src/code/strategy_map.cpp`](../../src/code/strategy_map.cpp) (`PathFinder::find_road_path`, `plan_roads` and `build_roads`), all matching the original.
- The decompiled A* search in [`src/code/strategy_map_pathfinding.cpp`](../../src/code/strategy_map_pathfinding.cpp), the step cost and terrain check in `strategy_map.cpp`, and the river check, neighbour scan and saved road state in [`src/code/world_map.cpp`](../../src/code/world_map.cpp).
  - Some of these don't match the original byte for byte, but they're functionally equivalent; each starts with a `NONMATCHING` comment.
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the general movement check and the callers, which aren't decompiled yet (NYD).

Addresses are virtual addresses in that exe (image base `0x400000`). Tile coordinates are the map's: `x` from the west, `y` from the **south**.

## When a region plans its roads

`PathFinder::build_roads(region, level)` (`0x4B21E2`):

- When the region has **no roads yet** (`Region::m_road_level`, `+0x20`, is 0), it calls `plan_roads(region, level)`, which plans all of the region's roads.
- Otherwise it **plans nothing**: it raises the region's road level by one (`Region::raise_road_level`, `0x483E6B`, up to 3), then raises every road tile in the region to it (roads → paved roads → highways).
  - So upgrading roads never re-plans them, and never retries a neighbour that has no road.
  - The `level` it's given is ignored here: each call on a region that has roads moves it up one level.
- Either way, it then updates the regions that the region's roads connect to, when the region has a port.

Callers (NYD):

1. `0x5B6C52` (in `settlement.cpp`) calls `build_roads(region, n + 1)` when the settlement's buildings give road level `n` (`0x5D48D0` with capability 9) and `n + 1` is above the region's road level.
   - Starting roads (`hinterland_roads` in `descr_strat.txt`) probably reach the game this way as the file loads, in file order (not checked).
2. `0x447C1B` reads a `road_level` token (alongside `farming_level` and `famine_threat`) and calls `build_roads` with it when it's below 5. Which file this parser reads wasn't identified.
3. `0xDAD07C` calls `plan_roads(region, 1)` **directly**, for the region at a tile position held by its object (`+0x18`). It's the only way to plan a region's roads a second time. What it belongs to wasn't identified.

## Planning one region's roads

`PathFinder::plan_roads(R, level)` (`0x4B1B5B`) sets R's road level first (`Region::set_road_level`, `0x483E16`, which turns 3 or more into 2; a `level` of 0 raises it by one instead), so a region's first roads are never highways. It then goes through R's neighbours in the order of R's neighbour list (`Region::m_neighbours`, `+0x164`, `0x54`-byte entries; see [Neighbour order](#neighbour-order-nyd)). A neighbour is skipped when it's a sea region (`StrategyMap::is_sea_region`, `Region::m_sea` at `+0x50`). For each other neighbour N:

1. **Does N already have a road towards R?** R looks through N's neighbour entries for a road whose other end (`Road::m_other`, `+0x1c`) is R, that is, one N planned.
   - If its pending list isn't empty, R calls `RoadManager::upgrade`, which moves the pending points into the drawn list and marks them on the map. **Nothing is re-planned.**
   - If nothing is pending, R plans from the road's last drawn point to R's own settlement, drops the first point (already drawn), and adds the rest to the road. This happens only when N planned the road while R already had roads and R plans again, which only the direct caller `0xDAD07C` can cause.
2. **Otherwise R plans a new road** from its settlement to N's settlement (`find_road_path`, below). The path comes back goal first, so it starts in N:
   - The run of tiles at N's end that lie in N becomes the **pending** part, but only while N has no roads yet (`N.m_road_level == 0`).
     - The index of the last of them is the road's border index; it's −1, and the whole road is drawn, when N already has roads.
   - The first tile past that run must lie in R (or in a sea region); otherwise the road is refused.
     - The search can't enter a third region anyway (see below), so this is only a backstop.
   - When the search fails, nothing is recorded. The pair gets another chance only when N plans its own roads; R's roads are then in place, so N draws the whole road at once.
3. **After the neighbours**, R plans a road from its settlement (`Region::m_settlement_position`, `+0x194`) to its port position (`+0x19c`).
   - It does this when R has a port object (`+0x148`) whose `+0x184` is non-zero, or when both positions are set (not `(0, 0)`).
   - When the path is found, `RoadManager::create_port_road` lays it, all drawn, with R as both ends. A port road that fails isn't tried again.

A road's points are stored in two lists:

- `RoadManager::create(R, N, border)` (`0x4A224F`) makes the road with R as planner (`+0x18`) and N as the other end (`+0x1c`).
  - It sets the split flag (`+0x24`) when the border index isn't −1.
- `Road::set_path` walks the path from R's end: the points up to the border index go to the pending list (`+0x0c`), and the rest go to the drawn list (`+0x00`).
  - Only the N tiles at N's end are pending, so N tiles the route passes through earlier, if it weaves across the border, are drawn at once.
- `RoadManager::register_tiles` (`0x4A2630`) marks only the **drawn** points.
  - On each tile it sets the road level (`Tile+0x1c` bits 12–14) to the tile's region's road level and points `Tile::m_road` (`+0x04`) at the road.
  - It skips tiles whose level is already 4.
- `StrategyMap::add_road(a, b, road)` (`0x4812B2`) stores the road in region a's neighbour entry for b (`+0x10`).
  - `create` stores it in the planner's entry; when the road is finished, `upgrade` stores it in the other region's entry too.

So **a road's route is fixed by whichever of its two regions plans first**. When the other region builds roads later, its half appears along the route that was already planned.

## The path search

`find_road_path(from, to)` (`0x4AE64B`) is decompiled:

- It sets the movement mode `[0x12D0604]` to 3 (road building).
- It refuses a goal that's off the map or blocked (`PathFinder::unk_004A605B`), with status 1.
- Otherwise it runs `AStar::search(2, from, to, true)` (`0x4B84E5`) and copies the path, goal first, into `PathFinder::m_path` (`+0xbc`).
  - The status (`+0x14`) is 2 when there's no path.

The search (`AStar`, `0x4B84E5`) works like this:

1. **Region limit:** in mode 2 a tile can be entered only when its region is the start tile's or the goal tile's (`0x4B8B85`).
   - For a port road both are the region itself.
2. **Moves:** the last argument (`true`) makes the search step through its 8-direction table (`0x12D08A8`, set by `0x4B821F`: (0,1), (1,1), (1,0), (1,−1), (0,−1), …) two at a time.
   - So roads move in **4 directions only**, tried in the order north, east, south, west.
3. **Bounds:** off-map tiles are skipped.
4. **Blocked steps:** a step must pass the general movement check `PathFinder::can_move` (`0x4B52BA`), which calls the terrain check `PathFinder::is_blocked` (`0x4A6862`) on the tile being entered:
   - **Edge mask:** the step is blocked when the tile's edge mask (`+0x25`) is `0xFF`, or has the bit for the direction it's entered from.
     - The bits come from the table at `0x10D0A00` (`[128, 64, 32, 16, 0, 8, 4, 2, 1]`, indexed by `(dy+1)*3 + (dx+1)`).
     - The map loader sets the mask to `0xFF` on every tile of ground 4, 5, 7 or 10 (high and low mountains, dense forest and ocean; at `0x475252`; NYD).
   - **Sea:** ground 10–12 is blocked.
   - **Rivers:** a river or river-source tile (`Tile+0x1c` bit 6 or bit 5, `Tile::is_river`) can be entered in road mode only when `StrategyMap::can_cross_river` (`0x47C6B8`) allows it:
     - A step along x is allowed into a bridge of model `0x6E` or `0x6F` (`Tile+0x0c`), unless a tile above or below the bridge (y ± 1) already has a road level.
     - A step along y is allowed into a bridge of model `0x6C` or `0x6D`, unless a tile beside it (x ± 1) already has a road level.
     - A tile with no model (`0xFFFF`) is allowed, except when it has `Tile+0x1c` bits 15–16 set (not identified) and so does one of the two tiles beside it along the step's axis.
     - Any other model refuses the step, and so does a diagonal step.
   - **Objects on tiles:** road planning has no character, so `can_move` runs with kind 6 and no faction (`PathFinder::set_character(0)`). It still checks what stands on the tiles:
     - A step is refused when the tile it leaves holds an object whose byte `+0x84` is set, unless the step enters the goal: `PathFinder::unk_004A74BA` returns the first non-zero virtual `+0x18` of the tile's objects (which objects give one is NYD).
     - **Characters don't block road planning.** `PathFinder::unk_004A95F3` returns the first character on the tile, but nothing when it's given no faction, and road planning has none.
       - For a moving character, it returns the first character on the tile and flags it when it's another faction's; which of those `0x4AC9E5` drops is NYD.
     - Near the goal (`RoadPoint::unk_004B61C2`, NYD), a step onto a tile with a landmark or bits 0–1 of `Tile+0x24` set needs `PathFinder::unk_004AA85D`'s approval when the goal tile has any of bits 0, 1, 2, 5 or 6 of `Tile+0x24` set.
       - It approves when the tile's `+0x25` isn't `0xFF` and bits 0, 1, 3, 5 and 6 of its `+0x24` are clear.
     - So an army standing on a tile when a road is planned doesn't move the route.
5. **Step cost** (`PathFinder::step_cost`, `0x4B634D`), for entering tile T, as float32:
   ```
   c = ground_cost[T.ground]                      // 0x10D01A0, below
   c *= T.movement_factor                          // T+0x30, from map.rwm (0.8, 0.9 or 1.0)
   if (T+0x1c bits 21-23 != 0) c *= 2.0            // not identified; set from saved tile data
   if (T has a drawn road, T+0x04 != 0) c *= 0.5   // index 3 of the table at 0xFE60C0
   else if (T is a river tile, bit 6)   c *= 7.0
   else                                 c *= 4.0
   c = max(c, 0.4)
   ```
   - A diagonal step would also be multiplied by a factor (`[0x12D05F0]`), but roads take none.
   - Outside road mode, the last three lines are a multiplication by a road-level table (`0xFE60C0`: 1.0, 0.75, 0.6, 0.5, 0.5, 1.41) indexed by T's road level, which is how roads speed up armies.
   - Ground costs (`0x10D01A0`), by ground code: 0 sparse forest 10, 1 fertile high 11, 2 fertile low 11, 3 wilderness 12, 4 high mountains 999, 5 low mountains 15, 6 hills 14, 7 dense forest 999, 8 fertile medium 13, 9 swamp 20, 10 ocean 999, 11 deep sea 8, 12 shallow sea 4.5, 13 beach 14.
   - **Only drawn tiles are cheap.** Pending halves don't set `Tile::m_road`, so they never attract other roads.
6. **Heuristic:** `h = 0.5 × fastsqrt(dx² + dy²)`, where `fastsqrt` halves the float's exponent bits (`((bits − 0x3F800000) >> 1) + 0x3F800000`).
7. **Open list:** a binary heap ordered by `f = g + h` (`PathNodeHeap`, built on the standard library's heap functions). A node is checked against the goal when it's taken off the heap.
   - When a node already on the open list is reached more cheaply, its parent, `g` and `f` change **in place, without re-sorting the heap**, so the heap order can be slightly off.
   - A closed node reached more cheaply is updated, and so are its children, their children and so on, wherever the new cost is lower (`AStar::propagate`, `0x4B9092`).
     - In road mode the cheapest possible step (10 × 0.8 × 0.5 = 4) is far above the most `h` can change in one step (about 0.5), so this shouldn't happen.
8. **Give up:** after more than **1,700** expansions in modes 0–3, and 3,400 in other modes.
   - So a long road can fail from one end and succeed from the other, since the two searches expand different nodes.

## Neighbour order

`StrategyMap::build_neighbours` (`0x47F003`) builds every region's neighbour list as the map loads:

- It scans rows from the north (`y = H−1` down to 0), and in each row `x = 0 … W−2`.
- It compares each tile with the tiles to its east `(x+1, y)`, north-east `(x+1, y+1)`, south `(x, y−1)` and south-east `(x+1, y−1)`, in that order.
  - The top row skips the north-east tile, and the bottom row the south and south-east ones.
- When the regions differ, `Region::neighbour` (`0x483C05`) finds or adds each region in the other's list, the tile's region's list first, and records the two tiles as border tiles of that pair.
- It also marks border tiles (`Tile+0x20` bit 8 between land regions, bit 9 on a land tile beside the sea, bit 10 on a sea tile beside land), and lists each land region's coastal tiles.
- So a region's neighbours are in the order this scan first finds them, and **regions that touch only at a corner are neighbours too**. Their road search can't cross a corner, though, so they can never get a road.

## Saves

Saves keep the roads, so loading a game never re-plans them:

- `RoadManager::save`/`load` write and read a `u16` count, then each road (the `ROAD_MANAGER` version is 1).
- Per road, `Road::save` writes:
  - the road's own address (used to fix up pointers to it after loading);
  - a marker (`0x41391C`);
  - the planner, the other region, the traffic counter (`+0x20`) and the ID (`+0x28`);
  - the drawn points, then the pending points, each as a count followed by 8-byte (x, y) points;
  - the split flag, and another marker.
- The `ROAD` version (2) is read from the save's version table, not per road; a version-1 save has no pending points.
- Each tile's road state is saved separately. `StrategyMap::load_tile_road` (`0x47D489`) reads 12-byte records:
  - the road's saved address goes to `Tile+0x04`;
  - a `u16` of bits goes to the tile's flags: bits 0–2 → `Tile+0x1c` bits 12–14 (road level); 3–4 → bits 15–16; 5 → bits 18–20; 6 → bits 21–23; 7 → bits 24–26; 8 → bits 27–29; 9 → bit 30; 10–12 → `Tile+0x20` bits 0–2; 13 → `Tile+0x20` bit 3;
  - then the tile's x and y as `u16`s.

## The road object

`Road` (0x30 bytes, declared in [`road.h`](../../src/code/road.h)):

| Offset | Member | Meaning |
|---|---|---|
| `+0x00` | `m_drawn` | Points shown on the map, in order from the planner's end |
| `+0x0c` | `m_pending` | Points in the other region, shown once it builds roads |
| `+0x18` | `m_planner` | The region that planned the road |
| `+0x1c` | `m_other` | The region at the other end (the planner again for a port road) |
| `+0x20` | `m_traffic` | A counter `add_traffic` adds to (at least 1); the aerial map fades the road by it |
| `+0x24` | `m_split` | Set while part of the route is pending |
| `+0x28` | `m_id` | From `RoadManager::free_id()` |
| `+0x2c` | `m_display` | The aerial map's handle for it, or −1 |

`RoadManager` is the campaign's (`Campaign+0xcd4c`) list of roads.

## Consequences

1. **Build order changes routes.** Drawn road tiles cost ×0.5 instead of ×4, so a road planned after others tends to join them. A region's own roads are planned one after another in its neighbour order, so its later roads ride its earlier ones, and on roads it has just finished from neighbours that built before it.
2. **The planner's map, not the final map, decides a route.** Roads drawn later don't move it. Usually only roads on the planner's side pull it: the other region has drawn roads of its own only when it built earlier and its own search towards the planner failed.
3. **Starting roads come first.** The regions that start with roads plan theirs as the campaign loads (NYD), so their routes don't depend on what the players build.
4. **A failed pair is retried once.** When R's search fails, N searches from its own end when it builds; upgrades don't retry, and port roads aren't retried at all.

## In-game checks

- With only Arsakia's roads built, the roads to Hatra, Seleucia and Susa (regions that start with roads) appeared in full. The road to Phraaspa, which has no roads, stopped at the Media border, as the pending part predicts.
- With only Vicus Venedae's roads built, every road out of it stopped at its region's border.
- Routes simulated with the rules above matched in-game routes around Tanais, Arsakia, Pripet and Mazaka. Where a route depends on build order, it matched only for some orders: for example, Mazaka's single road east matched only when Artaxarta built before Mazaka.

## Other notes

- `plan_roads` ends by calling `StrategyMap::unk_0047FC44` for R, then once for each **index** `0 … n−1` of R's neighbour list rather than for each neighbour's ID, which looks like a bug. What `0x47FC44` does wasn't identified.
- `register_tiles` calls `PathFinder::unk_004AC914` on a tile when its ground is `>= 7 || <= 8`, which is always true, so it calls it on every tile it marks. The `||` was probably meant to be `&&`; what `0x4AC914` does wasn't identified.

## Not verified

- Which file the `road_level` token at `0x447C1B` comes from, and what `0xDAD07C` belongs to.
- That starting roads are planned in `descr_strat.txt` order as the file loads.
- What `Tile+0x1c` bits 15–16 and 21–23, and the goal check `unk_004A605B` are.
- Which objects on tiles block road searches through their virtual `+0x18` (`PathFinder::unk_004A74BA`).

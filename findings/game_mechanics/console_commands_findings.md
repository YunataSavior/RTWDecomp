# Console commands: all of them, and which ones work

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the decompiled source in [`src/`](../../src/).

**Short version:** the exe has **96** console commands, but the game's console (the ~ key) runs only **21** of them: 19 on the campaign map and 3 in battle (`show_cursorstat` works in both). The other 75 are developer commands. The console refuses them with "command not recognised", even though they're compiled in. A campaign script's `console_command` doesn't have that restriction, so it can probably run every campaign command (NYD). Two commands can never run at all, and two more do nothing.

Sources:

- The decompiled console, [`src/code/env/dev_console.cpp`](../../src/code/env/dev_console.cpp).
  - Every command's registration (its name, flags and help text) matches the original byte for byte, as do the console's constructor and destructor.
  - The code that runs a line (`Console::run`), the help and the listing work the same as the original but aren't byte-identical.
  - The bodies of `give_trait`, `give_trait_points`, `force_battle_victory`, `force_battle_defeat` and most of the one-line commands (the toggles, `bestbuy`, `fire`, the `amdb_` ones and those that only call one function) are decompiled too.
- The word match, in [`src/code/romans_utils.cpp`](../../src/code/romans_utils.cpp).
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the callers and for what the other commands do (NYD).

Addresses are virtual addresses in that exe (image base `0x400000`).

## How a line is run

`Console::run` (`0xDE1718`) takes the typed line.

- A line starting with `?` asks for help: `?` alone lists the commands, and `?name` shows a command's help text.
  - While the console is restricted (see below), `?` alone gives no listing.
- Any other line is kept in a history of the last 10 lines, and its first word is matched against each command's name.
  - The match is case-sensitive, so `Add_money` isn't found (`0xE2DDF0`).
  - A name ends at a space, a tab, a comma, a colon, a semicolon or the end of the line.
- A command is run only when all of these hold:
  1. Its flags include the console's current mode: battle (`0x1`), campaign (`0x2`), battle editor (`0x4`) or campaign editor (`0x8`).
  2. In multiplayer (a guess for the flag at `0x18C9D98`), its flags include `0x20`.
  3. While the console is restricted, its flags include `0x10`.
     - It also needs a non-zero value at `+0x10`, but every command has one (−1, or 1 to 3), so that check never fails.
- When no command qualifies, the reply is "err: command not recognised (use ? for help).", the same as for a misspelt name.
  - Before a battle or the campaign map has set a mode, it's "err: console mode not initialised, no commands available".

Arguments are separated by spaces or commas.

- A name with spaces in it goes in double quotes, such as `give_trait "Flavius Julius" GoodCommander`.
- A word of 128 characters or more fails to read.

The commands that take a character's name look only at characters on the map, the player's faction first, then the other factions in turn order (`find_character`).

## Where the mode and the restriction are set (NYD)

- The battle sets mode 1 (`0x8BE97C`), the campaign map sets mode 2 (`0x8C81EB`) and the battle editor sets mode 4 (`0xDA729C`).
  - Nothing sets mode 8, the campaign editor's.
- The game's console (`0xCE5E06`, in `romans_ui.cpp`) sets the restriction before it runs a line and clears it afterwards.
  - The same function handles `exit` itself, without the console.
- A campaign script's `console_command` (`0xB40842`, in `script_semantics.cpp`) passes the rest of its line to the console with no restriction.
  - So a script can probably run any command that has the mode the game is in, including the developer ones.
- `battlefield_handler.cpp` also runs console lines (`0x8C0115`); what for wasn't checked.

## Which commands work

### In the game's console (the ~ key)

On the campaign map: `add_money`, `add_population`, `auto_win`, `bestbuy`, `character_reset`, `create_unit`, `disable_ai`, `give_ancillary`, `give_trait`, `halt_ai`, `move_character`, `oliphaunt`, `process_cq`, `puppify_my_love`, `run_ai`, `show_cursorstat`, `surrender_regions`, `toggle_fow` and `toggle_terrain`.

In battle: `jericho`, `show_cursorstat` and `toggle_restrictcam`.

Everything else, including `give_trait_points`, `kill_character`, `date`, `control`, `diplomatic_stance`, `create_building`, `capture_settlement` and `list_characters`, is refused there.

In multiplayer, the console accepts only `nw_stats` and `toggle_chat_log` (battle, flag `0x20`), and neither is allowed while restricted, so the game's console probably runs nothing at all there (NYD: the multiplayer flag is a guess).

### Never, or to no effect

- `adjust_sea_bed` and `filter_coastlines` are flagged for the campaign editor only, and nothing sets that mode, so they can never run.
- `reapply_rigid_model_influence` is for the battle editor only.
- `toggle_tow` does nothing: it calls the output window's toggle only when a check that always returns false passes, so that build had the window compiled out.
- `fire` does nothing: its handler is empty, and it ignores its arguments.

## Turning on a developer command

A command is allowed in the game's console when bit 4 (`0x10`) of its flags is set, and its flags are a constant in its registration, so a one-bit patch of the exe turns it on.

- `process_rq` registers with `push 2` (`6A 02`) at `0xDE2A84`, so changing the `02` at file offset `0x9E2A85` to `12` lets the ~ console run it on the campaign map.
  - `process_cq` registers with `push 0x12` (`6A 12`) at `0xDE241E`, the same instruction with the bit set.
  - For any other command, the `push` of its flags comes right after the `push` of its name, before `mov ecx, <the command>` and the call to `ConsoleCommand::ConsoleCommand` (`0xDE1E0D`).
- This patch was tried on the Steam exe, and `process_rq` then worked from the ~ console.
  - Steam's "Verify integrity of game files" would put the original exe back.
- Commands for a mode the game never sets (`adjust_sea_bed` and `filter_coastlines`) would need their mode bit changed as well.

## Other notes

- `bestbuy` switches a percentage at `0x10FDF20` between 100 and 90, which takes 10% off the cost of every unit a settlement can train, after the governor's training percentage, rounding down ([`code/recruitment_item.cpp`](../../src/code/recruitment_item.cpp)).
  - It doesn't change the training turns, and the cost stays at least 1.
- `toggle_restrictcam` does nothing while `toggle_pr` (PR mode) is on, since both use the same flag that `toggle_pr` switches.
- `force_battle_defeat`'s help says it makes the player's alliance win, but it gives the win to the side after the player's (wrapping round to the first), so it does what its name says.
  - Both look the side up from the player's faction and make every other side lose (`Battle::force_victory`, `0x5FD5D1`, NYD in detail).
- `give_trait_points` gives 1 point when the number is left out (decompiled; see [trait_effects_findings.md](trait_effects_findings.md#console-commands)).
- `reapply_rigid_model_influence`'s help text is in writable memory (`0x12CF098`), empty at start-up.

## All commands

Modes are where a command can run at all; "Game's console" says whether the ~ console allows it. The help texts are the game's own, with line breaks removed.

| Command | Modes | Game's console | Its help text |
|---|---|---|---|
| `add_money` | campaign | **yes** | add_money <opt:faction_type> <amount> : adds an amount of money to a faction's coffers, can be negative, default is player faction |
| `add_population` | campaign | **yes** | add_population <settlement_name> <amount> : adds an amount of population to a settlement, can be negative |
| `adjust_sea_bed` | campaign editor | no | adjust_sea_bed : adjusts whole sea bed to specified height |
| `ai_turn_speed` | campaign | no | ai_turn_speed <multiplier> : sets the maximum speed of turn processing during the ai round |
| `amdb_max` | campaign | no | amdb_max : sets aerial map overlay depth bias for max zoom |
| `amdb_min` | campaign | no | amdb_min : sets aerial map overlay depth bias for min zoom |
| `amdb_offset` | campaign | no | amdb_offset : sets aerial map overlay offset towards camera |
| `auto_win` | campaign | **yes** | auto_win <attacker/defender>: the attacker or defender wins the next autoresolved battle |
| `bestbuy` | campaign | **yes** | bestbuy : sells units cheaper |
| `bounds` | battle, campaign | no | bounds : toggle display of bounding objects |
| `building_debug` | battle | no | building_debug: toggles building debug mode. TAB to toggle view modes, RIGHT SHIFT-TAB to reset mode, LEFT SHIFT-TAB to go back modes. G damages mouse-over building. P displays plaza. |
| `burn_piggies_burn` | battle | no | burn_piggies_burn <on/off> : ignite all the piggy winks |
| `capture_settlement` | campaign | no | capture_settlement <settlement name> : evicts any resident characters and armies and gives the settlement to the local player |
| `character_reset` | campaign | **yes** | character_reset : resets the character back to it's start of turn settings |
| `clear_messages` | battle, campaign | no | Clear all the current stacked messages |
| `control` | campaign | no | control <faction_type> : switches player control to specified faction; old faction may not act correctly as ai faction |
| `create_building` | campaign | no | create_building <settlement_name> <building_level_id> : creates a building of the specified type in a settlement; for building level id's see export_descr_building.txt |
| `create_unit` | campaign | **yes** | create_unit <settlement/character_name> <unit_id> <opt:how_many> <opt:exp/armour/weapon>: creates one or more units of the specified type |
| `damage_wall` | campaign | no | damage_wall <settlement> <gate> <breach>: Damage wall of settlement. Forces 40% damage to a random gatehouse and a nearby straight section. Destroys gatehouse if 'gate' parameter present; breaches wall if 'breach' present |
| `date` | campaign | no | date <year> : changes the campaign date to the given year |
| `diplomacy_mission` | campaign | no | diplomacy_mission <ai_faction> <target_faction> <mission_type> <opt:mission_target> : creates a diplomacy mission |
| `diplomatic_stance` | campaign | no | diplomatic_stance <faction_a> <faction_b> <allied/neutral/war>: Set the diplomatic stance between the two factions |
| `disable_ai` | campaign | **yes** | disable_ai [opt: tac \| sub \| dip \| name]: disables all (default) or part of the ai for all factions |
| `event` | campaign | no | event <event_type> <opt:position> : creates an event at position |
| `filter_coastlines` | campaign editor | no | filter_coastlines : applies filter to world map coastlines |
| `fire` | battle | no | fire (mt \| toggle \| reload \| int) |
| `force_battle_defeat` | battle | no | force_battle_defeat : forces the local player's alliance to win the battle, completely destroying the enemy alliance |
| `force_battle_victory` | battle | no | force_battle_victory : forces the local player's alliance to win the battle, completely destroying the enemy alliance |
| `force_diplomacy` | campaign | no | force_diplomacy <accept/decline/off>: Forces the negotiator to accept or decline a proposition |
| `give_ancillary` | campaign | **yes** | give_ancillary <charactername> <ancillary name> : gives the character an ancillary |
| `give_trait` | campaign | **yes** | give_trait <charactername> <trait name> <opt:level>: gives the character a trait at level (default = level 1) |
| `give_trait_points` | campaign | no | give_trait_points <charactername> <trait name> <points>: gives the character points for trait |
| `halt_ai` | campaign | **yes** | halt_ai <opt:factiontypename> : halts the turn sequence just before the start of the specified faction's turn, or the current faction if no faction given |
| `ie` | battle | no | ie <reload> |
| `invulnerable_general` | battle | no | invulnerable_general <character_name> : makes that named general invulnerable in battle |
| `jericho` | battle | **yes** | jericho : and the walls came a-tumblin' down |
| `kill_character` | campaign | no | kill_character <character_name> : kills a character with the given name |
| `kill_faction` | campaign | no | kill_faction <faction_type> : removes the faction from the game |
| `lights` | battle, campaign | no | lights : <merge> <used> <auto> |
| `list_ancillaries` | campaign | no | list_ancillaries : lists all the available ancillaries |
| `list_characters` | campaign | no | list_characters <opt:faction_type> : lists all the characters in the world or those belonging to a faction |
| `list_traits` | campaign | no | list_traits : lists all the available traits |
| `list_units` | campaign | no | list_units <character/settlement name> : lists all of the units in an army, with details. |
| `message_collation_set` | battle, campaign | no | Set the message collation on or off (sets all factions) |
| `move_character` | campaign | **yes** | move_character <name> <x>,<y> : moves named character to position on campaign map |
| `mp` | campaign | no | mp <charactername> <amount>: gives the character movement points |
| `nw_stats` | battle (and in multiplayer) | no | nw_stats: toggles display of network stats. |
| `oliphaunt` | campaign | **yes** | oliphaunt : the biggest around |
| `output_unit_positions` | battle | no | output_unit_positions <filename> : output the positions of all units in the battle to the specified file |
| `perf_times` | battle, campaign | no | Toggle display of simple performance times of game update vs display |
| `process_cq` | campaign | **yes** | process_cq <settlement>: Completes all (possible) construction pending in queue |
| `process_rq` | campaign | no | process_rq <settlement>: Completes all (possible) recruitment pending in queue |
| `puppify_my_love` | campaign | **yes** | ?????? |
| `reapply_rigid_model_influence` | battle editor | no | (none) |
| `regenerate_radar` | battle | no | regenerate_radar: Does what it says on the tin |
| `reload_shaders` | battle, campaign | no | reload_shaders : reloads all vertex shaders |
| `reload_textures` | battle, campaign | no | reload_textures : reloads all textures |
| `reset_display` | battle, campaign, battle editor, campaign editor | no | reset_display: Forces a display_close(); display_open() display reset cycle |
| `run_ai` | campaign | **yes** | run_ai [opt:x]: re-starts an ai turn sequence after disableai or haltai has been triggered; use x to also re-enable all parts of the ai which have been individually disabled |
| `season` | campaign | no | season <season> : changes the campaign season to the given season |
| `set_building_health` | campaign | no | set_building_health <settlement_name> <building_chain> <final health percent>: sets health of a building of the specified type (eg core_building) in a settlement, so that the final health percentage is as specified; for building chains see export_descr_building.txt |
| `set_ranking_interval` | campaign | no | set_ranking_interval : sets the denominator of the faction ranking graph interval which is calculated as (number_of_turns / denominator). If set to 0, then the denominator will be set to number_of_turns, giving an interval of 1 |
| `shadow` | battle | no | shadow <toggle/next/prev> |
| `show_all_messages` | battle, campaign | no | Show all messages to all factions (on/off) |
| `show_battle_circle` | battle | no | show_battle_circle <x> <y> <r> <t> : display a circle at (x, y) of r radius for t seconds |
| `show_battle_line` | battle | no | show_battle_line <x1> <y1> <x2> <y2> <t> : display a line from (x1, y1) to (x2, y2) for t seconds |
| `show_battle_marker` | battle | no | show_battle_marker <x> <y> <t> : display a marker at (x, y) for t seconds |
| `show_battle_paths` | battle | no | show_battle_paths : show all valid processed paths in the pathfinder |
| `show_battle_paths_for_unit` | battle | no | show_battle_paths_for_unit <unit_id> <time>: show all valid processed paths in the pathfinder for a specific unit given a unit id |
| `show_battle_street_plan` | battle | no | show_battle_street_plan : show the street plan for the settlement |
| `show_cursorstat` | battle, campaign | **yes** | show_cursorstat : shows the cursor position and region id |
| `show_landings` | campaign | no | show_landings <opt:cursor/region_id> : shows the landing positions available to the ai from a given region, default hides them |
| `show_terrain_lines` | battle | no | show_terrain_lines <duration> : display defensive terrain features |
| `surrender_regions` | campaign | **yes** | surrender_regions <opt: horde> <opt:faction_type> : gives all this faction's regions to the slaves. "horde" surrenders regions for all factions that can be hordes |
| `test_ancillary_localisation` | campaign | no | adds all ancillary to the character info display ancillaries aren't actually added to the character |
| `test_message` | battle, campaign | no | Test the event message specified in descr_event_enums.txt |
| `test_network_scroll` | battle | no | test_network_scroll <is host (true/false)> |
| `test_victory_scroll` | campaign | no | test_victory_scroll <faction victor> <short campaign (true/false)>: Opens up the victory scroll declaring that the given faction is the victor If no faction is declared then the 'timeout' scroll is displayed |
| `toggle_chat_log` | battle (and in multiplayer) | no | toggle_chat_log : shows or hides the multiplayer chat window |
| `toggle_coastlines` | campaign, campaign editor | no | toggle_coastlines : toggles strategy map coastline display |
| `toggle_flowing_water` | campaign, campaign editor | no | toggles display of campaign map flowing water |
| `toggle_fow` | campaign | **yes** | toggle_fow : toggles the fog of war on or off |
| `toggle_game_update` | battle | no | toggle_game_update |
| `toggle_overlay` | battle, battle editor | no | toggle_overlay: |
| `toggle_perfect_spy` | campaign | no | toggle_perfect_spy : toggles everyone's spying ability to perfect and infinite range, and off again |
| `toggle_pr` | battle, campaign | no | toggle_pr: toggles pr mode. |
| `toggle_restrictcam` | battle | **yes** | toggle_restrictcam : toggles camera restrictions on or off |
| `toggle_terrain` | campaign | **yes** | toggle_terrain <opt:region/tiletype/climate/choke/landing/frontier/features/frontier_defend> : toggles the terrain to display various data sets, no param resets to normal |
| `toggle_tow` | campaign | no | toggle_tow : shows/hides the tabbed output window |
| `toggle_underlay` | battle, battle editor | no | toggle_underlay: |
| `trigger_advice` | battle, campaign | no | triggers an advice thread |
| `upgrade_effect` | battle | no | upgrade_effect <unit_id> : triggers unit upgrade effect |
| `victory` | campaign | no | victory <faction> <short>: show victory message for faction for short or long campaign. |
| `write_ui_cache` | battle, campaign | no | write_ui_cache: writes out the ui texture cache to disk |
| `zoom` | campaign | no | zoom : zooms to specified aerial map zoom |
| `zoom_to_unit` | battle | no | zoom_to_unit : zoom the camera to a unit of a specific id |

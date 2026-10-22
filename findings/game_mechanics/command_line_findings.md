# Command-line options: all of them, and which ones work

> **NYD (Not Yet Decompiled):** a claim marked NYD comes from disassembly of code that isn't decompiled yet, so it's an inference the matching source can't confirm yet. A heading marked NYD covers its whole section. Unmarked claims come from the decompiled source in [`src/`](../../src/).

**Short version:** `RomeTW.exe` makes **140** command-line option checks (138 different names), but **only the first 41 can ever take effect**. Right after it handles `-output_con_cmds`, the option reader returns. The other 99 checks, among them `-screen_res`, `-replay`, `-ghost`, `-host`, `-capture_audio` and a second `-enable_editor`, are compiled in but unreachable. The options that do work include `-mod:`, `-show_err`, `-nm`, `-ne`, `-multirun`, `-enable_editor`, `-strat:` and `-battle:`. `-default_campaign:` turns on the same file lookup as `-mod:`, and `-movie_cam:` with an unknown sub-option hangs the game. `-ai` hands every faction to the AI and never stops for a message, and with `-ai:enter_battles` every battle runs at its fastest speed. Two options the reader never checks still work, because the game's main function looks them up itself: `-quick_battle[:faction]` and `-snd_provider:"name"`.

Sources:

- The decompiled option reader and the `-mod` path functions, in [`src/code/romans_cmd_line.cpp`](../../src/code/romans_cmd_line.cpp).
  - Everything in the unit matches the original byte for byte, including the unreachable part.
- `WinMain` and the game's main function (`fn_00403412`), in [`src/code/romans.cpp`](../../src/code/romans.cpp).
  - `WinMain` matches byte for byte, and the main function differs from the original only in its stack slots.
- The decompiled code that reads the options' flags, all matching byte for byte except one stack slot pair in `Campaign::end_turn`.
  - The battle speeds in [`battle.cpp`](../../src/code/battle.cpp), the event messages' filter in [`ui/event_messages.cpp`](../../src/code/ui/event_messages.cpp) and their windows in [`ui/ui_message_handler.cpp`](../../src/code/ui/ui_message_handler.cpp).
  - The horde AI in [`ai/ai_global_strategy_director.cpp`](../../src/code/ai/ai_global_strategy_director.cpp), the end of a faction's turn in [`world.cpp`](../../src/code/world.cpp), and the turn's first selection in [`strategy_map.cpp`](../../src/code/strategy_map.cpp).
  - The keyboard shortcuts menu's buttons in [`menu/create_piece.cpp`](../../src/code/menu/create_piece.cpp).
- Disassembly of `RomeTW.exe` (Steam Rome: Total War Gold) for the library code that splits the command line, and for the rest of what reads the options' flags (NYD).

Addresses are virtual addresses in that exe (image base `0x400000`).

## How the command line is read

`WinMain` hands the command line to a splitter (`0xF3A500`, NYD), then calls the option reader (`0x408DCF`).

- The splitter keeps up to 256 options, each a name and a value (NYD).
  - A name follows a `-` or `/` and ends at a `:`, `=`, space or the end.
  - A value follows the `:` or `=`, and can be in double quotes.
- The reader looks each option up by name (`0xF3A770`), and the match is exact and case-sensitive (NYD).
- It checks the options in its own fixed order, not the command line's.
  - When two options set the same thing, the one checked later wins, whatever order they were typed in.
  - So `-no_exit_error` always beats `-show_err`, `-fplog` beats `-fperr`, which beats `-fpex`, and `-battle_ed:` beats `-strat:`, which beats `-battle:`.
- Each option that can be wrong reports a "Generic Error" (only the first error of a run is shown), and then the reader returns false.
  - `WinMain` then exits without starting the game.

## The options that work

These are checked before the reader's early return, in this order. Most only set a flag; [What the options change](#what-the-options-change) follows each flag to the code that reads it.

| Option | What the reader does |
|---|---|
| `-sw`, `-ne`, `-ae`, `-no_tnl`, `-nm`, `-na`, `-force_default_hz`, `-blank_ui_caches`, `-report_missing_images` | Each sets its own flag. |
| `-util:a,b,...` | Sets utility mode, and a flag for each of `validate_models`, `unit_models`, `encrypt`, and `animdb` or `sound` (not both). It's an error without options. |
| `-fpex`, `-fperr`, `-fplog` | Set a mode of 1, 2 or 3. `-fperr` and `-fplog` make `WinMain` run the game under `romans_seh.cpp`'s exception handler (`0x425F8F`), and `-fpex` changes nothing there. |
| `-enable_editor`, `-disr`, `-bookmark` | Each sets its own flag. |
| `-mod:folder` | Sets the mod folder. Without a value it resets the folder to `.` and the campaign to `imperial_campaign`, and turns the mod lookup off. |
| `-default_campaign:name` | Sets the campaign (256 characters at most), and turns the mod lookup on. |
| `-multirun` | Sets its flag. |
| `-battle:map`, `-strat:map`, `-battle_ed:map` | Each sets its own flag, and copies the map name into the one buffer they share. |
| `-gs_chat_name`, `-gs_group_room`, `-gs_game_password`, `-gs_email`, `-gs_profilenick`, `-gs_uniquenick`, `-gs_password`, `-gs_namespaceid`, `-gs_lan`, `-gs_host`, `-gs_join:a.b.c.d` | The GameSpy Arcade launcher's options: see below. |
| `-sprite_script`, `-show_err` | Each sets its own flag. |
| `-no_exit_error` | Clears `-show_err`'s flag. |
| `-movie_cam[:capture_video][:capture_audio]` | Sets the movie camera and two other flags, and clears two replay flags. `capture_video` and `capture_audio` each turn on a capture and quitting afterwards. |
| `-fbc` | Sets one of `-movie_cam`'s flags. |
| `-ai[:enter_battles=N]` | Sets a flag, and how many battles (0 at first; `enter_battles` without `=N` gives 1, and a negative N is ignored). |
| `-unlock_campaign` | Sets its flag. |
| `-output_con_cmds` | Writes every console command and its help to `documentation/console_commands.txt` (`Console::write_commands`, `0xDE19E7`, NYD for the file's format). |

A name option (`-mod`, `-default_campaign`, `-battle`, `-strat`, `-battle_ed`) stops at the first `-`, double quote, tab or space, and 256 characters or more is an error.

- `-mod:` measures the name that way, but then copies the whole value, so the folder can come out longer than the name it checked.

### `-mod` and `-default_campaign`

When the mod lookup is on, the game looks for a file under the mod folder first, and falls back to the normal one when it isn't there.

- A data file `x` is `<mod>/data/x` if that exists, else `data/x` (`0x40E567`; the folder version is `0x40E701`).
- A campaign map file `x` is `<mod>/data/world/maps/campaign/<campaign>/x` if that exists, else `data/world/maps/campaign/imperial_campaign/x` (`0x40E505` and `0x40E79C`, which are identical).
  - The fallback is always the imperial campaign's, whatever the campaign is.
- A path for another file is `<mod>/x` if that exists, else `x` (`0x40E3C9`).
- The saves and preferences folders are always `<mod>/saves` and `<mod>/preferences`, made when needed, even with the lookup off (`0x40E497`).
  - Without `-mod`, `<mod>` is `.`, so they're `./saves` and `./preferences`.

`-default_campaign:name` turns the lookup on without changing the folder.

- On its own it gives the folder `.`, so the campaign's map files come from `./data/world/maps/campaign/name/`, falling back to the imperial campaign's.

### GameSpy Arcade

The `-gs_` options are what the GameSpy Arcade launcher passes to start a multiplayer game.

- `-gs_chat_name:"name"` gives the player's name, `-gs_group_room:N` the room (the value's first character is skipped), and `-gs_game_password:"pw"` the game's password.
  - The quoted values are read up to the closing quote.
- When `-gs_email`, `-gs_profilenick`, `-gs_uniquenick`, `-gs_password` and `-gs_namespaceid` are all given, they replace the chat name, joined by backslashes in that order.
- `-gs_host` starts hosting, and `-gs_join:a.b.c.d` joins the host at that IPv4 address (only when all four numbers are there).
  - `-gs_lan` marks either one as a LAN game.

## What the options change

### `-ai`: the AI plays every faction

`-ai` is a way to watch the AI play itself: no faction is human, and nothing waits for a player.

- **No faction is human.** Starting a battle with `-battle:map` or a campaign with `-strat:map` gives the game an empty set of human factions (`fn_00403412`).
  - Without `-ai`, the human faction is `-faction`'s, and since `-faction` is never read, that's the lowest-numbered faction the map makes playable (`FactionMask::first`).
  - The main function also has hotseat cases (every playable faction human, or the unlockable ones too), but the hotseat mode is still 0 when it runs, since only the unreachable `-hm` sets it before the menus.
  - A campaign or custom battle started from the menu gets no human factions either (`0xD87220`, NYD).
  - `-quick_battle` ignores `-ai`: its first side is always human.
- **Every faction's event messages are made** (`fn_00CB52D0`).
  - Normally a faction's messages are made only when it's the player's, or in a hotseat game for every faction, and none while `BattleUIShell::unk_00B54B4C` says so (perhaps during a battle).
  - The `show_all_messages` console command makes them all too, as the player's.
- **Each message window is closed and deleted as soon as it's made** (`UIMessageHandler::open`), so no message waits to be read.
- **The game doesn't wait for a human's decision after a settlement is taken** (`0x575A4B`, NYD; probably the occupy, enslave or exterminate choice).
  - Without `-ai` it waits when the capturer is human, or in a hotseat game when it's the faction whose turn it is.
- **A question about two armies that general action `0xe` puts to each human player isn't asked** (`0x573EF5`, NYD).
- **A faction with no settlements gets the horde AI before it's flagged a horde** (`AIGlobalStrategyDirector::update_horde`).
  - That's when it could become a horde (`Faction::unk_004D43D5`), and its horde record's faction is none.
  - Without `-ai`, only a faction flagged a horde (`m_4f4`) gets the horde controller, and losing the flag deletes it.
- At the start of the player's turn, the campaign map doesn't select the character, settlement or fort that was waiting (`PathFinder::unk_004A4C7F`); neither does a hotseat game.
- The campaign's turn timings restart at each round (`Campaign::end_turn`), not only when a human's turn hands over to the AI.

`-ai:enter_battles=N` sets a count, but nothing ever counts it down. Its only readers are the battle speed functions, so all that matters is whether it's 0.

- **When it isn't 0, every battle runs at the fastest speed, 6 times normal, and can't be slowed** (`fn_005F5D9D`, `fn_005F5D4F`, `Battle::set_speed`, `Battle::set_all_speeds`).
  - The speeds are 0 (paused), 1, 2 and 6, and a battle normally runs at the slowest speed any active player asks for.
- `-ai:enter_battles` gives 1, so it's on; `-ai:enter_battles=0`, or a negative N, leaves it off.

### `-unlock_campaign` (NYD)

When the game loads its data (`0x405301`), `-unlock_campaign` writes a mask with every faction's bit set (`(1 << count) - 1`, `0xE308F0`) to `<data>/world/maps/campaign/<campaign>/unlocked.txt`.

- `<data>` is the `-mod` folder's `data` when there is one (`0x40E5C4`), and `<campaign>` is the `-default_campaign` one, `imperial_campaign` at first.
- The exe's other `/unlocked.txt` paths sit next to the `take_rome` and `outlive_factions` victory conditions' names, so the game presumably writes the file itself when a campaign is won, and the option unlocks every faction for that campaign.

### `-enable_editor`

- The keyboard shortcuts menu shows its categories one at a time: camera, battle, strat, misc and editor. The arrows skip the editor's without `-enable_editor` (`NextShortcutsButton::clicked`, `PreviousShortcutsButton::clicked`).
- One kind of menu button (vtable `0x105F024`) is disabled without it (`0xD39D45`, NYD).

### `-multirun`, `-battle`, `-strat` and the game's start

The game's main function (`fn_00403412`) opens the packs, makes the window, and starts the threads, the sound, the input and the display, then runs one mode and shuts down.

- **`-multirun` skips the check for a copy already running.** Without it, a second copy finds the mutex `Rome: Total War instance verification mutex` taken, shows text `0x172` and quits.
- **It picks the first mode whose flag is set**, in this order: `-view`, `-sprite_script`, `-replay` (without `save`), `-battle`, `-custom`, `-strat`, `-fmv`, `-menu`, `-host`, `-strat_ed`, `-battle_ed`, `-rtm`, `-quick_battle`, `-util`, and otherwise the legal screen, the two opening movies and the menus.
  - Only `-sprite_script`, `-battle`, `-strat`, `-battle_ed`, `-quick_battle` and `-util` can be set, since the reader returns before the others.
  - `-battle:map` and `-strat:map` start straight into that battle or campaign map, skipping the menus.
  - `-battle` beats `-strat` here, and `-strat` beats `-battle_ed`. Since all three share one map name and the reader's later option wins it, `-battle_ed:a -strat:b` starts a campaign on map `a`.
  - A GameSpy Arcade game skips the movies and goes straight to the menus.
- **When the sound can't start, the game stops** before the display starts, unless `-na` turns the sound off.
  - When the Miles 3D provider can't be set, it tries `Miles Fast 2D Positional Audio`, and stops with "Failed to initialise 3D audio!" when that fails too.

### Two options the reader never checks

The main function looks these two up by name itself, so they work although the option reader doesn't know them.

- **`-quick_battle[:faction]`** starts a quick battle for the faction (any faction without one), with its first side human.
  - The faction is its internal name, read with `%s` (`fn_00E3C320`).
- **`-snd_provider:"name"`** sets the Miles 3D sound provider (the default is `Miles Fast 2D Positional Audio`).
  - The value is read with `"%[a-zA-Z0-9().-+ ]"`, so it must be in double quotes, and it stops at any other character.

### The others (NYD)

- `-sw`, `-ne` and `-no_tnl` are read only by the display setup (`0xA8D3BD`), `-nm` only by the movie players (`code\game_fmv_handler.cpp`, `code\menu\ui_menu_fmv.cpp`), and `-na` by the sound code and the game's start.
- `-show_err` is read by `0x4050C9`, which the game's start calls, and `-bookmark` by the battle menu (`0xBB5ADB`).
- **`-ae` and `-disr` do nothing**: nothing reads their flags.

## The options that never work

These are all checked after the early return, so none of them can take effect. Their code says what each would have done.

- **Capture:** `-capture_tga`'s sub-options (`fps=N`, `dir=`), which the code reads from whatever value it was last given, since nothing looks `-capture_tga` itself up. Also `-capture_audio[:dir=]`.
- **Saves:** `-incremental_autosave[:N]`, `-end_of_turn_autosave`, `-save_game_fix_algorithm:N` (3 skips the checks of saved positions) and `-load_after_save`.
- **Starting straight into something:** `-overview:map`, `-fmv:map`, `-rtm:file`, `-custom:map`, `-strat_ed:map`, `-menu`, `-view:model` and `-dm:mount`.
- **Display:** `-screen_res:width,height`, `-minimal_ui`, `-hires_radar`, `-tv_banners`, `-showtime`, `-show_grid`, `-display_off`, `-engine`, `-engine_shadow`, `-nc`, `-sp`, `-sprn`, `-use_rtm_cam`, `-ob_cam`, `-daytime:minutes` (kept between 60 and 600,000) and `-tweak_pwdm:x`.
- **Battle:** `-no_damage`, `-no_battle_ai`, `-no_melee_interrupts`, `-infinite_missiles`, `-battle_ai_disable_outflanking`, `-battle_ai_force_outflanking`, `-battle_arcade_mode`, `-ignore_win_or_defeat`, `-desync_men`, `-dpr:range`, `-debug_fire_damage`, `-all_can_swim` and `-no_unit_cull`.
- **Campaign:** `-cheat`, `-faction:name`, `-df:faction`, `-hm:playable|all`, `-game_speed:N,on,skip_ai_factions`, `-disable_naval_ai`, `-disable_horde`, `-horde_log`, `-radar_ambush`, `-popularity_land_percentage:N`, `-force_campaign_battle_ai_defend:x,y`, `-dont_correct_regions`, `-check_building_infos`, `-custom_all` and `-use_campaign_costs_in_custom`.
- **Testing:** `-replay[:save|play|cont|speed=N|check|build_check_from_replay|nocheck|info|stop=N|flush[=N]|capture_and_quit|file="f"|cam="f"|compare="f"|show_battle_ui]`, `-ghost[:save|play|file="f"]`, `-qa`, `-diag_warn`, `-checksum:[+]MASK`, `-display_rand:N`, `-log=MASK[:ticks=MASK,start,end][:first_tick=MASK]`, `-nw[:log=MASK|bcast_on|bcast_off|mt_on|mt_off|local|resend=N]`, `-auto_quit`, `-auto_reload`, `-pr`, `-key:text`, `-test_daytypes`, `-dbmpf`, `-pdl`, `-dcdw` and `-hide_non_release_info`.
- **Multiplayer:** `-host:battle|strat:game[:player]`, `-lan`.
- **Memory and sound:** `-mem_force:N`, `-stop_alloc:hex`, `-stop_alloc_count:N`, `-vidmem:N`, `-snd_hw`, `-no_snd_hw`, `-snd_quality:N`, `-snd_profile:a:b:c:d` and `-snd_missing[:file]`.
- **Tools:** `-shortcuts:file`, `-ed_dir:folder`, `-prefs:file`, `-create_sprite_script`, `-output_skeleton_speeds`, `-ghm:file.tga`, `-rbm_delta:x`, `-generate_forest_canopy`, `-generate_forest_edging`, `-lbc_peasant_count:N` and `-ftow:file.tga`.
  - `-ftow` would have set the flag that makes `WinMain` read a file and exit without starting the game.
- **A second `-enable_editor`**, after the first one, which works.

Their help texts survive in the exe, such as "Replay options: -replay:save:play:speed=N:check:nocheck:stop=N:file=filename:show_battle_ui. -replay defaults to -replay:play:speed=1:check:file="network.rpy":stop=0 (never stops)".

## Bugs

- **`-movie_cam:` with any sub-option other than `capture_video` or `capture_audio` hangs the game.**
  - The loop only moves past a `:` or a name it knows, so on anything else (`-movie_cam:foo`) it never ends.
- **`-mod:` checks one length but copies another.** It measures the name only up to a `-`, space or tab, but copies the whole value, so `-mod:my-mod` is checked as `my` and stored as `my-mod`.
  - A value whose first part is short but whole is 256 characters or more would overflow the 256-character buffer.
- `-screen_res:"1024,768"` (in quotes) would have read a width of 0 and been rejected, since the width is read from before the opening quote. It's unreachable anyway.

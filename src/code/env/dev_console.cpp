#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\env\\dev_console.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

// The console (the ~ key in the game) and its commands.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unsorted.h"
#include "../battle.h"
#include "../faction.h"
#include "../romans_utils.h"
#include "../world.h"
#include "../world_map.h"
#include "../character.h"
#include "../character_record.h"
#include "../character_traits_db.h"
#include "dev_console.h"

#pragma intrinsic(strcmp)

// Where a command can be used: Console::m_mode is one of the first four.
enum ConsoleFlags {
    CONSOLE_BATTLE = 0x1,
    CONSOLE_CAMPAIGN = 0x2,
    CONSOLE_BATTLE_EDITOR = 0x4,
    CONSOLE_CAMPAIGN_EDITOR = 0x8,      // nothing sets this mode
    CONSOLE_RESTRICTED = 0x10,          // can be used while Console::m_restricted is set
    CONSOLE_MULTIPLAYER = 0x20,         // can be used in multiplayer (a guess)
};

class ConsoleCommand;

// A console command. Each one is a class of its own, with one instance that adds itself to the console (0x14 bytes).
class ConsoleCommand {
public:
    ConsoleCommand(unsigned int flags, const char* name, const char* help);  // 0x00DE1E0D

    // Runs the command on the rest of the line; any message goes to out.
    virtual bool execute(const char* args, char* out) = 0;

    bool unk_10() const { return m_10 != 0; }

    const char* m_name;                         // +0x04
    const char* m_help;                         // +0x08
    unsigned int m_flags;                       // +0x0c: ConsoleFlags
    int m_10;                                   // +0x10: -1, or 1 to 3 for some of the CONSOLE_RESTRICTED ones
};

extern "C" {
extern bool lbl_018C9D98;                       // probably set in multiplayer
extern UnkB4CFDA* lbl_029219D8;
extern char lbl_012CF098[];                    // reapply_rigid_model_influence's help
bool fn_00E2DDF0(const char* a, const char* b);  // whether the first words of a and b are the same
extern Battle* lbl_029219E4;
extern Battle* lbl_029219E8;
extern int lbl_010FDF20;                        // bestbuy's percentage: 100, or 90
extern bool lbl_01171980;                       // toggle_game_update
extern bool lbl_012CADA0;                       // PR mode (toggle_pr)
extern bool lbl_017B1C40;                       // the battle camera's restrictions (toggle_restrictcam)
extern bool lbl_012CB2F6;                       // bounds
extern bool lbl_017B2E0E;                       // set by reload_textures
extern int lbl_011A9F9C;                        // amdb_min
extern int lbl_015FC124;                        // amdb_max
extern float lbl_011A9FA0;                      // amdb_offset
void fn_00E28796();                             // in output_window.cpp
void fn_009CA053();
void fn_009CB01B();                             // toggle_coastlines
void fn_00DDF655();                             // building_debug
void fn_0040F17F();                             // perf_times
void fn_009A8D71();                             // reapply_rigid_model_influence
void fn_009CB277();                             // toggle_flowing_water
void fn_00BC0379();                             // toggle_chat_log
void fn_00CB6DC3(const char* args);             // test_message
}

class Unk01610450 {
public:
    void unk_009FFB4D();                        // 0x009FFB4D
};

extern "C" Unk01610450 lbl_01610450;

// Whether the tabbed output window is compiled in; it isn't.
inline bool has_output_window()
{
    return false;
}

Console g_console;

static char s_word[128];

static void write_error(const char* message, char* out);
static const char* skip_word(const char* text);
static char* next_word(const char* text);

Console::Console()
{
    // NONMATCHING - stack layout
    m_num_history = 0;
    m_mode = 0;
    m_restricted = false;
    char* none = 0;
    for (unsigned int i = 0; i < 10; i++)
        m_history[i] = none;
}

Console::~Console()
{
    for (unsigned int i = 0; i < 10; i++) {
        if (m_history[i] != 0)
            free(m_history[i]);
    }
}

bool Console::run(const char* text, char* out)
{
    // NONMATCHING - stack layout
    if (*text == '?') {
        if (m_mode == 0) {
            write_error("console mode not initialised, no commands available", out);
        } else if (text[1] == ' ' || text[1] == 0) {
            if (!m_restricted)
                list(out);
        } else {
            text++;
            help(text, out);
        }
        return false;
    }
    unsigned int slot = m_num_history % 10;
    if (m_history[slot] != 0)
        free(m_history[slot]);
    m_history[slot] = _strdup(text);
    m_num_history++;
    bool multiplayer = lbl_018C9D98;
    for (unsigned int i = 0; i < m_commands.size(); i++) {
        ConsoleCommand* command = m_commands[i];
        unsigned int flags = command->m_flags;
        bool allowed = (flags & m_mode) != 0;
        if (multiplayer && !(flags & CONSOLE_MULTIPLAYER))
            allowed = false;
        if (allowed && m_restricted) {
            if (!(flags & CONSOLE_RESTRICTED))
                allowed = false;
            else if (!command->unk_10())
                allowed = false;
        }
        if (allowed && fn_00E2DDF0(text, command->m_name)) {
            text = skip_word(text);
            return command->execute(text, out);
        }
    }
    if (m_mode == 0)
        write_error("console mode not initialised, no commands available", out);
    else
        write_error("command not recognised (use ? for help).", out);
    return false;
}

void Console::add(ConsoleCommand* command)
{
    // NONMATCHING - stack layout
    unsigned int i;
    for (i = 0; i < m_commands.size(); i++) {
        const char* name = m_commands[i]->m_name;
        int order = strcmp(command->m_name, name);
        if (order == 0)
            return;
        if (order < 0)
            break;
    }
    if (i == m_commands.size())
        m_commands.push_back(command);
    else
        m_commands.insert(command, i);
}

void Console::list(char* out)
{
    // NONMATCHING - the original nests the restricted and mode checks; stack layout
    char* p = out;
    char* end = p + 0x1000;
    p += sprintf(p, "RomeShell command listing:\n\n");
    for (unsigned int i = 0; i < m_commands.size(); i++) {
        if (m_restricted && !(m_commands[i]->m_flags & CONSOLE_RESTRICTED))
            continue;
        if (!(m_commands[i]->m_flags & m_mode))
            continue;
        p += sprintf(p, "%s\n", m_commands[i]->m_name);
        if (p > end - 100) {
            if (p < end - 22)
                sprintf(p, "\ntoo many commands...");
            return;
        }
    }
    sprintf(p, "\nuse '?cmdname' to retrieve help for a command");
    p += 46;
    sprintf(p, "\nmultiple word names should be wrapped in quotes");
}

void Console::help(const char* name, char* out)
{
    // NONMATCHING - the original nests the restricted, mode and name checks; stack layout
    for (unsigned int i = 0; i < m_commands.size(); i++) {
        if (m_restricted && !(m_commands[i]->m_flags & CONSOLE_RESTRICTED))
            continue;
        if (!(m_commands[i]->m_flags & m_mode))
            continue;
        if (fn_00E2DDF0(name, m_commands[i]->m_name)) {
            sprintf(out, "%s", m_commands[i]->m_help);
            return;
        }
    }
    sprintf(out, "err: command '%s' not recognised", name);
}

ConsoleCommand::ConsoleCommand(unsigned int flags, const char* name, const char* help)
    : m_name(name), m_help(help), m_flags(flags), m_10(-1)
{
    g_console.add(this);
}

// Writes "err: " and the message to out.
static void write_error(const char* message, char* out)
{
    // NONMATCHING - the original keeps each of the five fixed indices in a local of its own
    int length = 5;
    out[0] = 'e';
    out[1] = 'r';
    out[2] = 'r';
    out[3] = ':';
    out[4] = ' ';
    while (*message) {
        out[length] = *message;
        length++;
        message++;
    }
    out[length] = 0;
}

// Skips the first word of text (a quoted one whole), and the spaces and commas after it.
static const char* skip_word(const char* text)
{
    // NONMATCHING - the original reads the characters unsigned (movzx); stack layout
    if (*text == '"') {
        text++;
        for (;;) {
            if (*text == '"') {
                text++;
                break;
            }
            if (!*text)
                break;
            text++;
        }
    } else {
        while (*text != ' ' && *text != ',' && *text) {
            text++;
        }
    }
    while ((*text == ' ' || *text == ',') && *text) {
        text++;
    }
    return text;
}

// The character on the map with that name: the player's first, then the other factions'.
static Character* find_character(const char* name)
{
    // NONMATCHING - stack layout
    Faction* faction = lbl_029219D8->unk_00B4CFDA();
    Character* character = faction->unk_004C41EF(name);
    if (character != 0)
        return character;
    int player = faction->m_id;
    for (unsigned int i = 0; i < lbl_018CD818->m_num_factions; i++) {
        faction = lbl_018CD818->m_factions[i];
        if (faction->m_id != player) {
            character = faction->unk_004C41EF(name);
            if (character != 0)
                return character;
        }
    }
    return 0;
}

// The first word of text, without its quotes, in a shared buffer; 0 when it's 128 characters or longer.
static char* next_word(const char* text)
{
    // NONMATCHING - the original reads the characters unsigned (movzx); stack layout
    char* p = s_word;
    if (*text == '"') {
        text++;
        while (*text != '"' && *text) {
            *p = *text;
            p++;
            text++;
            if (p - s_word >= 0x80)
                return 0;
        }
    } else {
        while (*text != ' ' && *text != ',' && *text) {
            *p = *text;
            p++;
            text++;
            if (p - s_word >= 0x80)
                return 0;
        }
    }
    *p = 0;
    return s_word;
}

// The commands, in the order they're added to the console.
class ToggleTowCommand : public ConsoleCommand {
public:
    ToggleTowCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "toggle_tow",
        "toggle_tow : shows/hides the tabbed output window")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE33F9
};

static ToggleTowCommand s_toggle_tow;

class AddMoneyCommand : public ConsoleCommand {
public:
    AddMoneyCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "add_money",
        "add_money <opt:faction_type> <amount> : adds an amount of money to a faction's coffers, can be negative, default is player faction")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE3411
};

static AddMoneyCommand s_add_money;

class AddPopulationCommand : public ConsoleCommand {
public:
    AddPopulationCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "add_population",
        "add_population <settlement_name> <amount> : adds an amount of population to a settlement, can be negative")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE350A
};

static AddPopulationCommand s_add_population;

class MoveCharacterCommand : public ConsoleCommand {
public:
    MoveCharacterCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "move_character",
        "move_character <name> <x>,<y> : moves named character to position on campaign map")
    {
        m_10 = 2;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE360A
};

static MoveCharacterCommand s_move_character;

class AutoWinCommand : public ConsoleCommand {
public:
    AutoWinCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "auto_win",
        "auto_win <attacker/defender>: the attacker or defender wins the next autoresolved battle")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE36E8
};

static AutoWinCommand s_auto_win;

class CreateUnitCommand : public ConsoleCommand {
public:
    CreateUnitCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "create_unit",
        "create_unit <settlement/character_name> <unit_id> <opt:how_many> <opt:exp/armour/weapon>: creates one or more units of the specified type")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE37F1
};

static CreateUnitCommand s_create_unit;

class ToggleFowCommand : public ConsoleCommand {
public:
    ToggleFowCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "toggle_fow",
        "toggle_fow : toggles the fog of war on or off")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE3E67
};

static ToggleFowCommand s_toggle_fow;

class ToggleRestrictcamCommand : public ConsoleCommand {
public:
    ToggleRestrictcamCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_RESTRICTED, "toggle_restrictcam",
        "toggle_restrictcam : toggles camera restrictions on or off")
    {
        m_10 = 2;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE3EED
};

static ToggleRestrictcamCommand s_toggle_restrictcam;

class GiveAncillaryCommand : public ConsoleCommand {
public:
    GiveAncillaryCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "give_ancillary",
        "give_ancillary <charactername> <ancillary name> : gives the character an ancillary")
    {
        m_10 = 3;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE3F1C
};

static GiveAncillaryCommand s_give_ancillary;

class GiveTraitCommand : public ConsoleCommand {
public:
    GiveTraitCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "give_trait",
        "give_trait <charactername> <trait name> <opt:level>: gives the character a trait at level (default = level 1)")
    {
        m_10 = 3;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE3FED
};

static GiveTraitCommand s_give_trait;

class ProcessCqCommand : public ConsoleCommand {
public:
    ProcessCqCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "process_cq",
        "process_cq <settlement>: \nCompletes all (possible) construction pending in queue\n")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE40EC
};

static ProcessCqCommand s_process_cq;

class CharacterResetCommand : public ConsoleCommand {
public:
    CharacterResetCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "character_reset",
        "character_reset : resets the character back to it's start of turn settings")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE412E
};

static CharacterResetCommand s_character_reset;

class ShowCursorstatCommand : public ConsoleCommand {
public:
    ShowCursorstatCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "show_cursorstat",
        "show_cursorstat : shows the cursor position and region id")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE434B
};

static ShowCursorstatCommand s_show_cursorstat;

class ToggleTerrainCommand : public ConsoleCommand {
public:
    ToggleTerrainCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "toggle_terrain",
        "toggle_terrain <opt:region/tiletype/climate/choke/landing/frontier/features/frontier_defend> : toggles the terrain to display various data sets, no param resets to normal")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4472
};

static ToggleTerrainCommand s_toggle_terrain;

class BestbuyCommand : public ConsoleCommand {
public:
    BestbuyCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "bestbuy",
        "bestbuy : sells units cheaper")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE466C
};

static BestbuyCommand s_bestbuy;

class OliphauntCommand : public ConsoleCommand {
public:
    OliphauntCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "oliphaunt",
        "oliphaunt : the biggest around")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4698
};

static OliphauntCommand s_oliphaunt;

class JerichoCommand : public ConsoleCommand {
public:
    JerichoCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_RESTRICTED, "jericho",
        "jericho : and the walls came a-tumblin' down")
    {
        m_10 = 1;
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4B4D
};

static JerichoCommand s_jericho;

class WriteUiCacheCommand : public ConsoleCommand {
public:
    WriteUiCacheCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "write_ui_cache",
        "write_ui_cache: writes out the ui texture cache to disk")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4B65
};

static WriteUiCacheCommand s_write_ui_cache;

class GiveTraitPointsCommand : public ConsoleCommand {
public:
    GiveTraitPointsCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "give_trait_points",
        "give_trait_points <charactername> <trait name> <points>: gives the character points for trait")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4B7C
};

static GiveTraitPointsCommand s_give_trait_points;

class ListTraitsCommand : public ConsoleCommand {
public:
    ListTraitsCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "list_traits",
        "list_traits : lists all the available traits")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4C7B
};

static ListTraitsCommand s_list_traits;

class ListAncillariesCommand : public ConsoleCommand {
public:
    ListAncillariesCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "list_ancillaries",
        "list_ancillaries : lists all the available ancillaries")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4E04
};

static ListAncillariesCommand s_list_ancillaries;

class MpCommand : public ConsoleCommand {
public:
    MpCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "mp",
        "mp <charactername> <amount>: gives the character movement points")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE4F81
};

static MpCommand s_mp;

class ListCharactersCommand : public ConsoleCommand {
public:
    ListCharactersCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "list_characters",
        "list_characters <opt:faction_type> : lists all the characters in the world or those belonging to a faction")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5031
};

static ListCharactersCommand s_list_characters;

class ShowLandingsCommand : public ConsoleCommand {
public:
    ShowLandingsCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "show_landings",
        "show_landings <opt:cursor/region_id> : shows the landing positions available to the ai from a given region, default hides them")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE53B0
};

static ShowLandingsCommand s_show_landings;

class FilterCoastlinesCommand : public ConsoleCommand {
public:
    FilterCoastlinesCommand() : ConsoleCommand(CONSOLE_CAMPAIGN_EDITOR, "filter_coastlines",
        "filter_coastlines : applies filter to world map coastlines")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE56C1
};

static FilterCoastlinesCommand s_filter_coastlines;

class ToggleCoastlinesCommand : public ConsoleCommand {
public:
    ToggleCoastlinesCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_CAMPAIGN_EDITOR, "toggle_coastlines",
        "toggle_coastlines : toggles strategy map coastline display")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE56E8
};

static ToggleCoastlinesCommand s_toggle_coastlines;

class SetBuildingHealthCommand : public ConsoleCommand {
public:
    SetBuildingHealthCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "set_building_health",
        "set_building_health <settlement_name> <building_chain> <final health percent>: \nsets health of a building of the specified type (eg core_building) \nin a settlement, so that the final health percentage is as specified; for building chains see export_descr_building.txt")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE56FA
};

static SetBuildingHealthCommand s_set_building_health;

class AiTurnSpeedCommand : public ConsoleCommand {
public:
    AiTurnSpeedCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "ai_turn_speed",
        "ai_turn_speed <multiplier> : sets the maximum speed of turn processing during the ai round")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5A37
};

static AiTurnSpeedCommand s_ai_turn_speed;

class AmdbMinCommand : public ConsoleCommand {
public:
    AmdbMinCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "amdb_min",
        "amdb_min : sets aerial map overlay depth bias for min zoom")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5A7B
};

static AmdbMinCommand s_amdb_min;

class AmdbMaxCommand : public ConsoleCommand {
public:
    AmdbMaxCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "amdb_max",
        "amdb_max : sets aerial map overlay depth bias for max zoom")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5A96
};

static AmdbMaxCommand s_amdb_max;

class AmdbOffsetCommand : public ConsoleCommand {
public:
    AmdbOffsetCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "amdb_offset",
        "amdb_offset : sets aerial map overlay offset towards camera")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5AB1
};

static AmdbOffsetCommand s_amdb_offset;

class ZoomCommand : public ConsoleCommand {
public:
    ZoomCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "zoom",
        "zoom : zooms to specified aerial map zoom")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5ACD
};

static ZoomCommand s_zoom;

class SetRankingIntervalCommand : public ConsoleCommand {
public:
    SetRankingIntervalCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "set_ranking_interval",
        "set_ranking_interval : sets the denominator of the faction ranking graph interval which is calculated as (number_of_turns / denominator).  If set to 0, then the denominator will be set to number_of_turns, giving an interval of 1")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5B1A
};

static SetRankingIntervalCommand s_set_ranking_interval;

class RegenerateRadarCommand : public ConsoleCommand {
public:
    RegenerateRadarCommand() : ConsoleCommand(CONSOLE_BATTLE, "regenerate_radar",
        "regenerate_radar: Does what it says on the tin")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5B3C
};

static RegenerateRadarCommand s_regenerate_radar;

class AdjustSeaBedCommand : public ConsoleCommand {
public:
    AdjustSeaBedCommand() : ConsoleCommand(CONSOLE_CAMPAIGN_EDITOR, "adjust_sea_bed",
        "adjust_sea_bed : adjusts whole sea bed to specified height")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5B55
};

static AdjustSeaBedCommand s_adjust_sea_bed;

class ReloadShadersCommand : public ConsoleCommand {
public:
    ReloadShadersCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "reload_shaders",
        "reload_shaders : reloads all vertex shaders")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5B90
};

static ReloadShadersCommand s_reload_shaders;

class ReloadTexturesCommand : public ConsoleCommand {
public:
    ReloadTexturesCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "reload_textures",
        "reload_textures : reloads all textures")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5BAA
};

static ReloadTexturesCommand s_reload_textures;

class FireCommand : public ConsoleCommand {
public:
    FireCommand() : ConsoleCommand(CONSOLE_BATTLE, "fire",
        "fire (mt | toggle | reload | int)")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5BBE
};

static FireCommand s_fire;

class ToggleGameUpdateCommand : public ConsoleCommand {
public:
    ToggleGameUpdateCommand() : ConsoleCommand(CONSOLE_BATTLE, "toggle_game_update",
        "toggle_game_update")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5BCB
};

static ToggleGameUpdateCommand s_toggle_game_update;

class TogglePerfectSpyCommand : public ConsoleCommand {
public:
    TogglePerfectSpyCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "toggle_perfect_spy",
        "toggle_perfect_spy : toggles everyone's spying ability to perfect and infinite range, and off again")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5BE9
};

static TogglePerfectSpyCommand s_toggle_perfect_spy;

class BuildingDebugCommand : public ConsoleCommand {
public:
    BuildingDebugCommand() : ConsoleCommand(CONSOLE_BATTLE, "building_debug",
        "building_debug: \ntoggles building debug mode. \n  TAB to toggle view modes, RIGHT SHIFT-TAB to reset mode, LEFT SHIFT-TAB to go back modes.  \n  G damages mouse-over building.  P displays plaza.\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5C07
};

static BuildingDebugCommand s_building_debug;

class ResetDisplayCommand : public ConsoleCommand {
public:
    ResetDisplayCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN | CONSOLE_BATTLE_EDITOR | CONSOLE_CAMPAIGN_EDITOR, "reset_display",
        "reset_display: \nForces a display_close(); display_open() display reset cycle\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5C19
};

static ResetDisplayCommand s_reset_display;

class ToggleUnderlayCommand : public ConsoleCommand {
public:
    ToggleUnderlayCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_BATTLE_EDITOR, "toggle_underlay",
        "toggle_underlay: \n\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5C4E
};

static ToggleUnderlayCommand s_toggle_underlay;

class ToggleOverlayCommand : public ConsoleCommand {
public:
    ToggleOverlayCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_BATTLE_EDITOR, "toggle_overlay",
        "toggle_overlay: \n\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5CAA
};

static ToggleOverlayCommand s_toggle_overlay;

class ProcessRqCommand : public ConsoleCommand {
public:
    ProcessRqCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "process_rq",
        "process_rq <settlement>: \nCompletes all (possible) recruitment pending in queue\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5D06
};

static ProcessRqCommand s_process_rq;

class ForceDiplomacyCommand : public ConsoleCommand {
public:
    ForceDiplomacyCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "force_diplomacy",
        "force_diplomacy <accept/decline/off>: \nForces the negotiator to accept or decline a proposition\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5D48
};

static ForceDiplomacyCommand s_force_diplomacy;

class DiplomaticStanceCommand : public ConsoleCommand {
public:
    DiplomaticStanceCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "diplomatic_stance",
        "diplomatic_stance <faction_a> <faction_b> <allied/neutral/war>: \nSet the diplomatic stance between the two factions\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5DC8
};

static DiplomaticStanceCommand s_diplomatic_stance;

class ShadowCommand : public ConsoleCommand {
public:
    ShadowCommand() : ConsoleCommand(CONSOLE_BATTLE, "shadow",
        "shadow <toggle/next/prev>\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE5F5D
};

static ShadowCommand s_shadow;

class IeCommand : public ConsoleCommand {
public:
    IeCommand() : ConsoleCommand(CONSOLE_BATTLE, "ie",
        "ie <reload>\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE60A5
};

static IeCommand s_ie;

class InvulnerableGeneralCommand : public ConsoleCommand {
public:
    InvulnerableGeneralCommand() : ConsoleCommand(CONSOLE_BATTLE, "invulnerable_general",
        "invulnerable_general <character_name> : makes that named general invulnerable in battle")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6188
};

static InvulnerableGeneralCommand s_invulnerable_general;

class TestAncillaryLocalisationCommand : public ConsoleCommand {
public:
    TestAncillaryLocalisationCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "test_ancillary_localisation",
        "adds all ancillary to the character info display\nancillaries aren't actually added to the character")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE620A
};

static TestAncillaryLocalisationCommand s_test_ancillary_localisation;

class PerfTimesCommand : public ConsoleCommand {
public:
    PerfTimesCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "perf_times",
        "Toggle display of simple performance times of game update vs display")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6233
};

static PerfTimesCommand s_perf_times;

class BurnPiggiesBurnCommand : public ConsoleCommand {
public:
    BurnPiggiesBurnCommand() : ConsoleCommand(CONSOLE_BATTLE, "burn_piggies_burn",
        "burn_piggies_burn  <on/off> : ignite all the piggy winks")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6245
};

static BurnPiggiesBurnCommand s_burn_piggies_burn;

class TestMessageCommand : public ConsoleCommand {
public:
    TestMessageCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "test_message",
        "Test the event message specified in descr_event_enums.txt")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6375
};

static TestMessageCommand s_test_message;

class ShowTerrainLinesCommand : public ConsoleCommand {
public:
    ShowTerrainLinesCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_terrain_lines",
        "show_terrain_lines <duration> : display defensive terrain features")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE638B
};

static ShowTerrainLinesCommand s_show_terrain_lines;

class MessageCollationSetCommand : public ConsoleCommand {
public:
    MessageCollationSetCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "message_collation_set",
        "Set the message collation on or off (sets all factions)")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE63C3
};

static MessageCollationSetCommand s_message_collation_set;

class ShowAllMessagesCommand : public ConsoleCommand {
public:
    ShowAllMessagesCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "show_all_messages",
        "Show all messages to all factions (on/off)")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE64F4
};

static ShowAllMessagesCommand s_show_all_messages;

class ClearMessagesCommand : public ConsoleCommand {
public:
    ClearMessagesCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "clear_messages",
        "Clear all the current stacked messages")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE660D
};

static ClearMessagesCommand s_clear_messages;

class PuppifyMyLoveCommand : public ConsoleCommand {
public:
    PuppifyMyLoveCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "puppify_my_love",
        "??????")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6657
};

static PuppifyMyLoveCommand s_puppify_my_love;

class ReapplyRigidModelInfluenceCommand : public ConsoleCommand {
public:
    ReapplyRigidModelInfluenceCommand() : ConsoleCommand(CONSOLE_BATTLE_EDITOR, "reapply_rigid_model_influence",
        lbl_012CF098)
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6681
};

static ReapplyRigidModelInfluenceCommand s_reapply_rigid_model_influence;

class ToggleFlowingWaterCommand : public ConsoleCommand {
public:
    ToggleFlowingWaterCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_CAMPAIGN_EDITOR, "toggle_flowing_water",
        "toggles display of campaign map flowing water")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6693
};

static ToggleFlowingWaterCommand s_toggle_flowing_water;

class NwStatsCommand : public ConsoleCommand {
public:
    NwStatsCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_MULTIPLAYER, "nw_stats",
        "nw_stats: \ntoggles display of network stats.\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE66A5
};

static NwStatsCommand s_nw_stats;

class TogglePrCommand : public ConsoleCommand {
public:
    TogglePrCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "toggle_pr",
        "toggle_pr: \ntoggles pr mode.\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE66BC
};

static TogglePrCommand s_toggle_pr;

class ListUnitsCommand : public ConsoleCommand {
public:
    ListUnitsCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "list_units",
        "list_units <character/settlement name> : lists all of the units in an army, with details.")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE66DA
};

static ListUnitsCommand s_list_units;

class VictoryCommand : public ConsoleCommand {
public:
    VictoryCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "victory",
        "victory <faction> <short>: show victory message for faction for short or long campaign.")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE688B
};

static VictoryCommand s_victory;

class TriggerAdviceCommand : public ConsoleCommand {
public:
    TriggerAdviceCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "trigger_advice",
        "triggers an advice thread\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE696A
};

static TriggerAdviceCommand s_trigger_advice;

class DamageWallCommand : public ConsoleCommand {
public:
    DamageWallCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "damage_wall",
        "damage_wall <settlement> <gate> <breach>: \nDamage wall of settlement.  Forces 40% damage to a random gatehouse and a nearby straight section. Destroys gatehouse if 'gate' parameter present; breaches wall if 'breach' present\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6A78
};

static DamageWallCommand s_damage_wall;

class TestVictoryScrollCommand : public ConsoleCommand {
public:
    TestVictoryScrollCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "test_victory_scroll",
        "test_victory_scroll <faction victor> <short campaign (true/false)>: \nOpens up the victory scroll declaring that the given faction is the victor\nIf no faction is declared then the 'timeout' scroll is displayed")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6BD5
};

static TestVictoryScrollCommand s_test_victory_scroll;

class TestNetworkScrollCommand : public ConsoleCommand {
public:
    TestNetworkScrollCommand() : ConsoleCommand(CONSOLE_BATTLE, "test_network_scroll",
        "test_network_scroll <is host (true/false)>")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6D37
};

static TestNetworkScrollCommand s_test_network_scroll;

class DateCommand : public ConsoleCommand {
public:
    DateCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "date",
        "date <year> : changes the campaign date to the given year")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6E2A
};

static DateCommand s_date;

class SeasonCommand : public ConsoleCommand {
public:
    SeasonCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "season",
        "season <season> : changes the campaign season to the given season")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6EBF
};

static SeasonCommand s_season;

class UpgradeEffectCommand : public ConsoleCommand {
public:
    UpgradeEffectCommand() : ConsoleCommand(CONSOLE_BATTLE, "upgrade_effect",
        "upgrade_effect <unit_id> : triggers unit upgrade effect")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6F27
};

static UpgradeEffectCommand s_upgrade_effect;

class ForceBattleVictoryCommand : public ConsoleCommand {
public:
    ForceBattleVictoryCommand() : ConsoleCommand(CONSOLE_BATTLE, "force_battle_victory",
        "force_battle_victory : forces the local player's alliance to win the battle, completely destroying the enemy alliance")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6F79
};

static ForceBattleVictoryCommand s_force_battle_victory;

class ForceBattleDefeatCommand : public ConsoleCommand {
public:
    ForceBattleDefeatCommand() : ConsoleCommand(CONSOLE_BATTLE, "force_battle_defeat",
        "force_battle_defeat : forces the local player's alliance to win the battle, completely destroying the enemy alliance")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE6FC1
};

static ForceBattleDefeatCommand s_force_battle_defeat;

class OutputUnitPositionsCommand : public ConsoleCommand {
public:
    OutputUnitPositionsCommand() : ConsoleCommand(CONSOLE_BATTLE, "output_unit_positions",
        "output_unit_positions <filename> : output the positions of all units in the battle to the specified file")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE701D
};

static OutputUnitPositionsCommand s_output_unit_positions;

class ShowBattlePathsCommand : public ConsoleCommand {
public:
    ShowBattlePathsCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_paths",
        "show_battle_paths : show all valid processed paths in the pathfinder")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7257
};

static ShowBattlePathsCommand s_show_battle_paths;

class ShowBattlePathsForUnitCommand : public ConsoleCommand {
public:
    ShowBattlePathsForUnitCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_paths_for_unit",
        "show_battle_paths_for_unit <unit_id> <time>: show all valid processed paths in the pathfinder for a specific unit given a unit id")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE729D
};

static ShowBattlePathsForUnitCommand s_show_battle_paths_for_unit;

class ZoomToUnitCommand : public ConsoleCommand {
public:
    ZoomToUnitCommand() : ConsoleCommand(CONSOLE_BATTLE, "zoom_to_unit",
        "zoom_to_unit : zoom the camera to a unit of a specific id\n")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE72FF
};

static ZoomToUnitCommand s_zoom_to_unit;

class ShowBattleStreetPlanCommand : public ConsoleCommand {
public:
    ShowBattleStreetPlanCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_street_plan",
        "show_battle_street_plan : show the street plan for the settlement")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE74AF
};

static ShowBattleStreetPlanCommand s_show_battle_street_plan;

class ShowBattleMarkerCommand : public ConsoleCommand {
public:
    ShowBattleMarkerCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_marker",
        "show_battle_marker <x> <y> <t> : display a marker at (x, y) for t seconds")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE74F5
};

static ShowBattleMarkerCommand s_show_battle_marker;

class ShowBattleLineCommand : public ConsoleCommand {
public:
    ShowBattleLineCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_line",
        "show_battle_line <x1> <y1> <x2> <y2> <t> : display a line from (x1, y1) to (x2, y2) for t seconds")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7596
};

static ShowBattleLineCommand s_show_battle_line;

class ShowBattleCircleCommand : public ConsoleCommand {
public:
    ShowBattleCircleCommand() : ConsoleCommand(CONSOLE_BATTLE, "show_battle_circle",
        "show_battle_circle <x> <y> <r> <t> : display a circle at (x, y) of r radius for t seconds")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7673
};

static ShowBattleCircleCommand s_show_battle_circle;

class KillFactionCommand : public ConsoleCommand {
public:
    KillFactionCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "kill_faction",
        "kill_faction <faction_type> : removes the faction from the game")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7732
};

static KillFactionCommand s_kill_faction;

class DiplomacyMissionCommand : public ConsoleCommand {
public:
    DiplomacyMissionCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "diplomacy_mission",
        "diplomacy_mission <ai_faction> <target_faction> <mission_type> <opt:mission_target> : creates a diplomacy mission")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE77DB
};

static DiplomacyMissionCommand s_diplomacy_mission;

class EventCommand : public ConsoleCommand {
public:
    EventCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "event",
        "event <event_type> <opt:position> : creates an event at position")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7B0E
};

static EventCommand s_event;

class KillCharacterCommand : public ConsoleCommand {
public:
    KillCharacterCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "kill_character",
        "kill_character <character_name> : kills a character with the given name")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7C01
};

static KillCharacterCommand s_kill_character;

class ControlCommand : public ConsoleCommand {
public:
    ControlCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "control",
        "control <faction_type> : switches player control to specified faction; old faction may not act correctly as ai faction")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7C43
};

static ControlCommand s_control;

class CreateBuildingCommand : public ConsoleCommand {
public:
    CreateBuildingCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "create_building",
        "create_building <settlement_name> <building_level_id> : creates a building of the specified type in a settlement; for building level id's see export_descr_building.txt")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE7DD5
};

static CreateBuildingCommand s_create_building;

class CaptureSettlementCommand : public ConsoleCommand {
public:
    CaptureSettlementCommand() : ConsoleCommand(CONSOLE_CAMPAIGN, "capture_settlement",
        "capture_settlement <settlement name> : evicts any resident characters and armies and gives the settlement to the local player")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE8088
};

static CaptureSettlementCommand s_capture_settlement;

class DisableAiCommand : public ConsoleCommand {
public:
    DisableAiCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "disable_ai",
        "disable_ai [opt: tac | sub | dip | name]: disables all (default) or part of the ai for all factions")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE811E
};

static DisableAiCommand s_disable_ai;

class HaltAiCommand : public ConsoleCommand {
public:
    HaltAiCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "halt_ai",
        "halt_ai <opt:factiontypename> : halts the turn sequence just before the start of the specified faction's turn, or the current faction if no faction given")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE8319
};

static HaltAiCommand s_halt_ai;

class RunAiCommand : public ConsoleCommand {
public:
    RunAiCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "run_ai",
        "run_ai [opt:x]: re-starts an ai turn sequence after disableai or haltai has been triggered; \nuse x to also re-enable all parts of the ai which have been individually disabled")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE8369
};

static RunAiCommand s_run_ai;

class SurrenderRegionsCommand : public ConsoleCommand {
public:
    SurrenderRegionsCommand() : ConsoleCommand(CONSOLE_CAMPAIGN | CONSOLE_RESTRICTED, "surrender_regions",
        "surrender_regions <opt: horde> <opt:faction_type> : gives all this faction's regions to the slaves. \"horde\" surrenders regions for all factions that can be hordes")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE8471
};

static SurrenderRegionsCommand s_surrender_regions;

class ToggleChatLogCommand : public ConsoleCommand {
public:
    ToggleChatLogCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_MULTIPLAYER, "toggle_chat_log",
        "toggle_chat_log : shows  or hides the multiplayer chat window")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE85D3
};

static ToggleChatLogCommand s_toggle_chat_log;

class BoundsCommand : public ConsoleCommand {
public:
    BoundsCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "bounds",
        "bounds : toggle display of bounding objects")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE85E5
};

static BoundsCommand s_bounds;

class LightsCommand : public ConsoleCommand {
public:
    LightsCommand() : ConsoleCommand(CONSOLE_BATTLE | CONSOLE_CAMPAIGN, "lights",
        "lights : <merge> <used> <auto>")
    {
    }

    virtual bool execute(const char* args, char* out);    // 0x00DE8603
};

static LightsCommand s_lights;

bool ToggleTowCommand::execute(const char* args, char* out)
{
    if (has_output_window())
        fn_00E28796();
    return true;
}

bool ToggleRestrictcamCommand::execute(const char* args, char* out)
{
    bool restricted = lbl_017B1C40;
    if (!lbl_012CADA0)
        lbl_017B1C40 = !restricted;
    return true;
}

bool GiveTraitCommand::execute(const char* args, char* out)
{
    // NONMATCHING - the original reads *args unsigned (movzx); stack layout
    Character* character = find_character(next_word(args));
    if (character == 0) {
        write_error("character not found", out);
        return false;
    }
    args = skip_word(args);
    char* name = next_word(args);
    String key;
    key.set_scratch(name);
    TraitDescription* trait = find_trait(key);
    key.detach();
    if (trait != 0) {
        args = skip_word(args);
        unsigned int level = 1;
        if (*args)
            level = atoi(args);
        character->record()->set_trait(trait, level, true);
    } else {
        write_error("Could not find this trait in the database", out);
        return false;
    }
    return true;
}

bool BestbuyCommand::execute(const char* args, char* out)
{
    if (lbl_010FDF20 == 100)
        lbl_010FDF20 = 90;
    else
        lbl_010FDF20 = 100;
    return true;
}

bool GiveTraitPointsCommand::execute(const char* args, char* out)
{
    // NONMATCHING - the original reads *args unsigned (movzx); stack layout
    Character* character = find_character(next_word(args));
    if (character == 0) {
        write_error("character not found", out);
        return false;
    }
    args = skip_word(args);
    char* name = next_word(args);
    String key;
    key.set_scratch(name);
    TraitDescription* trait = find_trait(key);
    key.detach();
    if (trait != 0) {
        args = skip_word(args);
        int points = 1;
        if (*args)
            points = atoi(args);
        character->record()->add_trait_points(trait, points, true);
    } else {
        write_error("Could not find this trait in the database", out);
        return false;
    }
    return true;
}

bool FilterCoastlinesCommand::execute(const char* args, char* out)
{
    lbl_018CD80C->unk_00479BE9();
    fn_009CA053();
    lbl_01610450.unk_009FFB4D();
    return true;
}

bool ToggleCoastlinesCommand::execute(const char* args, char* out)
{
    fn_009CB01B();
    return true;
}

bool AmdbMinCommand::execute(const char* args, char* out)
{
    lbl_011A9F9C = atoi(args);
    return true;
}

bool AmdbMaxCommand::execute(const char* args, char* out)
{
    lbl_015FC124 = atoi(args);
    return true;
}

bool AmdbOffsetCommand::execute(const char* args, char* out)
{
    lbl_011A9FA0 = (float)atof(args);
    return true;
}

bool ReloadTexturesCommand::execute(const char* args, char* out)
{
    lbl_017B2E0E = true;
    return true;
}

bool FireCommand::execute(const char* args, char* out)
{
    return true;
}

bool ToggleGameUpdateCommand::execute(const char* args, char* out)
{
    lbl_01171980 = !lbl_01171980;
    return true;
}

bool BuildingDebugCommand::execute(const char* args, char* out)
{
    fn_00DDF655();
    return true;
}

bool PerfTimesCommand::execute(const char* args, char* out)
{
    fn_0040F17F();
    return true;
}

bool TestMessageCommand::execute(const char* args, char* out)
{
    fn_00CB6DC3(args);
    return true;
}

bool ReapplyRigidModelInfluenceCommand::execute(const char* args, char* out)
{
    fn_009A8D71();
    return true;
}

bool ToggleFlowingWaterCommand::execute(const char* args, char* out)
{
    fn_009CB277();
    return true;
}

bool TogglePrCommand::execute(const char* args, char* out)
{
    lbl_012CADA0 = !lbl_012CADA0;
    return true;
}

// The player's side wins.
bool ForceBattleVictoryCommand::execute(const char* args, char* out)
{
    // NONMATCHING - stack slot order differs
    Faction* player = lbl_029219D8->unk_00B4CFDA();
    int faction_id = player->m_id;
    int side = lbl_029219E8->side_of_faction(faction_id);
    lbl_029219E8->force_victory(side);
    return true;
}

// The side after the player's wins, so the player loses (its help says the opposite).
bool ForceBattleDefeatCommand::execute(const char* args, char* out)
{
    // NONMATCHING - stack slot order differs
    Faction* player = lbl_029219D8->unk_00B4CFDA();
    int faction_id = player->m_id;
    unsigned int num_sides = lbl_029219E4->num_sides();
    unsigned int side = (lbl_029219E4->side_of_faction(faction_id) + 1) % num_sides;
    lbl_029219E8->force_victory(side);
    return true;
}

bool ToggleChatLogCommand::execute(const char* args, char* out)
{
    fn_00BC0379();
    return true;
}

bool BoundsCommand::execute(const char* args, char* out)
{
    lbl_012CB2F6 = !lbl_012CB2F6;
    return true;
}

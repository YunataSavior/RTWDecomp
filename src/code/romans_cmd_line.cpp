#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\romans_cmd_line.cpp"
static char THIS_FILE[] = __FILE__;

// The command line's options, and the paths of files in the -mod folder. (The original's path: nothing asserts, but
// its THIS_FILE comes after romans.cpp's.)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unsorted.h"
#include "angle_constants.h"
#include "unk_id.h"
#include "romans_cmd_line.h"
#include "romans_utils.h"
#include "env/dev_console.h"

#pragma intrinsic(strcmp, strcpy, strlen)

// A file in a folder, or the folder itself (ca_libs; 0x1c bytes).
class FileFind {
public:
    FileFind(const UniString& path);            // 0x00F3B430
    ~FileFind();                                // 0x00F3B9F0
    bool found();                               // 0x00F3B9C0
    void close();                               // 0x00F3B9D0

    char m_data[0x1c];
};

extern "C" {
unsigned char* fn_00F3A770(const char* name);  // the option's value ("" for none), or 0 when it wasn't given
bool fn_00E2F761(const char* path);             // whether the file exists (code\romans_utils.cpp)
bool fn_00E2F6EF(const UniString& path);        // the same
void fn_00F3B090(const unsigned short* path);   // makes a folder (a guess)
int fn_00E3C320(const char* name);              // a faction from its name (code\enum_names.cpp)
char* fn_004255B6(const char* text);            // a key for the text (code\romans_key.cpp)
void fn_00425622(const char* key);              // (code\romans_key.cpp)
void fn_00D7FD25(int width, int height);        // sets the screen resolution (code\menu\menu_base.cpp)
void fn_00D7FD94();

extern const int lbl_00FE4728;                  // DiagnosticSettings' first checksum mask, 0x2aa85
extern int lbl_010216E8;                        // NetSettings' first log mask, 0x7f

// Set by the options; most aren't identified beyond their option's name.
extern bool lbl_012CA75F;                       // -sw
extern bool lbl_012CA75D;                       // -ne
extern bool lbl_012CA75E;                       // -ae
extern bool lbl_012CA760;                       // -no_tnl
extern bool lbl_012CA761;                       // -nm
extern bool lbl_012CA763;                       // -force_default_hz
extern bool lbl_02926E68;                       // -blank_ui_caches
extern bool lbl_012CB2F9;                       // -report_missing_images
extern bool lbl_012CB1EB;                       // -util:encrypt
extern bool lbl_012CA765;                       // -disr
extern bool lbl_012CAFB4;                       // -bookmark
extern bool lbl_012CAC9E;                       // a -mod folder was given
extern char lbl_010B5078[];                     // the -mod folder, "." at first
extern bool lbl_012CB1F1;                       // -show_err
extern bool lbl_012CA764;                       // -movie_cam, -use_rtm_cam
extern bool lbl_012CAFB3;                       // -movie_cam, -fbc
extern bool lbl_012CAFB6;                       // -movie_cam, -tv_banners
extern bool lbl_012CA981;
extern bool lbl_010B5068;
extern bool lbl_012CADA2;                       // -movie_cam:capture_video
extern bool lbl_012CA988;                       // capturing, then quitting
extern bool lbl_012CB1E8;                       // -capture_audio
extern int lbl_010B537C;                        // -capture_tga:fps=N
extern char lbl_010B5380[];                     // -capture_tga:dir=, "capture_tga" at first
extern bool lbl_012CA878;                       // -sprn
extern bool lbl_012CB0C8;                       // -hires_radar
extern bool lbl_012CB0C9;                       // -load_after_save
extern bool lbl_012CB0CA;                       // -incremental_autosave
extern int lbl_012CB0CC;
extern bool lbl_012CB0CB;                       // -end_of_turn_autosave
extern int lbl_012CB0D0;                        // -save_game_fix_algorithm (3 skips the checks of saved positions)
extern bool lbl_012CB0C5;                       // -cheat
extern unsigned char* lbl_012CA754;             // -shortcuts
extern unsigned char* lbl_012CA758;             // -ed_dir
extern bool lbl_012CAC9C;                       // -auto_quit
extern bool lbl_0126AE34;                       // cleared by -minimal_ui
extern bool lbl_012CAFB5;                       // -showtime
extern bool lbl_012CAA91;                       // -force_campaign_battle_ai_defend
extern bool lbl_012CAA92;                       // -battle_ai_disable_outflanking
extern bool lbl_012CAA93;                       // -battle_ai_force_outflanking
extern int lbl_010B5070;                        // -popularity_land_percentage
extern float lbl_010B5378;                      // -position_scale
extern bool lbl_012CAFAE;                       // -show_grid
extern bool lbl_012CA762;                       // -nc
extern bool lbl_012CA767;                       // -ignore_win_or_defeat
extern bool lbl_012CA76C;                       // -battle_arcade_mode
extern bool lbl_012CA87B;                       // -lan
extern bool lbl_012CAB9A;                       // -prefs was given a file
extern char lbl_012CAB9C[];                     // -prefs' file
extern bool lbl_012CA771;                       // -overview
extern bool lbl_012CA87D;                       // -create_sprite_script
extern bool lbl_012CA87E;                       // -output_skeleton_speeds
extern char lbl_0126AE2D;                       // -game_speed
extern bool lbl_02945094;                       // -game_speed:,on
extern int lbl_02945098;                        // -game_speed:,,skip_ai_factions
extern int lbl_010B506C;                        // -qa, -replay:flush
extern bool lbl_012CAA8D;                       // -replay:cont
extern int lbl_010B5064;                        // -replay:speed
extern bool lbl_012CA982;                       // -replay:build_check_from_replay
extern bool lbl_012CAA8C;                       // -replay:info
extern int lbl_012CA984;                        // -replay:stop
extern char lbl_012CA880[];                     // -replay:compare
extern bool lbl_012CAA8F;                       // -replay:show_battle_ui
extern bool lbl_012CAA8E;                       // -qa
extern bool lbl_012CAA94;                       // -ghost:save
extern bool lbl_012CAA95;                       // -ghost:play
extern char lbl_012CAA98[];                     // -ghost:file
extern bool lbl_012CAC9D;                       // -check_building_infos
extern bool lbl_012CB2FA;                       // -custom_all
extern bool lbl_012CB2FB;                       // -horde_log
extern int lbl_010B5484;                        // -vidmem
extern bool lbl_012CAFA4;                       // -dpr
extern float lbl_012CAFA8;                      // -dpr, squared
extern bool lbl_012CAFAC;                       // -desync_men
extern bool lbl_012CAFAD;                       // -ob_cam
extern bool lbl_012CAFB1;                       // -no_damage
extern bool lbl_012CAFAF;                       // -no_battle_ai
extern bool lbl_012CAFB0;                       // -no_melee_interrupts
extern bool lbl_012CAFB2;                       // -infinite_missiles
extern int lbl_010B5488;                        // -snd_hw, -no_snd_hw
extern int lbl_010B548C;                        // -snd_quality
extern bool lbl_012CAFB7;                       // -snd_profile
extern int lbl_010B5494;
extern int lbl_012CAFB8;
extern int lbl_010B5490;
extern int lbl_010B5498;
extern char lbl_012CAFBC[];                     // -ghm's tga
extern bool lbl_012CB0BC;                       // -dcdw
extern bool lbl_012CB0BD;                       // -rbm_delta
extern float lbl_012CB0C0;
extern bool lbl_012CB0C6;                       // -generate_forest_canopy
extern bool lbl_012CB0C7;                       // -generate_forest_edging
extern float lbl_010B549C;                      // -daytime, in minutes (a guess), from 60 to 600000
extern bool lbl_012CB0D4;                       // -radar_ambush
extern bool lbl_012CB0E0;                       // -disable_naval_ai
extern bool lbl_012CB0E1;                       // -test_daytypes
extern bool lbl_012CB0E5;                       // -dbmpf
extern bool lbl_012CB0E6;                       // -pdl
extern bool lbl_012CB0E7;                       // -snd_missing
extern unsigned char lbl_012CB0E8[];            // -snd_missing's file
extern int lbl_010B54A4;                        // -lbc_peasant_count
extern char lbl_010B54A8[];                     // -capture_audio:dir=, "capture_wav" at first
extern float lbl_010B55A8;                      // -tweak_pwdm
extern bool lbl_012CB1EC;                       // -hide_non_release_info
extern bool lbl_012CB1EE;                       // -dont_correct_regions
extern bool lbl_012CB1E9;                       // -auto_reload
extern bool lbl_012CB1EA;                       // -debug_fire_damage
extern bool lbl_012CB2F4;                       // -all_can_swim
extern bool lbl_012CB2F5;                       // -disable_horde
extern bool lbl_012CB2F7;                       // -no_unit_cull
extern bool lbl_012CB2F8;                       // -use_campaign_costs_in_custom

extern char lbl_012CACA0[];                     // the path the functions below return
}

// The campaign map position -force_campaign_battle_ai_defend gives.
struct CmdLinePosition {
    CmdLinePosition() : m_x(0), m_y(0) {}

    int m_x;
    int m_y;
};

static CmdLinePosition s_ai_defend_position;   // 0x012CA5D0
DiagnosticSettings g_diagnostic_settings;       // 0x012CA680
NetSettings g_net_settings;                     // 0x012CA700
GameSpyLaunch GameSpyLaunch::s_launch;

template <class T> inline T maximum(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <class T> inline T minimum(const T& a, const T& b)
{
    return a < b ? a : b;
}

template <class T> inline T clamp(const T& value, const T& low, const T& high)
{
    return minimum(maximum(value, low), high);
}

// The faction -faction gives.
inline void set_cmd_line_faction(int faction)
{
    lbl_0126AE6C = faction;
}

// Reports an error in the command line (once a run).
#define CMD_LINE_ERROR(message)                                 \
    if (!lbl_012CA5B8) {                                        \
        sprintf(lbl_010B3928, "Generic Error:\n\n%s", message); \
        lbl_012CA5B8 = true;                                    \
        fn_00F40060(0);                                         \
        fn_00F401D0("%s", lbl_010B3928);                        \
    }

// Reads a number into value (0 when there are no digits), and returns what follows it.
extern "C" unsigned char* fn_00408CDE(unsigned char* text, int* value)
{
    *value = 0;
    while (text != 0 && *text >= '0' && *text <= '9') {
        *value = *value * 10 + *text - '0';
        text++;
    }
    return text;
}

// Reads the first text in double quotes into string (or "", when there's none).
extern "C" void fn_00408D28(unsigned char* text, UniString* string)
{
    while (*text != '"' && *text != 0)
        text++;
    if (*text == '"') {
        text++;
        unsigned char* end = text;
        unsigned short length = 0;
        while (*end != '"' && *end != 0) {
            end++;
            length++;
        }
        *string = UniString((const char*)text, length);
    } else {
        *string = UniString();
    }
}

// Reads the command line's options (fn_00F3A500 has split it into them). False when one is wrong, once the error has
// been reported.
extern "C" bool fn_00408DCF()
{
    unsigned char* opt;

    if (fn_00F3A770("sw"))
        lbl_012CA75F = true;
    if (fn_00F3A770("ne"))
        lbl_012CA75D = true;
    if (fn_00F3A770("ae"))
        lbl_012CA75E = true;
    if (fn_00F3A770("no_tnl"))
        lbl_012CA760 = true;
    if (fn_00F3A770("nm"))
        lbl_012CA761 = true;
    if (fn_00F3A770("na"))
        lbl_012CAB99 = true;
    if (fn_00F3A770("force_default_hz"))
        lbl_012CA763 = true;
    if (fn_00F3A770("blank_ui_caches"))
        lbl_02926E68 = true;
    if (fn_00F3A770("report_missing_images"))
        lbl_012CB2F9 = true;
    if ((opt = fn_00F3A770("util")) != 0) {
        lbl_012CB0E2 = true;
        if (*opt == '\0') {
            CMD_LINE_ERROR("Must supply options for utility mode.");
            return false;
        }
        if (*opt == '"')
            opt++;
        unsigned char options[0x800];
        strcpy((char*)options, (char*)opt);
        if (options[strlen((char*)options) - 1] == '"')
            options[strlen((char*)options) - 1] = '\0';
        char* option = strtok((char*)options, ",");
        while (option != 0) {
            if (strcmp(option, "validate_models") == 0)
                lbl_012CB1F0 = true;
            if (strcmp(option, "unit_models") == 0)
                lbl_012CB1EF = true;
            if (strcmp(option, "encrypt") == 0)
                lbl_012CB1EB = true;
            if (strcmp(option, "animdb") == 0)
                lbl_012CB0E3 = true;
            else if (strcmp(option, "sound") == 0)
                lbl_012CB0E4 = true;
            option = strtok(0, ",");
        }
    }
    if (fn_00F3A770("fpex"))
        lbl_012CB0D8 = 1;
    if (fn_00F3A770("fperr"))
        lbl_012CB0D8 = 2;
    if (fn_00F3A770("fplog"))
        lbl_012CB0D8 = 3;
    if (fn_00F3A770("enable_editor"))
        lbl_012CAA90 = true;
    if (fn_00F3A770("disr"))
        lbl_012CA765 = true;
    if (fn_00F3A770("bookmark"))
        lbl_012CAFB4 = true;
    if ((opt = fn_00F3A770("mod")) != 0) {
        lbl_012CAC9E = false;
        if (*opt != '\0' && *opt != ' ') {
            int length = 0;
            while (opt[length] != '-' && opt[length] != ' ' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strcpy(lbl_010B5078, (char*)opt);
            lbl_012CAC9E = true;
        } else {
            strcpy(lbl_010B5178, "imperial_campaign");
            strcpy(lbl_010B5078, ".");
        }
    }
    if ((opt = fn_00F3A770("default_campaign")) != 0 && *opt != '\0' && *opt != ' ') {
        int length = 0;
        while (opt[length] != '-' && opt[length] != ' ' && opt[length] != '\t' && opt[length] != '\0')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_010B5178, (char*)opt, length);
        lbl_010B5178[length] = '\0';
        lbl_012CAC9E = true;
    }
    if (fn_00F3A770("multirun"))
        lbl_012CB0C4 = true;
    if ((opt = fn_00F3A770("battle")) != 0) {
        lbl_012CA76E = true;
        if (*opt != '\0') {
            int length = 0;
            if (*opt == '"')
                opt++;
            while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B4E60, (char*)opt, length);
            lbl_010B4E60[length] = '\0';
        }
    }
    if ((opt = fn_00F3A770("strat")) != 0) {
        lbl_012CA770 = true;
        if (*opt != '\0') {
            int length = 0;
            if (*opt == '"')
                opt++;
            while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B4E60, (char*)opt, length);
            lbl_010B4E60[length] = '\0';
        }
    }
    if ((opt = fn_00F3A770("battle_ed")) != 0) {
        lbl_012CA775 = true;
        if (*opt != '\0') {
            int length = 0;
            if (*opt == '"')
                opt++;
            while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B4E60, (char*)opt, length);
            lbl_010B4E60[length] = '\0';
        }
    }

    // The options the GameSpy Arcade launcher gives.
    UniString gs_chatname;
    UniString gs_game_password;
    int gs_room = 0;
    if ((opt = fn_00F3A770("gs_chat_name")) != 0)
        fn_00408D28(opt, &gs_chatname);
    if ((opt = fn_00F3A770("gs_group_room")) != 0) {
        opt++;
        opt = fn_00408CDE(opt, &gs_room);
    }
    if ((opt = fn_00F3A770("gs_game_password")) != 0)
        fn_00408D28(opt, &gs_game_password);
    UniString gs_email;
    UniString gs_profilenick;
    UniString gs_uniquenick;
    UniString gs_password;
    UniString gs_namespaceid;
    if ((opt = fn_00F3A770("gs_email")) != 0) {
        fn_00408D28(opt, &gs_email);
        if ((opt = fn_00F3A770("gs_profilenick")) != 0) {
            fn_00408D28(opt, &gs_profilenick);
            if ((opt = fn_00F3A770("gs_uniquenick")) != 0) {
                fn_00408D28(opt, &gs_uniquenick);
                if ((opt = fn_00F3A770("gs_password")) != 0) {
                    fn_00408D28(opt, &gs_password);
                    if ((opt = fn_00F3A770("gs_namespaceid")) != 0) {
                        fn_00408D28(opt, &gs_namespaceid);
                        gs_chatname = gs_email + "\\" + gs_profilenick + "\\" + gs_uniquenick + "\\" + gs_password + "\\" +
                                    gs_namespaceid;
                    }
                }
            }
        }
    }
    bool gs_lan = fn_00F3A770("gs_lan") != 0;
    if ((opt = fn_00F3A770("gs_host")) != 0)
        GameSpyLaunch::s_launch = GameSpyLaunch(gs_room, gs_chatname, gs_game_password, gs_lan);
    if ((opt = fn_00F3A770("gs_join")) != 0) {
        NetAddress host;
        int ip0 = 0;
        int ip1 = 0;
        int ip2 = 0;
        int ip3 = 0;
        bool complete = false;
        opt = fn_00408CDE(opt, &ip0);
        if (*opt != '\0' && *opt == '.') {
            opt++;
            opt = fn_00408CDE(opt, &ip1);
            if (*opt != '\0' && *opt == '.') {
                opt++;
                opt = fn_00408CDE(opt, &ip2);
                if (*opt != '\0' && *opt == '.') {
                    opt++;
                    opt = fn_00408CDE(opt, &ip3);
                    complete = true;
                    host.set(ip0, ip1, ip2, ip3, 0);
                }
            }
        }
        if (complete)
            GameSpyLaunch::s_launch = GameSpyLaunch(host, gs_room, gs_chatname, gs_game_password, gs_lan);
    }

    if (fn_00F3A770("sprite_script"))
        lbl_012CA777 = true;
    if (fn_00F3A770("show_err"))
        lbl_012CB1F1 = true;
    if (fn_00F3A770("no_exit_error"))
        lbl_012CB1F1 = false;
    if ((opt = fn_00F3A770("movie_cam")) != 0) {
        lbl_012CA764 = true;
        lbl_012CAFB3 = true;
        lbl_012CAFB6 = true;
        lbl_012CA981 = false;
        lbl_010B5068 = false;
        while (*opt != '\0' && *opt != ' ') {
            if (*opt == ':')
                opt++;
            if (strncmp((char*)opt, "capture_video", 13) == 0) {
                lbl_012CADA2 = true;
                lbl_012CA988 = true;
                opt += 13;
            }
            if (strncmp((char*)opt, "capture_audio", 13) == 0) {
                lbl_012CB1E8 = true;
                lbl_012CA988 = true;
                opt += 13;
            }
        }
    }
    if (fn_00F3A770("fbc"))
        lbl_012CAFB3 = true;
    if ((opt = fn_00F3A770("ai")) != 0) {
        lbl_012CA766 = true;
        lbl_012CA768 = 0;
        if (strncmp((char*)opt, "enter_battles", 13) == 0) {
            opt += 13;
            if (*opt == '=') {
                opt++;
                unsigned char* start = opt;
                while (*opt != ':' && *opt != ' ' && *opt != '\t' && *opt != '\0')
                    opt++;
                int battles = atoi((char*)start);
                if (battles >= 0)
                    lbl_012CA768 = battles;
            } else {
                lbl_012CA768 = 1;
            }
        }
    }
    if ((opt = fn_00F3A770("unlock_campaign")) != 0)
        lbl_012CB1ED = true;
    if (fn_00F3A770("output_con_cmds"))
        g_console.write_commands("documentation/console_commands.txt");

    // The release build stops here: the rest is compiled, but never reached.
    return true;

    // -capture_tga's options (though nothing looks for -capture_tga).
    while (*opt != '\0' && *opt != ' ') {
        if (*opt == ':') {
            opt++;
        } else if (strncmp((char*)opt, "fps", 3) == 0) {
            opt += 3;
            if (*opt == '=') {
                opt++;
                unsigned char* value = opt;
                while (*opt != ':' && *opt != ' ' && *opt != '\t' && *opt != '\0')
                    opt++;
                lbl_010B537C = atoi((char*)value);
            } else {
                CMD_LINE_ERROR("Must supply fps as 'fps=N'.");
                return false;
            }
        } else if (strncmp((char*)opt, "dir", 3) == 0) {
            opt += 3;
            if (*opt != '=') {
                CMD_LINE_ERROR("Must supply a target dir as dir=dirname.");
                return false;
            }
            opt++;
            int length = 0;
            bool quoted = false;
            if (*opt == '"') {
                quoted = true;
                opt++;
            }
            while ((quoted || opt[length] != ':') && opt[length] != '"' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B5380, (char*)opt, length);
            lbl_010B5380[length] = '\0';
            opt += length;
            opt++;
        } else {
            CMD_LINE_ERROR("Capture tga options: -capture_tga:fps=N:dir=dirname.  -capture_tga defaults to "
                           "-capture_tga:fps=30:dir=capture_tga");
            return false;
        }
    }
    if (fn_00F3A770("sp"))
        lbl_012CA776 = true;
    if (fn_00F3A770("sprn"))
        lbl_012CA878 = true;
    if (fn_00F3A770("hires_radar"))
        lbl_012CB0C8 = true;
    if (fn_00F3A770("load_after_save"))
        lbl_012CB0C9 = true;
    if ((opt = fn_00F3A770("incremental_autosave")) != 0) {
        lbl_012CB0CA = true;
        if (*opt == ':')
            opt++;
        lbl_012CB0CC = atoi((char*)opt);
    }
    if ((opt = fn_00F3A770("end_of_turn_autosave")) != 0)
        lbl_012CB0CB = true;
    if ((opt = fn_00F3A770("save_game_fix_algorithm")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_012CB0D0 = atoi((char*)opt);
    }
    if (fn_00F3A770("cheat"))
        lbl_012CB0C5 = true;
    if ((opt = fn_00F3A770("shortcuts")) != 0)
        lbl_012CA754 = opt;
    if ((opt = fn_00F3A770("ed_dir")) != 0)
        lbl_012CA758 = opt;
    if ((opt = fn_00F3A770("mem_force")) != 0) {
        lbl_012CA75C = true;
        int size;
        fn_00408CDE(opt, &size);
        lbl_010B4E5C = size;
    }
    if (fn_00F3A770("auto_quit"))
        lbl_012CAC9C = true;
    if (fn_00F3A770("pr"))
        lbl_012CADA0 = true;
    if (fn_00F3A770("minimal_ui"))
        lbl_0126AE34 = false;
    if (fn_00F3A770("showtime"))
        lbl_012CAFB5 = true;
    if (fn_00F3A770("tv_banners"))
        lbl_012CAFB6 = true;
    if (fn_00F3A770("enable_editor"))
        lbl_012CAA90 = true;
    if ((opt = fn_00F3A770("force_campaign_battle_ai_defend")) != 0) {
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'force_campaign_battle_ai_defend' Must supply a campaign map "
                           "tile.");
            return false;
        }
        int x = 0;
        int y = 0;
        sscanf((char*)opt, "%d,%d", &x, &y);
        s_ai_defend_position.m_x = x;
        s_ai_defend_position.m_y = y;
        lbl_012CAA91 = true;
    }
    if ((opt = fn_00F3A770("battle_ai_disable_outflanking")) != 0)
        lbl_012CAA92 = true;
    if ((opt = fn_00F3A770("battle_ai_force_outflanking")) != 0)
        lbl_012CAA93 = true;
    if ((opt = fn_00F3A770("popularity_land_percentage")) != 0)
        lbl_010B5070 = atoi((char*)opt);
    if ((opt = fn_00F3A770("dm")) != 0) {
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'dm' Must supply a mount model type name when using view mode "
                           "with mounts.");
            return false;
        }
        int length = 0;
        while (opt[length] != '-' && opt[length] != '\t' && opt[length] != '\0' && opt[length] != ',')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters for mount names.");
            return false;
        }
        strncpy(lbl_012CA778, (char*)opt, length);
        lbl_012CA778[length] = '\0';
    }
    if ((opt = fn_00F3A770("df")) != 0)
        lbl_010B4F60 = fn_00E3C320((char*)opt);
    if ((opt = fn_00F3A770("position_scale")) != 0)
        lbl_010B5378 = atof((char*)opt);
    if (fn_00F3A770("engine"))
        lbl_012CA879 = true;
    if (fn_00F3A770("show_grid"))
        lbl_012CAFAE = true;
    if ((opt = fn_00F3A770("custom")) != 0) {
        lbl_012CA76F = true;
        if (*opt != '\0') {
            int length = 0;
            if (*opt == '"')
                opt++;
            while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B4E60, (char*)opt, length);
            lbl_010B4E60[length] = '\0';
        }
    }
    if (fn_00F3A770("engine_shadow")) {
        lbl_012CA879 = true;
        lbl_012CA87A = true;
    }
    if (fn_00F3A770("nc"))
        lbl_012CA762 = true;
    if (fn_00F3A770("use_rtm_cam"))
        lbl_012CA764 = true;
    if ((opt = fn_00F3A770("key")) != 0) {
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'key' must be given a string for which to generate a key.");
            return false;
        }
        int length = 0;
        while (opt[length] != '-' && opt[length] != ' ' && opt[length] != '\t' && opt[length] != '\0' &&
               opt[length] != ',')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a key longer than 256 characters.");
            return false;
        }
        char text[0x100];
        strncpy(text, (char*)opt, length);
        text[length] = '\0';
        char* key = fn_004255B6(text);
        fn_00425622(key);
    }
    if ((opt = fn_00F3A770("view")) != 0) {
        lbl_012CA76D = true;
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'view' Must supply a model type name when using view mode.");
            return false;
        }
        int length = 0;
        while (opt[length] != '-' && opt[length] != ' ' && opt[length] != '\t' && opt[length] != '\0' &&
               opt[length] != ',')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_010B4E60, (char*)opt, length);
        lbl_010B4E60[length] = '\0';
        lbl_010B5074 = atoi((char*)opt + length);
    }
    if ((opt = fn_00F3A770("ignore_win_or_defeat")) != 0)
        lbl_012CA767 = true;
    if ((opt = fn_00F3A770("battle_arcade_mode")) != 0)
        lbl_012CA76C = true;
    if (fn_00F3A770("lan"))
        lbl_012CA87B = true;
    if ((opt = fn_00F3A770("prefs")) != 0 && *opt != '\0') {
        int length = 0;
        if (*opt == '"')
            opt++;
        while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_012CAB9C, (char*)opt, length);
        lbl_012CAB9C[length] = '\0';
        lbl_012CAB9A = length != 0;
    }
    if ((opt = fn_00F3A770("overview")) != 0) {
        lbl_012CA771 = true;
        if (*opt == '\0') {
            CMD_LINE_ERROR("Must supply an overview map name when bypassing straight to overview from command line.");
            return false;
        }
        int length = 0;
        if (*opt == '"')
            opt++;
        while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_010B4E60, (char*)opt, length);
        lbl_010B4E60[length] = '\0';
    }
    if ((opt = fn_00F3A770("fmv")) != 0) {
        lbl_012CA772 = true;
        if (*opt == '\0') {
            CMD_LINE_ERROR("Must supply a strategy map name when bypassing straight to fmv from command line.");
            return false;
        }
        int length = 0;
        if (*opt == '"')
            opt++;
        while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_010B4E60, (char*)opt, length);
        lbl_010B4E60[length] = '\0';
    }
    if ((opt = fn_00F3A770("rtm")) != 0) {
        lbl_012CA87C = true;
        if (*opt == '\0') {
            CMD_LINE_ERROR("Must supply a rtm file name when bypassing straight to rtm from command line.");
            return false;
        }
        int length = 0;
        if (*opt == '"')
            opt++;
        while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
            return false;
        }
        strncpy(lbl_010B4E60, (char*)opt, length);
        lbl_010B4E60[length] = '\0';
    }
    if (fn_00F3A770("create_sprite_script"))
        lbl_012CA87D = true;
    if (fn_00F3A770("output_skeleton_speeds"))
        lbl_012CA87E = true;
    if ((opt = fn_00F3A770("strat_ed")) != 0) {
        lbl_012CA774 = true;
        if (*opt != '\0') {
            int length = 0;
            if (*opt == '"')
                opt++;
            while (opt[length] != '-' && opt[length] != '"' && opt[length] != '\t' && opt[length] != '\0')
                length++;
            if (length >= 0x100) {
                CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                return false;
            }
            strncpy(lbl_010B4E60, (char*)opt, length);
            lbl_010B4E60[length] = '\0';
        }
    }
    if (fn_00F3A770("menu"))
        lbl_012CA773 = true;
    if ((opt = fn_00F3A770("screen_res")) != 0) {
        if (*opt == '\0') {
            CMD_LINE_ERROR("Command-line option 'screen_res' must be given comma-seperated screen width and "
                           "height in pixels.");
            return false;
        }
        unsigned char* xres_text = opt;
        if (*opt == '"')
            opt++;
        while (*opt != ',' && *opt != '"' && *opt != '\t' && *opt != '\0')
            opt++;
        if (*opt == ',')
            opt++;
        unsigned char* yres_text = opt;
        int xres = atoi((char*)xres_text);
        int yres = atoi((char*)yres_text);
        if (xres > 0 && yres > 0) {
            fn_00D7FD25(xres, yres);
            fn_00D7FD94();
        } else {
            CMD_LINE_ERROR("Command-line option 'screen_res' contained space, zero or negative number.");
            return false;
        }
    }
    if ((opt = fn_00F3A770("hm")) != 0) {
        if (strcmp((char*)opt, "playable") == 0)
            lbl_02945090 = 2;
        else if (strcmp((char*)opt, "all") == 0)
            lbl_02945090 = 1;
    }
    if ((opt = fn_00F3A770("faction")) != 0 && strcmp((char*)opt, "") != 0)
        set_cmd_line_faction(fn_00E3C320((char*)opt));
    if ((opt = fn_00F3A770("game_speed")) != 0) {
        int speed = atoi((char*)opt);
        lbl_0126AE2D = speed;
        while (*opt != ',' && *opt != ' ' && *opt != '\t' && *opt != '\0')
            opt++;
        if (*opt == ',')
            opt++;
        if (strspn((char*)opt, "on"))
            lbl_02945094 = true;
        while (*opt != ',' && *opt != ' ' && *opt != '\t' && *opt != '\0')
            opt++;
        if (*opt == ',')
            opt++;
        if (strspn((char*)opt, "skip_ai_factions"))
            lbl_02945098 = 1;
    }
    if (fn_00F3A770("display_off"))
        lbl_012CAB98 = true;
    if (fn_00F3A770("qa")) {
        lbl_010B506C = 1;
        lbl_012CA981 = true;
    }
    if ((opt = fn_00F3A770("replay")) != 0) {
        lbl_012CA87F = true;
        lbl_010B5068 = false;
        lbl_012CA980 = false;
        while (*opt != '\0' && *opt != ' ') {
            if (*opt == ':')
                opt++;
            if (strncmp((char*)opt, "save", 4) == 0) {
                lbl_012CA980 = true;
                opt += 4;
            } else if (strncmp((char*)opt, "play", 4) == 0) {
                lbl_012CA980 = false;
                opt += 4;
            } else if (strncmp((char*)opt, "cont", 4) == 0) {
                lbl_012CAA8D = true;
                opt += 4;
            } else if (strncmp((char*)opt, "speed", 5) == 0) {
                opt += 5;
                if (*opt == '=') {
                    opt++;
                    unsigned char* value = opt;
                    while (*opt != ':' && *opt != ' ' && *opt != '\t' && *opt != '\0')
                        opt++;
                    lbl_010B5064 = atoi((char*)value);
                } else {
                    CMD_LINE_ERROR("Must supply speed factor as 'speed=N'.");
                    return false;
                }
                if (lbl_010B5064 == 0)
                    lbl_010B5064 = 1;
            } else if (strncmp((char*)opt, "check", 5) == 0) {
                lbl_012CA981 = true;
                lbl_010B5068 = true;
                opt += 5;
            } else if (strncmp((char*)opt, "build_check_from_replay", 23) == 0) {
                lbl_012CA982 = true;
                opt += 23;
            } else if (strncmp((char*)opt, "nocheck", 7) == 0) {
                lbl_012CA981 = false;
                lbl_010B5068 = false;
                opt += 7;
            } else if (strncmp((char*)opt, "info", 4) == 0) {
                lbl_012CAA8C = true;
                opt += 4;
            } else if (strncmp((char*)opt, "stop", 4) == 0) {
                opt += 4;
                if (*opt == '=') {
                    opt++;
                    unsigned char* value = opt;
                    while (*opt != ':' && *opt != ' ' && *opt != '\t' && *opt != '\0')
                        opt++;
                    lbl_012CA984 = atoi((char*)value);
                } else {
                    CMD_LINE_ERROR("Must supply stop tick as 'stop=N'.");
                    return false;
                }
            } else if (strncmp((char*)opt, "flush", 5) == 0) {
                opt += 5;
                if (*opt == '=') {
                    opt++;
                    unsigned char* value = opt;
                    while (*opt != ':' && *opt != ' ' && *opt != '\t' && *opt != '\0')
                        opt++;
                    lbl_010B506C = atoi((char*)value);
                } else {
                    lbl_010B506C = 1;
                }
            } else if (strncmp((char*)opt, "capture_and_quit", 16) == 0) {
                opt += 16;
                lbl_012CA988 = true;
            } else if (strncmp((char*)opt, "file", 4) == 0) {
                opt += 4;
                if (*opt != '=') {
                    CMD_LINE_ERROR("Must supply a replay file name as file=\"filename\".");
                    return false;
                }
                opt++;
                if (*opt != '"') {
                    CMD_LINE_ERROR("Must supply a replay file name in double quotes.");
                    return false;
                }
                opt++;
                int length = 0;
                while (opt[length] != '"' && opt[length] != '\0')
                    length++;
                if (length >= 0x100) {
                    CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                    return false;
                }
                if (opt[length] != '"') {
                    CMD_LINE_ERROR("Must supply a replay file name in double quotes.");
                    return false;
                }
                strncpy(lbl_010B4F64, (char*)opt, length);
                lbl_010B4F64[length] = '\0';
                opt += length;
                opt++;
            } else if (strncmp((char*)opt, "cam", 3) == 0) {
                opt += 3;
                if (*opt != '=') {
                    CMD_LINE_ERROR("Must supply a camera file name as file=\"filename\".");
                    return false;
                }
                opt++;
                if (*opt != '"') {
                    CMD_LINE_ERROR("Must supply a camera file name in double quotes.");
                    return false;
                }
                opt++;
                int length = 0;
                while (opt[length] != '"' && opt[length] != '\0')
                    length++;
                if (length >= 0x100) {
                    CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                    return false;
                }
                if (opt[length] != '"') {
                    CMD_LINE_ERROR("Must supply a replay file name in double quotes.");
                    return false;
                }
                strncpy(lbl_012CA98C, (char*)opt, length);
                lbl_012CA98C[length] = '\0';
                opt += length;
                opt++;
            } else if (strncmp((char*)opt, "compare", 7) == 0) {
                opt += 7;
                if (*opt != '=') {
                    CMD_LINE_ERROR("Must supply a replay comparison file name as compare=\"filename\".");
                    return false;
                }
                opt++;
                if (*opt != '"') {
                    CMD_LINE_ERROR("Must supply a replay comparison file name in double quotes.");
                    return false;
                }
                opt++;
                int length = 0;
                while (opt[length] != '"' && opt[length] != '\0')
                    length++;
                if (length >= 0x100) {
                    CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                    return false;
                }
                if (opt[length] != '"') {
                    CMD_LINE_ERROR("Must supply a replay comparision file name in double quotes.");
                    return false;
                }
                strncpy(lbl_012CA880, (char*)opt, length);
                lbl_012CA880[length] = '\0';
                opt += length;
                opt++;
            } else if (strncmp((char*)opt, "show_battle_ui", 14) == 0) {
                lbl_012CAA8F = true;
                opt += 14;
            } else {
                CMD_LINE_ERROR("Replay options: -replay:save:play:speed=N:check:nocheck:stop=N:file=filename:"
                               "show_battle_ui.  -replay defaults to -replay:play:speed=1:check:file=\"network.rpy\":"
                               "stop=0  (never stops)");
                return false;
            }
        }
    }
    {
        bool qa = fn_00F3A770("qa") != 0;
        lbl_012CAA8E = qa;
        if (qa || (opt = fn_00F3A770("diag_warn")) != 0) {
            g_diagnostic_settings.m_enabled = true;
            g_diagnostic_settings.m_warn = true;
        }
        if ((opt = fn_00F3A770("checksum")) != 0) {
            bool added = false;
            int checksum;
            if (*opt == '+') {
                added = true;
                opt++;
            }
            if (*opt == '+' && added) {
                checksum = 0x1ffeff;
                opt++;
            } else {
                opt = fn_00408CDE(opt, &checksum);
                if (added)
                    checksum |= lbl_00FE4728;
            }
            g_diagnostic_settings.m_enabled = true;
            g_diagnostic_settings.set_checksum(checksum);
        }
        if ((opt = fn_00F3A770("display_rand")) != 0) {
            int value;
            opt = fn_00408CDE(opt, &value);
            g_diagnostic_settings.m_enabled = true;
            g_diagnostic_settings.set_display_rand(value);
        }
        if ((opt = fn_00F3A770("log")) != 0 || qa) {
            int mask = lbl_00FE4728;
            int ticks_mask = 0;
            int start = 0;
            int last = 0;
            bool have_start_mask = false;
            int start_mask = 0;
            if (opt != 0) {
                if (*opt == '=')
                    opt++;
                if (*opt == '+' || (*opt >= '0' && *opt <= '9')) {
                    bool added = false;
                    if (*opt == '+') {
                        added = true;
                        opt++;
                    }
                    if (*opt == '+' && added) {
                        mask = 0x1ffeff;
                        opt++;
                    } else {
                        opt = fn_00408CDE(opt, &mask);
                        if (added)
                            mask |= lbl_00FE4728;
                    }
                }
                while (*opt != '\0' && *opt != ' ') {
                    if (*opt == ':')
                        opt++;
                    if (strncmp((char*)opt, "ticks", 5) == 0) {
                        opt += 5;
                        ticks_mask = lbl_00FE4728;
                        if (*opt == '=') {
                            opt++;
                            bool added = false;
                            if (*opt == '+') {
                                added = true;
                                opt++;
                            }
                            if (*opt == '+' && added) {
                                ticks_mask = 0x1ffeff;
                                opt++;
                            } else {
                                opt = fn_00408CDE(opt, &ticks_mask);
                            }
                            if (*opt == ',') {
                                opt++;
                                opt = fn_00408CDE(opt, &start);
                                if (*opt == ',') {
                                    opt++;
                                    opt = fn_00408CDE(opt, &last);
                                }
                            }
                            if (added)
                                ticks_mask |= lbl_00FE4728;
                        }
                    } else if (strncmp((char*)opt, "first_tick", 10) == 0) {
                        opt += 11;
                        bool added = false;
                        if (*opt == '+') {
                            added = true;
                            opt++;
                        }
                        if (*opt == '+' && added) {
                            start_mask = 0x1ffeff;
                            opt++;
                        } else {
                            opt = fn_00408CDE(opt, &start_mask);
                            if (added)
                                start_mask |= lbl_00FE4728;
                        }
                        have_start_mask = true;
                        opt++;
                    } else {
                        CMD_LINE_ERROR("'log' options incorrect: -log=MASK<:ticks=MASK<,start,end>><:first_tick=MASK>");
                        return false;
                    }
                }
            }
            g_diagnostic_settings.m_enabled = true;
            g_diagnostic_settings.m_log = true;
            g_diagnostic_settings.set_log(mask, ticks_mask, start, last, have_start_mask, start_mask);
        }
        {
            bool log = false;
            bool local = false;
            int mask = lbl_010216E8;
            int resend = 6;
            bool use_broadcast = true;
            bool mt_off = false;
            if (qa)
                log = true;
            if ((opt = fn_00F3A770("nw")) != 0) {
                while (*opt != '\0' && *opt != ' ') {
                    if (*opt == ':') {
                        opt++;
                    } else if (strncmp((char*)opt, "log", 3) == 0) {
                        opt += 3;
                        if (*opt == '=') {
                            opt++;
                            bool added = false;
                            if (*opt == '+') {
                                added = true;
                                opt++;
                            }
                            opt = fn_00408CDE(opt, &mask);
                            if (added)
                                mask |= lbl_010216E8;
                        } else if (*opt != '\0' && *opt != ' ') {
                            opt++;
                        }
                    } else if (strncmp((char*)opt, "bcast_on", 8) == 0) {
                        opt += 8;
                        use_broadcast = true;
                    } else if (strncmp((char*)opt, "bcast_off", 9) == 0) {
                        opt += 9;
                        use_broadcast = false;
                    } else if (strncmp((char*)opt, "mt_on", 5) == 0) {
                        opt += 5;
                        mt_off = false;
                    } else if (strncmp((char*)opt, "mt_off", 6) == 0) {
                        opt += 6;
                        mt_off = true;
                    } else if (strncmp((char*)opt, "local", 5) == 0) {
                        opt += 5;
                        local = true;
                    } else if (strncmp((char*)opt, "resend", 6) == 0) {
                        opt += 6;
                        if (*opt == '=') {
                            opt++;
                            opt = fn_00408CDE(opt, &resend);
                        }
                    } else {
                        CMD_LINE_ERROR("'nw' options incorrect: -nw<:log=MASK><:resend=N><:local>");
                        return false;
                    }
                }
            }
            g_net_settings.set(log, local, mask, resend, use_broadcast, mt_off, g_diagnostic_settings);
        }
    }
    if ((opt = fn_00F3A770("ghost")) != 0) {
        strcpy(lbl_012CAA98, "input_save.svg");
        while (*opt != '\0' && *opt != ' ') {
            if (*opt == ':')
                opt++;
            if (strncmp((char*)opt, "save", 4) == 0) {
                if (!lbl_012CAA95)
                    lbl_012CAA94 = true;
                opt += 4;
            } else if (strncmp((char*)opt, "play", 4) == 0) {
                if (!lbl_012CAA94)
                    lbl_012CAA95 = true;
                opt += 4;
            } else if (strncmp((char*)opt, "file", 4) == 0) {
                opt += 4;
                if (*opt != '=') {
                    CMD_LINE_ERROR("Must supply a ghost file name as file=\"filename\".");
                    return false;
                }
                opt++;
                if (*opt != '"') {
                    CMD_LINE_ERROR("Must supply a ghost file name in double quotes.");
                    return false;
                }
                opt++;
                int length = 0;
                while (opt[length] != '"' && opt[length] != '\0')
                    length++;
                if (length >= 0x100) {
                    CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                    return false;
                }
                if (opt[length] != '"') {
                    CMD_LINE_ERROR("Must supply a ghost file name in double quotes.");
                    return false;
                }
                strncpy(lbl_012CAA98, (char*)opt, length);
                lbl_012CAA98[length] = '\0';
                opt += length;
                opt++;
            } else {
                CMD_LINE_ERROR("Ghost options: -ghost:save:play:file=filename  -ghost defaults to "
                               "-ghost:play:file=\"input_save.svg\"");
                return false;
            }
        }
    }
    if (fn_00F3A770("check_building_infos"))
        lbl_012CAC9D = true;
    if (fn_00F3A770("custom_all"))
        lbl_012CB2FA = true;
    if (fn_00F3A770("horde_log"))
        lbl_012CB2FB = true;
    if ((opt = fn_00F3A770("host")) != 0) {
        if (*opt == ':')
            opt++;
        bool ok = true;
        bool battle = true;
        if (strncmp((char*)opt, "battle", 6) == 0) {
            battle = true;
            opt += 6;
        } else if (strncmp((char*)opt, "strat", 5) == 0) {
            battle = false;
            opt += 5;
        } else {
            ok = false;
        }
        if (ok && *opt == ':') {
            opt++;
            unsigned int length = 0;
            while (opt[length] != '\0' && opt[length] != ':')
                length++;
            if (length > 0) {
                strncpy(lbl_012CAEA4, (char*)opt, length);
                lbl_012CAEA4[length] = '\0';
                strcpy(lbl_012CADA4, "default name");
                lbl_012CADA3 = true;
                lbl_010B5480 = battle;
                opt += length;
                if (*opt == ':') {
                    opt++;
                    unsigned int name_length = 0;
                    while (opt[name_length] != '\0' && opt[name_length] != ':')
                        name_length++;
                    if (name_length > 0) {
                        strncpy(lbl_012CADA4, (char*)opt, name_length);
                        lbl_012CADA4[name_length] = '\0';
                    }
                }
            }
        }
    }
    if ((opt = fn_00F3A770("vidmem")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_010B5484 = atoi((char*)opt);
    }
    if ((opt = fn_00F3A770("dpr")) != 0) {
        if (*opt == ':')
            opt++;
        float range = atof((char*)opt);
        lbl_012CAFA4 = true;
        lbl_012CAFA8 = range * range;
    }
    if (fn_00F3A770("desync_men"))
        lbl_012CAFAC = true;
    if (fn_00F3A770("ob_cam"))
        lbl_012CAFAD = true;
    if (fn_00F3A770("no_damage"))
        lbl_012CAFB1 = true;
    if (fn_00F3A770("no_battle_ai"))
        lbl_012CAFAF = true;
    if (fn_00F3A770("no_melee_interrupts"))
        lbl_012CAFB0 = true;
    if (fn_00F3A770("infinite_missiles"))
        lbl_012CAFB2 = true;
    if (fn_00F3A770("snd_hw"))
        lbl_010B5488 = 1;
    if (fn_00F3A770("no_snd_hw"))
        lbl_010B5488 = 0;
    if ((opt = fn_00F3A770("snd_quality")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_010B548C = atoi((char*)opt);
    }
    if ((opt = fn_00F3A770("snd_profile")) != 0) {
        lbl_012CAFB7 = true;
        sscanf((char*)opt, "%i:%i:%i:%i", &lbl_010B5494, &lbl_012CAFB8, &lbl_010B5490, &lbl_010B5498);
    }
    if ((opt = fn_00F3A770("ghm")) != 0) {
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'ghm' Must supply a tga filename.");
            return false;
        }
        int length = 0;
        while (opt[length] != '-' && opt[length] != '\t' && opt[length] != '\0' && opt[length] != ',')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a filename longer than 256 characters for ghm.");
            return false;
        }
        strncpy(lbl_012CAFBC, (char*)opt, length);
        lbl_012CAFBC[length] = '\0';
    }
    if (fn_00F3A770("dcdw"))
        lbl_012CB0BC = true;
    if ((opt = fn_00F3A770("rbm_delta")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_012CB0BD = true;
        lbl_012CB0C0 = atof((char*)opt);
    }
    if (fn_00F3A770("generate_forest_canopy"))
        lbl_012CB0C6 = true;
    if (fn_00F3A770("generate_forest_edging"))
        lbl_012CB0C7 = true;
    if ((opt = fn_00F3A770("daytime")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_010B549C = atof((char*)opt) * 60.0;
        lbl_010B549C = clamp(lbl_010B549C, 60.0f, 600000.0f);
    }
    if (fn_00F3A770("radar_ambush"))
        lbl_012CB0D4 = true;
    if ((opt = fn_00F3A770("stop_alloc")) != 0)
        sscanf((char*)opt, "%x", &lbl_012CB0DC);
    if ((opt = fn_00F3A770("stop_alloc_count")) != 0)
        sscanf((char*)opt, "%d", &lbl_010B54A0);
    if (fn_00F3A770("disable_naval_ai"))
        lbl_012CB0E0 = true;
    if (fn_00F3A770("test_daytypes"))
        lbl_012CB0E1 = true;
    if (fn_00F3A770("dbmpf"))
        lbl_012CB0E5 = true;
    if (fn_00F3A770("pdl"))
        lbl_012CB0E6 = true;
    if ((opt = fn_00F3A770("snd_missing")) != 0) {
        if (*opt == '\0') {
            lbl_012CB0E7 = true;
        } else {
            if (*opt == '"')
                opt++;
            if (*opt == '-') {
                lbl_012CB0E7 = true;
            } else {
                strcpy((char*)lbl_012CB0E8, (char*)opt);
                if (lbl_012CB0E8[strlen((char*)lbl_012CB0E8) - 1] == '"')
                    lbl_012CB0E8[strlen((char*)lbl_012CB0E8) - 1] = '\0';
            }
        }
    }
    if ((opt = fn_00F3A770("lbc_peasant_count")) != 0)
        lbl_010B54A4 = atoi((char*)opt);
    if ((opt = fn_00F3A770("capture_audio")) != 0) {
        lbl_012CB1E8 = true;
        while (*opt != '\0' && *opt != ' ') {
            if (*opt == ':') {
                opt++;
            } else if (strncmp((char*)opt, "dir", 3) == 0) {
                opt += 3;
                if (*opt != '=') {
                    CMD_LINE_ERROR("Must supply a target dir as dir=dirname.");
                    return false;
                }
                opt++;
                int length = 0;
                bool quoted = false;
                if (*opt == '"') {
                    quoted = true;
                    opt++;
                }
                while ((quoted || opt[length] != ':') && opt[length] != '"' && opt[length] != '\0')
                    length++;
                if (length >= 0x100) {
                    CMD_LINE_ERROR("Can't have a name longer than 256 characters.");
                    return false;
                }
                strncpy(lbl_010B54A8, (char*)opt, length);
                lbl_010B54A8[length] = '\0';
                opt += length;
                opt++;
            } else {
                CMD_LINE_ERROR("Capture audio options: -capture_audio:dir=dirname");
                return false;
            }
        }
    }
    if ((opt = fn_00F3A770("tweak_pwdm")) != 0) {
        if (*opt == ':')
            opt++;
        lbl_010B55A8 = atof((char*)opt);
    }
    if ((opt = fn_00F3A770("hide_non_release_info")) != 0)
        lbl_012CB1EC = true;
    if ((opt = fn_00F3A770("dont_correct_regions")) != 0)
        lbl_012CB1EE = true;
    if ((opt = fn_00F3A770("auto_reload")) != 0)
        lbl_012CB1E9 = true;
    if ((opt = fn_00F3A770("debug_fire_damage")) != 0)
        lbl_012CB1EA = true;
    if ((opt = fn_00F3A770("ftow")) != 0) {
        if (*opt == '\0' || *opt == ' ') {
            CMD_LINE_ERROR("Command-line option 'ftow' Must supply a tga filename.");
            return false;
        }
        int length = 0;
        while (opt[length] != '-' && opt[length] != '\t' && opt[length] != '\0' && opt[length] != ',')
            length++;
        if (length >= 0x100) {
            CMD_LINE_ERROR("Can't have a filename longer than 256 characters for ghm.");
            return false;
        }
        strncpy(lbl_012CB1F4, (char*)opt, length);
        lbl_012CB1F4[length] = '\0';
        lbl_012CB1F2 = true;
    }
    if ((opt = fn_00F3A770("all_can_swim")) != 0)
        lbl_012CB2F4 = true;
    if ((opt = fn_00F3A770("disable_horde")) != 0)
        lbl_012CB2F5 = true;
    if ((opt = fn_00F3A770("no_unit_cull")) != 0)
        lbl_012CB2F7 = true;
    if ((opt = fn_00F3A770("use_campaign_costs_in_custom")) != 0)
        lbl_012CB2F8 = true;
    return true;
}

// path in the -mod folder, when there's one and the file is there; otherwise path.
extern "C" const char* fn_0040E3C9(const char* path)
{
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/%s", lbl_010B5078, path);
        if (fn_00E2F761(lbl_012CACA0))
            return lbl_012CACA0;
    }
    return path;
}

// The -mod folder.
extern "C" const char* fn_0040E40F()
{
    return lbl_010B5078;
}

// The same as fn_0040E3C9, for a path in a UniString.
extern "C" void fn_0040E419(UniString* path)
{
    UniString modded;
    if (lbl_012CAC9E) {
        modded = uni_format("%s/%S", lbl_010B5078, *path);
        if (fn_00E2F6EF(modded)) {
            *path = modded;
            return;
        }
    }
}

// The folder name in the -mod folder, made if it isn't there (saves, preferences).
extern "C" const char* fn_0040E497(const char* name)
{
    {
        UniString path(lbl_010B5078);
        fn_00F3B090(path.chars());
    }
    sprintf(lbl_012CACA0, "%s/%s", lbl_010B5078, name);
    {
        UniString path(lbl_012CACA0);
        fn_00F3B090(path.chars());
    }
    return lbl_012CACA0;
}

// A file in the campaign's map folder: the -mod folder's, when the file is there; otherwise the imperial campaign's.
extern "C" const char* fn_0040E505(const char* file)
{
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/data/world/maps/campaign/%s/%s", lbl_010B5078, lbl_010B5178, file);
        if (fn_00E2F761(lbl_012CACA0))
            return lbl_012CACA0;
    }
    sprintf(lbl_012CACA0, "data/world/maps/campaign/imperial_campaign/%s", file);
    return lbl_012CACA0;
}

// file's path in the data folder: the -mod folder's, when the file is there.
extern "C" const char* fn_0040E567(const char* file)
{
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/data/%s", lbl_010B5078, file);
        if (fn_00E2F761(lbl_012CACA0))
            return lbl_012CACA0;
    }
    sprintf(lbl_012CACA0, "data/%s", file);
    return lbl_012CACA0;
}

// The data folder: the -mod folder's, when it has one.
extern "C" const char* fn_0040E5C4()
{
    bool use_default = true;
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/data", lbl_010B5078);
        FileFind find(lbl_012CACA0);
        if (find.found()) {
            use_default = false;
            find.close();
        }
    }
    if (use_default)
        sprintf(lbl_012CACA0, "data");
    return lbl_012CACA0;
}

// The same as fn_0040E567, for a path in a UniString.
extern "C" void fn_0040E654(UniString* path)
{
    UniString modded;
    if (lbl_012CAC9E) {
        modded = uni_format("%s/data/%S", lbl_010B5078, *path);
        if (fn_00E2F6EF(modded)) {
            *path = modded;
            return;
        }
    }
    *path = uni_format("data/%S", *path);
}

// A folder in the data folder: the -mod folder's, when it's there.
extern "C" const char* fn_0040E701(const char* folder)
{
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/data/%s", lbl_010B5078, folder);
        FileFind find(lbl_012CACA0);
        if (find.found()) {
            find.close();
            return lbl_012CACA0;
        }
    }
    sprintf(lbl_012CACA0, "data/%s", folder);
    return lbl_012CACA0;
}

// The same as fn_0040E505.
extern "C" const char* fn_0040E79C(const char* file)
{
    if (lbl_012CAC9E) {
        sprintf(lbl_012CACA0, "%s/data/world/maps/campaign/%s/%s", lbl_010B5078, lbl_010B5178, file);
        if (fn_00E2F761(lbl_012CACA0))
            return lbl_012CACA0;
    }
    sprintf(lbl_012CACA0, "data/world/maps/campaign/imperial_campaign/%s", file);
    return lbl_012CACA0;
}

NetSettings::NetSettings()
    : m_log(false), m_local(false), m_log_mask(lbl_010216E8), m_resend(6), m_broadcast(true), m_no_threads(false)
{
}

GameSpyLaunch::GameSpyLaunch() : m_launched(false), m_host(false), m_lan(false), m_room(0), m_20(5)
{
}

GameSpyLaunch::~GameSpyLaunch()
{
}

GameSpyLaunch::GameSpyLaunch(NetAddress host, int room, UniString name, UniString password, bool lan)
    : m_launched(true), m_host(false), m_lan(lan), m_name(name), m_password(password), m_address(host), m_room(room),
      m_20(0)
{
}

GameSpyLaunch::GameSpyLaunch(int room, UniString name, UniString password, bool lan)
    : m_launched(true), m_host(true), m_lan(lan), m_name(name), m_password(password), m_room(room), m_20(0)
{
}

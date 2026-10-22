// The command line's options, and the paths of files in the -mod folder (code\romans_cmd_line.cpp).
#ifndef ROMANS_CMD_LINE_H
#define ROMANS_CMD_LINE_H

#include "core_strings.h"

// What -checksum, -display_rand, -log and -diag_warn ask for (0x30 bytes).
struct DiagnosticSettings {
    DiagnosticSettings();                       // 0x00418AC9 (code\romans_game.cpp)

    void set_checksum(int mask) { m_checksum_mask = mask; }
    void set_display_rand(int value) { m_display_rand = value; }
    void set_log(int mask, int tick_mask, int first_tick, int last_tick, bool has_start_mask, int start_mask)
    {
        m_log_mask = mask;
        m_has_tick_mask = true;
        m_tick_mask = tick_mask;
        m_first_tick = first_tick;
        m_last_tick = last_tick;
        m_has_start_mask = has_start_mask;
        m_start_mask = start_mask;
    }

    bool m_enabled;                             // +0x00: any of them was given
    int m_checksum_mask;                        // +0x04: 0x2aa85 at first
    int m_display_rand;                         // +0x08: -1 at first
    bool m_log;                                 // +0x0c
    int m_log_mask;                             // +0x10
    int m_tick_mask;                            // +0x14: what is logged from first_tick to last_tick
    bool m_has_tick_mask;                       // +0x18
    int m_first_tick;                           // +0x1c
    int m_last_tick;                            // +0x20
    bool m_has_start_mask;                      // +0x24
    int m_start_mask;                           // +0x28: what is logged from the first tick (first_tick=)
    bool m_warn;                                // +0x2c: -diag_warn, or -qa
};

extern DiagnosticSettings g_diagnostic_settings;   // 0x012CA680

// What -nw asks for (0x40 bytes).
struct NetSettings {
    NetSettings();                              // 0x0040E7FE

    void set(bool log, bool local, int log_mask, int resend, bool broadcast, bool no_threads,
             const DiagnosticSettings& diagnostics)
    {
        m_log = log;
        m_local = local;
        m_log_mask = log_mask;
        m_resend = resend;
        m_broadcast = broadcast;
        m_no_threads = no_threads;
        m_diagnostics = diagnostics;
    }

    bool m_log;                                 // +0x00: -qa sets it
    bool m_local;                               // +0x01
    int m_log_mask;                             // +0x04
    int m_resend;                               // +0x08: 6 at first
    bool m_broadcast;                           // +0x0c: true at first
    bool m_no_threads;                          // +0x0d: mt_off
    DiagnosticSettings m_diagnostics;           // +0x10
};

// An IPv4 address and port (ca_libs; 0x10 bytes, a sockaddr_in).
class NetAddress {
public:
    NetAddress();                               // 0x00F4C060
    ~NetAddress();                              // 0x00F8CEB0
    bool set(int a, int b, int c, int d, int port);  // 0x00F4C140

    char m_data[0x10];
};

// The GameSpy game the command line starts in: the GameSpy Arcade launcher's -gs_ options (0x24 bytes).
struct GameSpyLaunch {
    GameSpyLaunch();                            // 0x0040E846
    ~GameSpyLaunch();                           // 0x0040E898
    GameSpyLaunch(NetAddress host, int room, UniString name, UniString password, bool lan);  // 0x0040E8C2: joins
    GameSpyLaunch(int room, UniString name, UniString password, bool lan);  // 0x0040E93D: hosts

    bool m_launched;                            // +0x00
    bool m_host;                                // +0x01
    bool m_lan;                                 // +0x02
    UniString m_name;                           // +0x04: the chat name, or the five login options joined by \s
    UniString m_password;                       // +0x08: the game's
    NetAddress m_address;                       // +0x0c: the host's, when joining
    int m_room;                                 // +0x1c: -gs_group_room
    int m_20;                                   // +0x20: 5 at first, 0 once launched

    static GameSpyLaunch s_launch;              // 0x012CA6B8
};

// The options that change how the game plays, which other units read.
extern "C" {
extern bool lbl_012CAA90;                       // -enable_editor
extern bool lbl_012CB0C4;                       // -multirun: more than one copy can run
extern bool lbl_012CA76E;                       // -battle
extern bool lbl_012CA770;                       // -strat
extern bool lbl_012CA766;                       // -ai: no faction is human
extern int lbl_012CA768;                        // -ai:enter_battles=N: battles run at the fastest speed
extern bool lbl_012CB1ED;                       // -unlock_campaign
extern int lbl_02945090;                        // the hotseat mode (-hm): 2 for playable, 1 for all, 0 for none
}

// The options the game's main function reads (code\romans.cpp).
extern "C" {
extern unsigned int lbl_012CB0DC;               // -stop_alloc
extern int lbl_010B54A0;                        // -stop_alloc_count
extern bool lbl_012CA75C;                       // -mem_force
extern int lbl_010B4E5C;                        // -mem_force's size
extern bool lbl_012CAB99;                       // -na
extern bool lbl_012CA76D;                       // -view
extern bool lbl_012CA776;                       // -sp
extern char lbl_012CA778[];                     // -dm's mount model
extern int lbl_010B4F60;                        // -df's faction
extern bool lbl_012CA879;                       // -engine
extern bool lbl_012CA87A;                       // -engine_shadow
extern int lbl_010B5074;                        // -view's number, after the model
extern bool lbl_012CA777;                       // -sprite_script
extern bool lbl_012CA87F;                       // -replay
extern bool lbl_012CA980;                       // -replay:save
extern char lbl_012CA98C[];                     // -replay:cam
extern char lbl_010B4F64[];                     // -replay:file, "temp/battle.rpy" at first
extern bool lbl_012CA76F;                       // -custom
extern bool lbl_012CA772;                       // -fmv
extern bool lbl_012CA773;                       // -menu
extern bool lbl_012CADA3;                       // -host
extern char lbl_012CAEA4[];                     // -host's game name
extern char lbl_012CADA4[];                     // -host's player name, "default name" at first
extern bool lbl_010B5480;                       // -host: a battle rather than a campaign
extern bool lbl_012CA774;                       // -strat_ed
extern bool lbl_012CA775;                       // -battle_ed
extern bool lbl_012CA87C;                       // -rtm
extern bool lbl_012CB0E2;                       // -util
extern bool lbl_012CB0E3;                       // -util:animdb
extern bool lbl_012CB0E4;                       // -util:sound
extern bool lbl_012CB1EF;                       // -util:unit_models
extern bool lbl_012CB1F0;                       // -util:validate_models
extern bool lbl_012CADA0;                       // -pr (PR mode, toggle_pr)
extern int lbl_0126AE6C;                        // -faction's faction
extern char lbl_010B5178[];                     // the campaign, "imperial_campaign" at first
extern char lbl_010B4E60[];                     // the map -battle, -strat and the others start on
extern bool lbl_012CAB98;                       // -display_off
}

#endif

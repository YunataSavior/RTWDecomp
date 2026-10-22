#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\romans.cpp"
// (The original's path: nothing asserts, but its THIS_FILE comes between auto_optimise.cpp's and romans_cmd_line.cpp's,
// as its code comes between theirs, and its code refers to its .data.)
static char THIS_FILE[] = __FILE__;

// The game's entry point, and the game's main function: it starts the systems, runs the mode the command line asks
// for (the menus without one), and shuts the systems down.
#include <windows.h>
#include <fstream.h>
#include <stdio.h>
#include <string.h>

#include "unsorted.h"
#include "core_strings.h"
#include "map_constants.h"
#include "romans_cmd_line.h"
#include "romans_utils.h"

#pragma intrinsic(strcpy)

struct PackFile;
struct BattleDescription;
struct BattleTemplateSlot;

// A growing array: full, it chains a block twice its size (probably ca_libs; 0x14 bytes).
template <class T>
class ChainArray {
public:
    void push_back(const T& value);
    unsigned int size() const;
    T& operator[](unsigned int i)
    {
        if (i >= m_capacity)
            return (*m_next)[i - m_capacity];
        return m_data[i];
    }
    void clear();

    T* m_data;
    ChainArray* m_next;
    ChainArray* m_prev;
    unsigned int m_capacity;
    unsigned int m_size;
};

// The packs the game opens, kept open until it shuts down.
extern "C" ChainArray<PackFile*> lbl_012CA2D0;

// The sprites' packs (code\battlefield_handler.cpp; 0x14 bytes, a guess).
class SpritePacks {
public:
    SpritePacks();                              // 0x008C0455
    ~SpritePacks();                             // 0x008C059A
    void open();                                // 0x008C046B: packs\sprites_0.pak and the ones after it

    char m_data[0x14];
};

// The battle's packs (code\battlefield_handler.cpp; 0x14 bytes, a guess).
class BattlePacks {
public:
    BattlePacks();                              // 0x008C060B
    ~BattlePacks();                             // 0x008C0675
    void open();                                // 0x008C0621: packs\battle_open.pak and the ones after it

    char m_data[0x14];
};

// A file read through the packs (ca_libs\core\pc_buffers.cpp; 0x20 bytes).
class InputFile {
public:
    InputFile(const UniString& path, int);      // 0x00F3CF00
    ~InputFile();                               // 0x00F3CF40
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual bool is_open();                     // +0x0c

    char m_4[0x1c];
};

// The modes the game's loop runs (code\romans_event_handler.cpp, a guess).
class GameHandler {
public:
    virtual void unk_00();
};

// Shows a model or a sprite on its own, for -view and -sprite_script (code\env\model_view_mode.cpp).
class ModelViewMode : public GameHandler {
public:
    void set(int, const char* model, bool, const char*, int faction, bool engine, bool engine_shadow, bool,
             float);                            // 0x00DC0AEE
    void unk_00DC0BD6();                        // 0x00DC0BD6
    void unk_00DC0CDF();                        // 0x00DC0CDF
    void unk_00DC0CFA();                        // 0x00DC0CFA
    void unk_00DC1595();                        // 0x00DC1595
};

// Plays the movies (code\game_fmv_handler.cpp).
class FmvHandler : public GameHandler {
public:
    void add(const UniString& file, bool);     // 0x008C6981
    void start(GameHandler* next, int, int faction, int);  // 0x008C6A10: then runs next
};

// The screen between two modes (code\transition_screen_handler.cpp).
class TransitionScreenHandler {
public:
    void start(GameHandler* next, const UniString* text, int faction);  // 0x008C9797
};

// The real-time movie tool, -rtm (code\rtm\rtm_handler.cpp).
class RtmHandler : public GameHandler {
public:
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
    virtual void unk_20();
    virtual void unk_24();
    virtual void unk_28();
    virtual void unk_2c();
    virtual void unk_30();
    virtual void unk_34();
    virtual void unk_38();
    virtual void unk_3c();
    virtual void unk_40();
    virtual void unk_44();
    virtual void unk_48();
    virtual void unk_4c();
    virtual void load(UniString map);           // +0x50
    void unk_00DCEB0F();                        // 0x00DCEB0F
    void set_next(GameHandler* next) { m_next = next; }

    char m_4[0xc4];
    GameHandler* m_next;                        // +0xc8
};

// The campaign map editor, -strat_ed (code\strat_ed.cpp).
class StratEd {
public:
    bool load(const UniString& map);            // 0x00DB30C9
};

// What the battle map editor opens (code\battle_ed.cpp, a guess; 0x18 bytes).
struct BattleEdSettings {
    BattleEdSettings(const UniString& map);     // 0x00406424
    BattleEdSettings& operator=(const BattleEdSettings& other);  // 0x00406465

    int m_0;
    int m_4;
    int m_8;
    int m_c;
    bool m_10;
    UniString m_map;                            // +0x14
};

// The battle map editor, -battle_ed (code\battle_ed.cpp, a guess).
class BattleEdHandler : public GameHandler {
public:
    void set_settings(const BattleEdSettings& settings) { m_settings = settings; }

    BattleEdSettings m_settings;                // +0x04
};

// The display (code\display\display_manager_item.cpp).
class DisplayManager {
public:
    bool init(short width, short height, bool windowed);  // 0x00A8D3BD
    void unk_00A8E2D6();                        // 0x00A8E2D6: draws a frame (a guess)
    void unk_00A8FFA2();                        // 0x00A8FFA2
    void unk_00A90193();                        // 0x00A90193
    void unk_00A9089F(void* overlay);           // 0x00A9089F
};

// The textures (code\display\display_manager_texture.cpp).
class TextureManager {
public:
    void unk_00A93747();                        // 0x00A93747
};

// A picture shown over the screen (code\display\fade.cpp).
class Fade {
public:
    void show(const char* file, float);         // 0x0097AA52
};

// The sound (code\sound2.cpp).
class Sound {
public:
    bool init(int rate, int bits, int channels);  // 0x00AED1FE
    bool set_provider(const char* name);       // 0x00AED277: the Miles 3D provider
};

// The network (code\network\network_manager.cpp).
class NetworkManager {
public:
    void unk_00B7CDD7(bool);                    // 0x00B7CDD7
};

// The animations' database (code\display\anim_db.cpp).
class AnimDb {
public:
    void unk_00ACEE16();                        // 0x00ACEE16
};

// A set of factions, a bit for each (code\romans_utils.cpp; 0x04 bytes).
class FactionMask {
public:
    FactionMask() { m_bits = 0; }
    FactionMask(const FactionMask& other) { m_bits = other.m_bits; }
    FactionMask(int faction) { set(faction); }
    void set(int faction);                      // 0x00E308BC: faction's bit, or all of them for the faction count
    int first();                                // 0x00E30946: the first faction in the set
    FactionMask operator|(const FactionMask& other) const
    {
        FactionMask mask = *this;
        mask.m_bits |= other.m_bits;
        return mask;
    }

    unsigned int m_bits;
};

// A battle or a campaign to start, read from a map (code\romans_game.cpp; 0xe0 bytes).
class BattleSetup {
public:
    BattleSetup();                              // 0x00418B2B
    ~BattleSetup();                             // 0x004063B9
    void load(const UniString& map, const UniString& campaign, bool campaign_map);  // 0x00418D72
    void load(void* forces, const BattleDescription& description);  // 0x0041919A
    bool loaded() const { return m_loaded; }
    FactionMask playable() const;               // 0x00406374
    FactionMask all() const;                    // 0x0040638D: the playable factions and the unlockable ones

    char m_0[0x44];
    bool m_loaded;                              // +0x44
    FactionMask m_playable;                     // +0x48
    FactionMask m_unlockable;                   // +0x4c
    char m_50[0x90];
};

// A player of a battle or a campaign (code\romans_game.cpp; 0x1e4 bytes).
struct PlayerSlot {
    char m_data[0x1e4];
};

// The players: which factions play, and which are human (code\romans_game.cpp; 0x27b4 bytes).
class Players {
public:
    Players();                                  // 0x004062C2
    ~Players();                                 // 0x00406318
    int unk_00414F80(int faction);              // 0x00414F80
    void set_factions(FactionMask factions, FactionMask humans);  // 0x0041500F
    void set_human(int slot, bool human);       // 0x004151E2
    void unk_00415A0E(int slot, int);           // 0x00415A0E
    void unk_00415AC9(int slot, int);           // 0x00415AC9
    void unk_00415AF5(int slot, int);           // 0x00415AF5
    void unk_00415B21(int slot, int);           // 0x00415B21
    void unk_00415B7E(int slot, BattleTemplateSlot* from);  // 0x00415B7E

    PlayerSlot m_slots[21];
};

// (code\romans_game.cpp; 0x20 bytes.)
class Unk415D0E {
public:
    Unk415D0E();                                // 0x00415D0E

    char m_data[0x20];
};

struct Pair {
    int m_0;
    int m_4;
};

struct Quad {
    int m_0;
    int m_4;
    int m_8;
    int m_c;
};

// A custom battle's settings, from the battle template (code\romans_game.cpp; 0x60 bytes).
struct BattleDescription {
    BattleDescription(const UniString& campaign, void* armies, int, int, int, int, int, int, int, bool, int, Quad,
                      float, float, Pair, int);  // 0x00416B74

    int m_0;
    UniString m_4;
    int m_8;
    UniString m_c;
    char m_10[0x50];
};

// A side of the battle template (code\battle_template.cpp; 0x1c8 bytes).
struct BattleTemplateSlot {
    bool active();                              // 0x00617E10
    int unk_00617E61();                         // 0x00617E61
    int control();                              // 0x00617E70: 1 for a human

    char m_data[0x1c8];
};

// The custom battle and quick battle being set up (code\battle_template.cpp; 0xea8 bytes).
class BattleTemplate {
public:
    int state();                                // 0x0061922F: 5 when it's ready
    void reset();                               // 0x00619973
    void unk_0061B877(int faction);             // 0x0061B877: a quick battle for the faction
    int unk_0061BC7B(int);                      // 0x0061BC7B
    int unk_0061BCC1(int slot);                 // 0x0061BCC1
    int unk_04() const { return m_4; }
    Pair unk_08() const { return m_8; }
    float unk_10() const { return m_10; }
    float unk_14() const { return m_14; }
    int unk_18() const { return m_18; }
    int unk_24() const { return m_24; }
    int unk_2c() const { return m_2c; }
    int unk_30() const { return m_30; }
    const Quad& unk_34() const { return m_34; }
    bool unk_44() const { return m_44; }
    int unk_50() const { return m_50; }
    int unk_54() const { return m_54; }

    int m_0;
    int m_4;
    Pair m_8;
    float m_10;
    float m_14;
    int m_18;
    int m_1c;
    int m_20;
    int m_24;
    int m_28;
    int m_2c;
    int m_30;
    Quad m_34;
    bool m_44;
    int m_48;
    int m_4c;
    int m_50;
    int m_54;
    char m_58[8];                               // +0x58
    char m_60[8];                               // +0x60
    BattleTemplateSlot m_slots[8];              // +0x68
};

extern "C" {
void fn_00F48EA0(unsigned int);                 // -stop_alloc (ca_libs\display\pc_display.cpp)
void fn_00F48EB0(int);                          // -stop_alloc_count
void fn_00F48EC0(int);                          // -mem_force
PackFile* fn_00F3BB50(const unsigned short* path, int);  // opens a pack (ca_libs\core\pc_buffers.cpp)
void fn_00F3C360(PackFile* pack);              // closes it
void fn_00D92647(int);                          // (code\localisation\language.cpp)
void fn_00D9543F();                             // (code\localisation\string_table_manager.cpp)
const UniString* fn_00D939EA(int);              // a text from the game's text table
void fn_00D85238();                             // (code\menu\menu_base.cpp)
void fn_00D852CA();
void fn_00D7FA5B(const UniString&);
void fn_00D86D52(BattleSetup& setup, const UniString& name, UniString& password);  // hosts (code\menu\menu_handler.cpp)
bool fn_00F48620(HINSTANCE instance, HINSTANCE prev_instance, const char* title, int);  // makes the window (ca_libs\core\pc_core.cpp)
bool fn_00F440A0(void (*)(), void (*)(), int);  // starts the input (ca_libs\core\pc_core.cpp, a guess)
bool fn_00F3FF30(void (*)(), void (*)(), void (*)());  // starts the threads (ca_libs\core\pc_core_threads.cpp, a guess)
void fn_00F3EEA0(int milliseconds);             // waits
unsigned char* fn_00F3A770(const char* name);  // the option's value ("" for none), or 0 when it wasn't given
int fn_00E3C320(const char* name);              // a faction from its name (code\enum_names.cpp)
void fn_0041014C();                             // (code\romans_event_handler.cpp)
void fn_004101BE();
void fn_0041020A();
void fn_00410225();
void fn_004102A7();
void fn_0040FE26(GameHandler* handler);         // runs the game's loop, from handler
void fn_0040FEF3(bool display_off);
void fn_0040F750();
void fn_0040FF06(int);
void fn_00E2863D();                             // (code\env\output_window.cpp)
bool fn_00E2F867(const char* path);             // whether the file exists (code\romans_utils.cpp)
void fn_0041C3BA();                             // (code\romans_game.cpp)
bool fn_0041C3CE(BattleSetup& setup, Players& players, int faction, DiagnosticSettings& diagnostics, int, int);  // runs it
bool fn_0041CE9E(const char* file, DiagnosticSettings* diagnostics);  // plays a replay
void fn_0061B403(InputFile& file, BattleTemplate& battle);  // reads a battle template (code\battle_template.cpp)
void fn_00B2B7AE();                             // -util:sound (code\sound_db.cpp)
void fn_00AE0BE1();                             // -util:unit_models (code\display\model_db.cpp)
void fn_00AE0C11();                             // -util:validate_models
void fn_0040E419(UniString* path);              // the path in the -mod folder, when it's there (code\romans_cmd_line.cpp)
bool fn_00405301();                             // loads the game's data
bool fn_0040520C();
bool fn_00406104();
void fn_004050C9();                             // shuts the systems down

extern Sound lbl_018BD658;
extern NetworkManager lbl_018BF608;
extern DisplayManager lbl_017B2540;
extern TextureManager lbl_017B2E00;
extern Fade lbl_015CD200;
extern int lbl_017B26F8;
extern AnimDb lbl_017B5688;
extern ModelViewMode lbl_02948A48;
extern BattleTemplate lbl_02943828;
extern FmvHandler lbl_014D6428;
extern TransitionScreenHandler lbl_014D6710;
extern GameHandler* lbl_02921ACC;               // the menus
extern int lbl_02921A90;
extern RtmHandler* lbl_018BF5D0;
extern GameHandler lbl_02947F5C;                // the campaign map editor's
extern StratEd lbl_02948038;
extern BattleEdHandler lbl_02947C60;
extern UniString lbl_015CC0EC;                  // the -replay:cam camera file (code\display\cam_control_rtm.cpp)
extern bool lbl_015CC1C0;                       // there is one
extern char lbl_0126AC28[];                     // the Miles 3D provider, "Miles Fast 2D Positional Audio" at first
extern short lbl_0126ABF8;                      // the screen's width (a guess)
extern short lbl_0126ABFA;                      // its height
extern bool lbl_0126ABFC;                       // windowed (a guess)
extern char lbl_012CA3A0[];                     // the exe's path
extern bool lbl_012CA5BA;                       // the window is made
extern bool lbl_012CA5BB;                       // the threads are started
extern bool lbl_012CA5BC;
extern bool lbl_012CA5BD;                       // the sound is started
extern bool lbl_012CA5BE;                       // the input is started
extern bool lbl_012CADA1;                       // every side of a custom or quick battle is human (never set)
}

inline PackFile* open_pack(const UniString& path)
{
    return fn_00F3BB50(path.chars(), 0);
}

// Reads the battle template from file, or resets it when there's no file. (A constructor: VC6 expands an inline
// constructor holding an if-else, but not an inline function.)
struct ReadTemplate {
    ReadTemplate(InputFile& file, BattleTemplate& battle)
    {
        if (file.is_open())
            fn_0061B403(file, battle);
        else
            battle.reset();
    }
};

inline const char* map_name()
{
    return lbl_010B4E60;
}

inline void set_replay_camera(const UniString& file)
{
    lbl_015CC1C0 = true;
    lbl_015CC0EC = file;
}

inline void copy_to_ascii(char* dst, const unsigned short* src)
{
    for (;;) {
        char c = (char)*src;
        *dst = c;
        unsigned short w = *src;
        if (w == 0)
            break;
        dst++;
        src++;
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show)
{
    if (!fn_00EFA483(0x1298))
        return 0;
    fn_00425B4F();
    fn_00F32ED0(2500000);
    fn_00402805();

    char* full_command_line = GetCommandLineA();
    UniString arguments(full_command_line);
    char buffer[2001] = {0};
    if (arguments.length() > 2000)
        arguments.truncate(2000);
    copy_to_ascii(buffer, arguments.chars());
    lbl_012CA6B0 = buffer;

    fn_00F3A500(command_line);
    if (!fn_00408DCF())
        return 0;

    if (lbl_012CB1F2 == true) {
        const int size = 0x800;
        ifstream input;
        input.flags(0);
        input.open(lbl_012CB1F4, ios::in);
        char line[size];
        while (!input.eof())
            input.getline(line, size);
        input.close();
        return 0;
    } else if (lbl_012CB1F2 == true) {
        const int size = 0x800;
        ifstream input;
        input.flags(0);
        input.open(lbl_012CB1F4, ios::in);
        char line[size];
        while (!input.eof())
            input.getline(line, size);
        input.close();
        return 0;
    }

    // -fperr and -fplog run the game under romans_seh.cpp's exception handler.
    bool run_seh = lbl_012CB0D8 == 2 || lbl_012CB0D8 == 3;
    if (!run_seh)
        return fn_00403412(instance, prev_instance);
    lbl_012CE880 = lbl_012CB0D8 == 2;
    return fn_00425F8F(instance, prev_instance);
}

extern "C" int fn_00403412(HINSTANCE instance, HINSTANCE prev_instance)
{
    // NONMATCHING - stack slot order differs
    if (lbl_012CB0DC > 0)
        fn_00F48EA0(lbl_012CB0DC);
    if (lbl_010B54A0 > -1)
        fn_00F48EB0(lbl_010B54A0);
    lbl_018BF608.unk_00B7CDD7(lbl_012CB0D8 != 0);

    PackFile* mod_1 = open_pack(fn_0040E567("packs/mod_1.pak"));
    if (mod_1)
        lbl_012CA2D0.push_back(mod_1);
    PackFile* mod_0 = open_pack(fn_0040E567("packs/mod_0.pak"));
    if (mod_0)
        lbl_012CA2D0.push_back(mod_0);
    PackFile* patch_1 = open_pack(fn_0040E567("packs/patch_1.pak"));
    if (patch_1)
        lbl_012CA2D0.push_back(patch_1);
    PackFile* patch_0 = open_pack(fn_0040E567("packs/patch_0.pak"));
    if (patch_0)
        lbl_012CA2D0.push_back(patch_0);
    SpritePacks sprites;
    sprites.open();
    BattlePacks battle;
    battle.open();
    PackFile* ui_0 = open_pack(fn_0040E567("packs/ui_0.pak"));
    if (ui_0) {
        lbl_012CA2D0.push_back(ui_0);
        PackFile* ui_1 = open_pack(fn_0040E567("packs/ui_1.pak"));
        if (ui_1)
            lbl_012CA2D0.push_back(ui_1);
    }
    PackFile* game_init = open_pack(fn_0040E567("packs/game_init.pak"));

    fn_00D92647(1);
    fn_00D9543F();
    fn_00D85238();
    fn_00D852CA();
    fn_00D7FA5B(UniString(""));
    GetModuleFileNameA(0, lbl_012CA3A0, 0x200);

    bool failed = false;
    HANDLE mutex = CreateMutexA(0, TRUE, "Rome: Total War instance verification mutex");
    if (!lbl_012CB0C4 && GetLastError() == ERROR_ALREADY_EXISTS) {
        fn_00F40080(1, "%S", fn_00D939EA(0x172)->chars());
        return 0;
    }
    if (lbl_012CA75C) {
        int size = lbl_010B4E5C;
        if (size >= 0)
            fn_00F48EC0(size);
    }

    short width;
    short height;
    bool windowed;
    if (fn_00F48620(instance, prev_instance, "Rome: Total War", 0)) {
        lbl_012CA5BA = true;
    } else {
        failed = true;
        goto end;
    }
    if (fn_00F3FF30(fn_0041014C, fn_004101BE, fn_0041020A)) {
        lbl_012CA5BB = true;
    } else {
        failed = true;
        goto end;
    }
    if (fn_0040520C()) {
        lbl_012CA5BC = true;
    } else {
        fn_00F40080(1, "%S", fn_00D939EA(0x171)->chars());
        goto end;
    }
    if (false)
        fn_00E2863D();
    if (!lbl_012CAB99) {
        lbl_012CA5BD = lbl_018BD658.init(44100, 16, 2);
        if (!lbl_012CA5BD) {
            failed = true;
            goto end;
        }
        char provider[0x200];
        strcpy(provider, lbl_0126AC28);
        unsigned char* option = fn_00F3A770("snd_provider");
        if (option) {
            const char* format = "\"%[a-zA-Z0-9().-+ ]\"";
            sscanf((char*)option, format, provider);
        }
        if (!lbl_018BD658.set_provider(provider)) {
            strcpy(lbl_0126AC28, "Miles Fast 2D Positional Audio");
            if (!lbl_018BD658.set_provider(lbl_0126AC28)) {
                char message[0x400];
                sprintf(message, "Failed to initialise 3D audio!\n");
                fn_00F40080(1, message);
                goto end;
            }
        }
    } else {
        lbl_012CA5BD = true;
    }
    if (fn_00F440A0(fn_00410225, fn_004102A7, 1)) {
        lbl_012CA5BE = true;
    } else {
        fn_00F40080(1, "%S", fn_00D939EA(0x171)->chars());
        goto end;
    }

    width = lbl_0126ABF8;
    height = lbl_0126ABFA;
    windowed = lbl_0126ABFC;
    if (lbl_017B2540.init(width, height, windowed)) {
        lbl_017B2E00.unk_00A93747();
        lbl_018BF608.unk_00B7CDD7(lbl_012CB0D8 != 0);
        lbl_015CD200.show(fn_0040E567("menu/splash.tga"), 1.5f);
        lbl_017B26F8 = 0;
        lbl_017B2540.unk_00A90193();
        lbl_017B2540.unk_00A8FFA2();
        lbl_017B2540.unk_00A9089F(&lbl_015CD200);
        lbl_017B2540.unk_00A8E2D6();
        lbl_017B2540.unk_00A8E2D6();
        lbl_017B2540.unk_00A8E2D6();
        lbl_017B2540.unk_00A9089F(0);
        fn_00406104();
        lbl_018BF608.unk_00B7CDD7(lbl_012CB0D8 != 0);

        bool ran = false;
        if (lbl_012CA76D == true) {
            // -view: a model.
            if (fn_00405301()) {
                lbl_02948A48.set(lbl_010B5074, lbl_010B4E60, lbl_012CA776, (unsigned char)lbl_012CA778[0] ? lbl_012CA778 : 0,
                                 lbl_010B4F60, lbl_012CA879, lbl_012CA87A, false, 1.0f);
                fn_0040FE26(&lbl_02948A48);
                ran = true;
            }
        } else if (lbl_012CA777) {
            // -sprite_script: makes the sprites sprite_script.txt lists.
            if (fn_00405301()) {
                int count = 0;
                {
                    ScriptReader reader("sprite_script.txt", 0);
                    while (!reader.at_end()) {
                        count++;
                        reader.next_line();
                    }
                }
                ScriptReader script("sprite_script.txt", 0);
                int faction = 21;
                bool root = false;
                float fov = 1.0f;
                int line = 0;
                char model[0x100];
                char rest[0x100];
                while (!script.at_end()) {
                    line++;
                    fn_00F401D0("%d of %d\n", line, count);
                    strcpy(model, script.word());
                    script.skip_word();
                    faction = fn_00E3C320(script.word());
                    script.skip_word();
                    if (script.match("sprite_root")) {
                        root = true;
                        script.skip_word();
                    } else {
                        root = false;
                    }
                    if (script.match("fov")) {
                        script.skip_word();
                        fov = script.read_float();
                        script.skip_word();
                    } else {
                        fov = 1.0f;
                    }
                    if (script.word_length() > 0)
                        strcpy(rest, script.rest_of_line());
                    else
                        rest[0] = '\0';
                    script.next_line();
                    lbl_02948A48.set(1, model, true, (unsigned char)rest[0] ? rest : 0, faction, false, false, root, fov);
                    lbl_02948A48.unk_00DC0BD6();
                    lbl_02948A48.unk_00DC0CFA();
                    lbl_02948A48.unk_00DC1595();
                    lbl_02948A48.unk_00DC0CDF();
                }
                ran = false;
            }
        } else if (lbl_012CA87F && !lbl_012CA980) {
            // -replay: plays a battle's replay.
            if (fn_00405301()) {
                if ((unsigned char)lbl_012CA98C[0])
                    set_replay_camera(lbl_012CA98C);
                ran = fn_0041CE9E(lbl_010B4F64, &g_diagnostic_settings);
            }
        } else if (lbl_012CA76E == true) {
            // -battle: a historical battle, or the map's.
            if (fn_00405301()) {
                ran = false;
                BattleSetup setup;
                setup.load("", lbl_010B4E60, true);
                if (setup.loaded()) {
                    Players players;
                    int faction = lbl_0126AE6C;
                    if (faction == 21)
                        faction = FactionMask(setup.m_playable).first();
                    if (!lbl_012CA766) {
                        if (lbl_02945090 == 2)
                            players.set_factions(setup.playable(), setup.m_playable);
                        else
                            players.set_factions(setup.playable(), faction);
                    } else {
                        players.set_factions(setup.playable(), FactionMask());
                    }
                    int player = players.unk_00414F80(faction);
                    ran = fn_0041C3CE(setup, players, player, g_diagnostic_settings, 0, 0);
                }
            }
        } else if (lbl_012CA76F == true) {
            // -custom: the custom battle in custom\<map>.cbt.
            if (fn_00405301()) {
                ran = false;
                Unk415D0E unused;
                UniString path("custom/");
                path.append(lbl_010B4E60);
                path.append(".cbt");
                fn_0040E419(&path);
                InputFile file(path, 0);
                ReadTemplate(file, lbl_02943828);
                if (lbl_02943828.state() == 5) {
                    BattleDescription description(lbl_010B5178, lbl_02943828.m_60, lbl_02943828.unk_18(),
                                                  lbl_02943828.unk_54(), lbl_02943828.unk_50(),
                                                  lbl_02943828.unk_0061BC7B(0x219), lbl_02943828.unk_04(),
                                                  lbl_02943828.unk_24(), lbl_02943828.unk_2c(),
                                                  lbl_02943828.unk_44(), lbl_02943828.unk_30(), lbl_02943828.unk_34(),
                                                  lbl_02943828.unk_10(), lbl_02943828.unk_14(),
                                                  lbl_02943828.unk_08(), 0);
                    BattleSetup setup;
                    setup.load(lbl_02943828.m_58, description);
                    if (setup.loaded()) {
                        Players players;
                        unsigned int i;
                        for (i = 0; i < 8; i++) {
                            BattleTemplateSlot* slot = &lbl_02943828.m_slots[i % 8];
                            if (slot->active()) {
                                players.unk_00415AF5(i, i);
                                players.unk_00415B21(i, slot->unk_00617E61());
                                players.unk_00415AC9(i, lbl_02943828.unk_0061BCC1(i));
                                players.unk_00415A0E(i, 1);
                                bool human = slot->control() == 1;
                                if (lbl_012CA766 == true)
                                    human = false;
                                players.set_human(i, human);
                                players.unk_00415B7E(i, slot);
                            }
                        }
                        if (lbl_012CADA1 || lbl_012CADA0 && lbl_02945090 != 0) {
                            for (i = 0; i < 8; i++) {
                                BattleTemplateSlot* side = &lbl_02943828.m_slots[i % 8];
                                if (side->active())
                                    players.set_human(i, true);
                            }
                        }
                        ran = fn_0041C3CE(setup, players, 0, g_diagnostic_settings, 0, 0);
                    }
                }
            }
        } else if (lbl_012CA770 == true) {
            // -strat: a campaign on the map.
            if (fn_00405301()) {
                ran = false;
                BattleSetup setup;
                setup.load("", lbl_010B4E60, false);
                if (setup.loaded()) {
                    Players players;
                    int faction = 0;
                    if (lbl_02945090 == 0)
                        faction = lbl_0126AE6C;
                    if (faction == 21)
                        faction = FactionMask(setup.m_playable).first();
                    if (!lbl_012CA766) {
                        if (lbl_02945090 == 1)
                            players.set_factions(setup.all(), setup.m_playable | setup.m_unlockable);
                        else if (lbl_02945090 == 2)
                            players.set_factions(setup.playable(), setup.m_playable);
                        else
                            players.set_factions(setup.playable(), faction);
                    } else {
                        players.set_factions(setup.playable(), FactionMask());
                    }
                    int player = players.unk_00414F80(faction);
                    ran = fn_0041C3CE(setup, players, player, g_diagnostic_settings, 0, 0);
                }
            }
        } else if (lbl_012CA772 == true) {
            // -fmv: plays the movie named for the map.
            lbl_014D6428.add(lbl_010B4E60, true);
            lbl_014D6428.start(0, 0, 21, 0);
            fn_0040FE26(&lbl_014D6428);
            ran = true;
        } else if (lbl_012CA773 == true) {
            // -menu: the menus, straight away.
            if (fn_00405301()) {
                lbl_014D6710.start(lbl_02921ACC, 0, 21);
                ran = true;
            }
        } else if (lbl_012CADA3 == true) {
            // -host: hosts a multiplayer game.
            if (fn_00405301()) {
                BattleSetup setup;
                setup.load(lbl_012CAEA4, lbl_012CAEA4, lbl_010B5480);
                UniString name(lbl_012CADA4);
                UniString password;
                fn_00D86D52(setup, name, password);
                lbl_014D6710.start(lbl_02921ACC, 0, 21);
                ran = true;
            }
        } else if (lbl_012CA774 == true) {
            // -strat_ed: the campaign map editor.
            if (fn_00405301()) {
                fn_0041C3BA();
                lbl_014D6710.start(&lbl_02947F5C, 0, 21);
                ran = lbl_02948038.load(UniString(lbl_010B4E60));
            }
        } else if (lbl_012CA775 == true) {
            // -battle_ed: the battle map editor.
            if (fn_00405301()) {
                lbl_02947C60.set_settings(BattleEdSettings(UniString(lbl_010B4E60)));
                fn_0040FE26(&lbl_02947C60);
                ran = true;
            }
        } else if (lbl_012CA87C == true) {
            // -rtm: the real-time movie tool.
            if (fn_00405301()) {
                fn_0041C3BA();
                fn_0040FF06(lbl_02921A90);
                lbl_018BF5D0->unk_00DCEB0F();
                lbl_018BF5D0->load(UniString(map_name()));
                lbl_018BF5D0->set_next(lbl_02921ACC);
                lbl_014D6710.start(lbl_018BF5D0, 0, 21);
                ran = true;
            }
        } else if (fn_00F3A770("quick_battle")) {
            // -quick_battle: a quick battle, for the faction given or any.
            int faction = 21;
            unsigned char* option = fn_00F3A770("quick_battle");
            if (*option) {
                char name[0x80];
                if (sscanf((char*)option, "%s", name) == 1)
                    faction = fn_00E3C320(name);
            }
            ran = false;
            if (fn_00405301()) {
                lbl_02943828.unk_0061B877(faction);
                int state = lbl_02943828.state();
                if (state == 5) {
                    Unk415D0E unused;
                    BattleDescription description(lbl_010B5178, lbl_02943828.m_60, lbl_02943828.unk_18(),
                                                  lbl_02943828.unk_54(), lbl_02943828.unk_50(),
                                                  lbl_02943828.unk_0061BC7B(0x219), lbl_02943828.unk_04(),
                                                  lbl_02943828.unk_24(), lbl_02943828.unk_2c(),
                                                  lbl_02943828.unk_44(), lbl_02943828.unk_30(), lbl_02943828.unk_34(),
                                                  lbl_02943828.unk_10(), lbl_02943828.unk_14(),
                                                  lbl_02943828.unk_08(), 0);
                    BattleSetup setup;
                    setup.load(lbl_02943828.m_58, description);
                    if (setup.loaded()) {
                        Players players;
                        unsigned int i;
                        for (i = 0; i < 8; i++) {
                            BattleTemplateSlot* slot = &lbl_02943828.m_slots[i % 8];
                            if (slot->active()) {
                                players.unk_00415B21(i, slot->unk_00617E61());
                                players.unk_00415AC9(i, lbl_02943828.unk_0061BCC1(i));
                                players.unk_00415B7E(i, slot);
                            }
                        }
                        if (lbl_012CADA1 || lbl_012CADA0 && lbl_02945090 != 0) {
                            for (i = 0; i < 8; i++) {
                                BattleTemplateSlot* side = &lbl_02943828.m_slots[i % 8];
                                if (side->active())
                                    players.set_human(i, true);
                            }
                        } else {
                            players.set_human(0, true);
                        }
                        ran = fn_0041C3CE(setup, players, 0, g_diagnostic_settings, 0, 0);
                    }
                }
            }
        } else if (lbl_012CB0E2) {
            // -util: tools that check or rebuild the data.
            if (fn_00405301()) {
                if (lbl_012CB0E3)
                    lbl_017B5688.unk_00ACEE16();
                if (lbl_012CB0E4)
                    fn_00B2B7AE();
                if (lbl_012CB1EF)
                    fn_00AE0BE1();
                if (lbl_012CB1F0)
                    fn_00AE0C11();
            }
        } else if (fn_00405301()) {
            // No mode: the legal screen, the movies, then the menus (or the GameSpy game straight away).
            if (fn_00E2F867("data/menu/legal.tga")) {
                lbl_015CD200.show("data/menu/legal.tga", 1.5f);
                lbl_017B26F8 = 0;
                lbl_017B2540.unk_00A90193();
                lbl_017B2540.unk_00A8FFA2();
                lbl_017B2540.unk_00A9089F(&lbl_015CD200);
                lbl_017B2540.unk_00A8E2D6();
                lbl_017B2540.unk_00A8E2D6();
                lbl_017B2540.unk_00A8E2D6();
                lbl_017B2540.unk_00A9089F(0);
                fn_00F3EEA0(15000);
            }
            if (GameSpyLaunch::s_launch.m_launched) {
                fn_0040FE26(lbl_02921ACC);
            } else {
                lbl_014D6428.add("data/fmv/ATVI_Spinning_Logo_640x480.wmv", true);
                lbl_014D6428.add("data/fmv/rome_title.wmv", true);
                lbl_014D6428.start(lbl_02921ACC, 0, 21, 0);
                fn_0040FE26(&lbl_014D6428);
            }
            ran = true;
        }

        if (game_init)
            fn_00F3C360(game_init);
        if (ran) {
            fn_0040FEF3(lbl_012CAB98);
            fn_0040F750();
        }
    } else {
        fn_00F40080(1, "%S", fn_00D939EA(0x171)->chars());
    }

end:
    if (failed)
        fn_00F40080(1, "%S", fn_00D939EA(0x173)->chars());
    ReleaseMutex(mutex);
    fn_004050C9();
    for (unsigned int i = 0; i < lbl_012CA2D0.size(); i++)
        fn_00F3C360(lbl_012CA2D0[i]);
    lbl_012CA2D0.clear();
    return 0;
}

// The event messages: each makes a message of its type for a faction and sends it.
#include "unsorted.h"
#include "../faction.h"
#include "../romans_utils.h"
#include "../world.h"
#include "../romans_cmd_line.h"
#include "event_messages.h"
#include "strat_ui_v2.h"

// The message types, read from a file (not identified).
class EventMessageTypes {
public:
    EventMessage* create(int type, Faction* faction);  // 0x008F3499: null when the type is off or unknown (a guess)
};

// The battle's interface (codeattle_uishell.cpp; not identified).
class BattleUIShell {
public:
    bool unk_00B54B4C();                // 0x00B54B4C: a battle is being fought (a guess)
};

extern "C" {
extern bool lbl_0292A100;               // show_all_messages
extern BattleUIShell* lbl_018CD81C;
extern EventMessageTypes lbl_015706A8;
extern Campaign* lbl_02921AC0;
void fn_00CB5210(Faction* faction, EventMessage* message);  // sends it to the faction

// Whether a faction's event messages are made: 1 for the player's, 2 for another human's in a hotseat game, and 0 for
// none. Under -ai every faction's are made, and show_all_messages makes them all as the player's.
int fn_00CB52D0(int faction)
{
    int mode = 0;
    if (lbl_0292A100)
        mode = 1;
    else if (lbl_012CA766)
        mode = 2;
    else if (!lbl_018CD81C->unk_00B54B4C()) {
        bool own = faction == lbl_018DC3A4->unk_00B4CFFE();
        bool other_human = fn_00C803D8()
            ? (bool(!own) && lbl_02945090 != 0) || (own && !fn_00C803CE()->unk_154()) : false;
        if (other_human)
            mode = 2;
        else if (own)
            mode = 1;
    }
    return mode;
}

// To the faction that lost its eagle (type 0x60).
void fn_00CB67A8(Faction* faction)
{
    EventMessage* message = lbl_015706A8.create(0x60, faction);
    fn_00CB5210(faction, message);
}

// The Marian reforms (type 0x7b), to every major faction whose unk_178() is 0.
void fn_00CB691A(void* title, void* text, const char* image)
{
    String image_name;
    image_name = image;
    for (unsigned int i = 0; i < lbl_02921AC0->num_factions(); i++) {
        Faction* faction = lbl_02921AC0->faction_at(i);
        if (faction->unk_178() == 0 && faction->unk_00445B2B()) {
            EventMessage* message = lbl_015706A8.create(0x7b, faction);
            *message << title << image_name << text;
            fn_00CB5210(faction, message);
        }
    }
}

// A family's popularity has gone above 66 (type 0x82).
void fn_00CBAE4F(Faction* faction)
{
    EventMessage* message = lbl_015706A8.create(0x82, faction);
    fn_00CB5210(faction, message);
}

// The advisor on a change of the player's senate standing or popularity (type 0x87). rising goes in twice.
void fn_00CBB282(void* text, bool senate, bool rising)
{
    Faction* player = lbl_018DC3A4->unk_00B4CFDA();
    EventMessage* message = lbl_015706A8.create(0x87, player);
    *message << senate << rising << rising << text;
    fn_00CB5210(player, message);
}

// A shadow faction comes back (type 0x92), to the player, unless it is the player's own shadowed_by faction.
void fn_00CBB55B(Faction* faction)
{
    if (lbl_018DC3A4->unk_00B4CFDA()->shadowed_by() != faction->id()) {
        EventMessage* message = lbl_015706A8.create(0x92, lbl_018DC3A4->unk_00B4CFDA());
        *message << faction->display_name();
        fn_00CB5210(lbl_018DC3A4->unk_00B4CFDA(), message);
    }
}

// A dead faction comes back (type 0x96), to the player.
void fn_00CBB5FC(Faction* faction)
{
    EventMessage* message = lbl_015706A8.create(0x96, lbl_018DC3A4->unk_00B4CFDA());
    *message << faction->display_name();
    fn_00CB5210(lbl_018DC3A4->unk_00B4CFDA(), message);
}
// Rome and Constantinople are held by one of the two empires (type 0x9c), to every faction (Faction::unk_004CE245).
void fn_00CBB889()
{
    for (unsigned int i = 0; i < lbl_02921AC0->num_factions(); i++) {
        Faction* other = lbl_02921AC0->faction_at(i);
        EventMessage* message = lbl_015706A8.create(0x9c, other);
        fn_00CB5210(other, message);
    }
}
}

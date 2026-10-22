#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\menu\\create_piece.cpp"
// The menus' pieces. (The original's path, which its __FILE__s give.)

#include "../romans_cmd_line.h"

// The keyboard shortcuts' categories, which the shortcuts menu shows one at a time: camera, battle, strat, misc and
// editor. Only -enable_editor shows the editor's.
enum {
    SHORTCUTS_EDITOR = 4,
    NUM_SHORTCUT_CATEGORIES = 5
};

extern "C" {
extern bool lbl_02944C1B;               // the category was changed
extern int lbl_02944C1C;                // the category shown
}

// A menu button (not identified; the shortcut category buttons' base's vtable is 0x01060030).
class MenuButton {
public:
    virtual void unk_00();
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
    virtual void unk_50();
    virtual void unk_54();
    virtual void unk_58();
    virtual void unk_5c();
    virtual void unk_60();
    virtual void unk_64();
    virtual void unk_68();
    virtual void unk_6c();
    virtual void unk_70();
    virtual void unk_74();
    virtual void unk_78();
    virtual void clicked();             // +0x7c
};

// The shortcuts menu's button for the next category (vtable 0x0105EC00).
class NextShortcutsButton : public MenuButton {
public:
    void clicked();                     // 0x00D3E89E
};

// The shortcuts menu's button for the previous category (vtable 0x0105EC80).
class PreviousShortcutsButton : public MenuButton {
public:
    void clicked();                     // 0x00D3E8EA
};

// Without -enable_editor, the editor's category is skipped.
void NextShortcutsButton::clicked()
{
    int category = lbl_02944C1C;
    category = (category + 1 + NUM_SHORTCUT_CATEGORIES) % NUM_SHORTCUT_CATEGORIES;
    lbl_02944C1C = category;
    lbl_02944C1B = true;
    if (!lbl_012CAA90 && category == SHORTCUTS_EDITOR)
        clicked();
}

void PreviousShortcutsButton::clicked()
{
    int category = lbl_02944C1C;
    category = (category - 1 + NUM_SHORTCUT_CATEGORIES) % NUM_SHORTCUT_CATEGORIES;
    lbl_02944C1C = category;
    lbl_02944C1B = true;
    if (!lbl_012CAA90 && category == SHORTCUTS_EDITOR)
        clicked();
}

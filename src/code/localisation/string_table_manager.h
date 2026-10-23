// The text tables (code\localisation\string_table_manager.cpp).
#ifndef STRING_TABLE_MANAGER_H
#define STRING_TABLE_MANAGER_H

#include "unsorted.h"
#include "core_strings.h"

// A table of texts by key, from a text file and its lookup file (not identified).
class StringTable {
public:
    virtual void unk_00();
    virtual const UniString* lookup(const UniString& key);            // +0x04
    virtual const UniString* text(int index);                       // +0x08
    virtual const UniString* lookup_text(const UniString& key);       // +0x0c
    void unk_00D9405A();                                              // 0x00D9405A
};

// The text tables, by the files loaded into them.
enum {
    SHARED_TABLE,                       // shared.txt
    BATTLE_TABLE,                       // battle.txt
    STRAT_TABLE,                        // strat.txt
    DIPLOMACY_TABLE,                    // diplomacy.txt
    BATTLE_ED_TABLE,                    // battle_ed.txt
    TABLE_5,                            // not loaded by the setup (0x00D9543F)
    TOOLTIPS_TABLE,                     // tooltips.txt
    NUM_TABLES
};

const UniString* battle_text(int index);        // 0x00D939C7
const UniString* shared_text(int index);        // 0x00D939EA
const UniString* strat_text(int index);         // 0x00D93A0C
const UniString* battle_ed_text(int index);     // 0x00D93A2F
const UniString* diplomacy_text(int index);     // 0x00D93A52
const UniString* tooltips_text(int index);      // 0x00D93A75
StringTable* table_at(int index);               // 0x00D93A98

extern "C" StringTable* lbl_02946598[NUM_TABLES];

// Table INDEX.
inline StringTable*& string_table(int index)
{
    return lbl_02946598[index];
}

extern "C" {
StringTable* fn_00D95980(int, const UniString& file, bool, const UniString& lookup_file, bool, bool, bool);
void fn_00D95AB2(StringTable* table);
}

#endif

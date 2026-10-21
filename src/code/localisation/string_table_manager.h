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
    virtual void unk_08();
    virtual const UniString* lookup_text(const UniString& key);       // +0x0c
    void unk_00D9405A();                                              // 0x00D9405A
};

extern "C" {
StringTable* fn_00D95980(int, const UniString& file, bool, const UniString& lookup_file, bool, bool, bool);
void fn_00D95AB2(StringTable* table);
}

#endif

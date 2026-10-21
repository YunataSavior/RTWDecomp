// The campaign's victory conditions (code\win_conditions.cpp).
#ifndef WIN_CONDITIONS_H
#define WIN_CONDITIONS_H

#include "unsorted.h"

class ScriptReader;

// Skips lines to the first that starts with key, then skips key; false when there's none.
bool find_key(ScriptReader& reader, const char* key);  // 0x00445BA4

#endif

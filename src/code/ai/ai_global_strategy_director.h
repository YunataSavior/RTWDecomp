// A faction's AI's overall strategy (code\ai\ai_global_strategy_director.cpp; the class's name is a guess from the
// unit's).
#ifndef AI_GLOBAL_STRATEGY_DIRECTOR_H
#define AI_GLOBAL_STRATEGY_DIRECTOR_H

#include "unsorted.h"
#include "../faction.h"

// The AI of a horde: a faction without settlements, moving its armies (code\ai\ai_horde_controller.cpp; 0x70 bytes).
class AIHordeController {
public:
    AIHordeController(FactionAI* ai);   // 0x0051090D
    virtual ~AIHordeController();

    char m_pad4[0x70 - 4];
};

// The overall strategy (not identified beyond its unit).
class AIGlobalStrategyDirector {
public:
    void unk_0050B2A2();                // 0x0050B2A2: empties its lists and deletes the horde controller
    void update_horde();                // 0x0050B3E7: makes or deletes the horde controller
    inline void make_horde_controller();  // 0x0050F5B0 (an inline VC6 doesn't expand)

    char m_pad0[4];
    FactionAI* m_ai;                    // +0x04
    Faction* m_faction;                 // +0x08
    char m_padc[0x364 - 0xc];
    AIHordeController* m_horde;         // +0x364: while the faction is a horde
};

#endif

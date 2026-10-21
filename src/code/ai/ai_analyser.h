// What the AI knows about the factions (code\ai\ai_analyser.cpp).
#ifndef AI_ANALYSER_H
#define AI_ANALYSER_H

#include "unsorted.h"

struct ArmyStrengths;

// What the AI knows about each faction (an AI unit's globals).
struct AIFactionSummary {
    int m_value;                        // positive when the faction is a threat to someone (not identified)
    char m_pad4[0x40 - 4];
};

// The AI's total strength of a faction's armies or fleets (the same 64-byte records as AIFactionSummary's, from
// 0x004E282B and 0x004E28AA).
struct AIFactionStrength {
    int m_strength;
    char m_pad4[0x40 - 4];
};

// The AI's record of each pair of factions (0xc bytes from 0x012D1548, by faction × 21 + other; set in 0x004E18FA).
struct AIFactionPair {
    bool m_strongest_neighbour;         // +0x0a
    char m_pad1[0xc - 1];
};

// The same records' +0x00: the length of the land border between the faction's regions and the other's (0x004E3FAD).
struct AIFactionBorder {
    int m_length;
    char m_pad4[0xc - 4];
};

extern "C" {
extern AIFactionSummary lbl_012D0F78[21];
extern AIFactionStrength lbl_012D0F74[21];  // the total strength of each faction's armies
extern AIFactionStrength lbl_012D0F68[21];  // the total strength of each faction's fleets
extern AIFactionBorder lbl_012D1548[21 * 21];
extern AIFactionPair lbl_012D1552[21 * 21];
int fn_004E335E(int faction);
void fn_004E4403(int faction, int region, ArmyStrengths* strengths);  // the armies in the region (and its settlements)
int fn_004E282B(Faction* faction, int* count);  // the faction's armies' total strength (those without +0xac set)
}

#endif

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

// Another figure of the same 64-byte records, by the field's own address.
struct AIFactionFigure {
    int m_value;
    char m_pad4[0x40 - 4];
};

// Another field of the pair records below, by the field's own address.
struct AIFactionPairFigure {
    int m_value;
    char m_pad4[0xc - 4];
};

struct AIFactionPairFlag {
    bool m_set;
    char m_pad1[0xc - 1];
};

// A region of an AI region group (0x10 bytes).
struct AIRegionGroupEntry {
    int m_region;
    char m_pad4[0xc];
};

// A group of a faction's regions that border each other (codeii_analyser_region_group.cpp).
struct AIRegionGroup {
    int count_4c() const { return m_4c; }
    bool unk_4c() const { return count_4c() != 0 ? true : false; }

    Array<AIRegionGroupEntry> m_regions;
    char m_padc[4];
    int m_faction;                      // +0x10
    char m_pad14[0x4c - 0x14];
    int m_4c;                           // +0x4c
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
extern AIFactionFigure lbl_012D0F54[21];  // a total over the faction's settlements, by their level (0x004E3074)
extern AIFactionFigure lbl_012D0F7C[21];  // how many of its living neighbours it's at war with (fn_004E335E)
extern AIFactionFigure lbl_012D0F84[21];  // the faction it has bit 3 of the relation's flags with, or 21
// The faction's armies' strength near the other's regions (0x004E2A1F; +0x04 of the pair records; 1 at least
// while they share a border).
extern AIFactionPairFigure lbl_012D154C[21 * 21];
// Whether a faction allied to it, and not to the other, borders the other (0x004E3299; +0x08).
extern AIFactionPairFlag lbl_012D1550[21 * 21];
extern AIFactionPairFlag lbl_012D1551[21 * 21];  // +0x09 (set at 0x004E1EF6, not identified)
bool fn_004E99BE(int region);               // its region group's +0x08 is 1 (code\ai\ai_analyser_region_group.cpp)
bool fn_004E99ED(int faction, int other);   // (the same unit)
AIRegionGroup* fn_004E94E5(int faction, int index);  // the faction's INDEXth region group, or 0
unsigned int fn_004E9547(int faction);     // how many region groups the faction has
int fn_004E4AC6(int region);                // four parts of a region's value as a target
int fn_004E4B16(int region);
int fn_004E4BE6(int region);
int fn_004E4C70(int region);                // by its settlement's level, its population and more
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

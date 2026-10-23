// Units (code\unit.cpp).
#ifndef UNIT_H
#define UNIT_H

#include "unsorted.h"
#include "spying_info.h"

// A unit's soldier (4 bytes of bitfields; not identified).
struct UnitSoldier {
    unsigned int m_0 : 23;
    unsigned int m_23 : 3;              // experience? (a guess)
    unsigned int m_26 : 3;
    unsigned int m_29 : 3;
};

// What a unit's +0x44 points to (not identified).
class UnitUnk44 {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08(Unit* unit);    // +0x08: the unit leaves (a guess)
};

// A unit's first 0xc bytes (not identified).
class Unit0 {
public:
    virtual ~Unit0();

    int m_4;                            // +0x4
    bool m_8;                           // +0x8
};

// A unit of an army (code/unit.cpp).
class Unit : public Unit0, public UnitSpyingInfo {
public:
    UnitUnk44* unk_44() const { return m_44; }
    Settlement* unk_48() const { return m_48; }
    UnitUnk50* unk_50() const { return m_50; }
    Character* unk_278() const { return m_278; }
    unsigned int num_soldiers() const { return m_num_soldiers; }

    void unk_007C28A2();                // 0x007C28A2
    void unk_007C2E5E();                // 0x007C2E5E
    void unk_007B64D2(Army* army, int, int);  // 0x007B64D2: moves it into the army (a guess)
    int unk_60() const { return m_60; }
    bool unk_007C8444();                // 0x007C8444: still fighting in a battle (a guess)
    int unk_007B9438();                 // 0x007B9438: how many of its four m_298 slots are filled
    int category();                     // 0x007B9414: its type's m_2c (4 for ships)

    virtual ~Unit();
    virtual void vf_004(); virtual void vf_008(); virtual void vf_00c(); virtual void vf_010(); virtual void vf_014();
    virtual void vf_018(); virtual void vf_01c(); virtual void vf_020(); virtual void vf_024(); virtual void vf_028(); virtual void vf_02c();
    virtual void vf_030(); virtual void vf_034(); virtual void vf_038(); virtual void vf_03c(); virtual void vf_040(); virtual void vf_044();
    virtual void vf_048(); virtual void vf_04c(); virtual void vf_050(); virtual void vf_054(); virtual void vf_058(); virtual void vf_05c();
    virtual void vf_060(); virtual void vf_064(); virtual void vf_068(); virtual void vf_06c(); virtual void vf_070(); virtual void vf_074();
    virtual void vf_078(); virtual void vf_07c(); virtual void vf_080(); virtual void vf_084(); virtual void vf_088(); virtual void vf_08c();
    virtual void vf_090(); virtual void vf_094(); virtual void vf_098(); virtual void vf_09c(); virtual void vf_0a0(); virtual void vf_0a4();
    virtual void vf_0a8(); virtual void vf_0ac(); virtual void vf_0b0(); virtual void vf_0b4(); virtual void vf_0b8(); virtual void vf_0bc();
    virtual void vf_0c0(); virtual void vf_0c4(); virtual void vf_0c8(); virtual void vf_0cc(); virtual void vf_0d0(); virtual void vf_0d4();
    virtual void vf_0d8(); virtual void vf_0dc(); virtual void vf_0e0(); virtual void vf_0e4(); virtual void vf_0e8(); virtual void vf_0ec();
    virtual void vf_0f0(); virtual void vf_0f4(); virtual void vf_0f8(); virtual void vf_0fc(); virtual void vf_100(); virtual void vf_104();
    virtual void vf_108(); virtual void vf_10c(); virtual void vf_110(); virtual void vf_114(); virtual void vf_118(); virtual void vf_11c();
    virtual void vf_120(); virtual void vf_124(); virtual void vf_128(); virtual void vf_12c(); virtual void vf_130(); virtual void vf_134();
    virtual void vf_138(); virtual void vf_13c(); virtual void vf_140(); virtual void vf_144(); virtual void vf_148(); virtual void vf_14c();
    virtual void vf_150(); virtual void vf_154(); virtual void vf_158(); virtual void vf_15c(); virtual void vf_160(); virtual void vf_164();
    virtual void vf_168(); virtual void vf_16c(); virtual void vf_170(); virtual void vf_174(); virtual void vf_178(); virtual void vf_17c();
    virtual void vf_180(); virtual void vf_184(); virtual void vf_188(); virtual void vf_18c(); virtual void vf_190(); virtual void vf_194();
    virtual void vf_198(); virtual void vf_19c(); virtual void vf_1a0(); virtual void vf_1a4(); virtual void vf_1a8(); virtual void vf_1ac();
    virtual void vf_1b0(); virtual void vf_1b4(); virtual void vf_1b8(); virtual void vf_1bc(); virtual void vf_1c0(); virtual void vf_1c4();
    virtual void vf_1c8(); virtual void vf_1cc(); virtual void vf_1d0(); virtual void vf_1d4(); virtual void vf_1d8(); virtual void vf_1dc();
    virtual void vf_1e0(); virtual void vf_1e4(); virtual void vf_1e8(); virtual void vf_1ec(); virtual void vf_1f0(); virtual void vf_1f4();
    virtual void vf_1f8(); virtual void vf_1fc(); virtual void vf_200(); virtual void vf_204(); virtual void vf_208(); virtual void vf_20c();
    virtual void vf_210(); virtual void vf_214(); virtual void vf_218(); virtual void vf_21c(); virtual void vf_220(); virtual void vf_224();
    virtual void vf_228(); virtual void vf_22c(); virtual void vf_230(); virtual void vf_234(); virtual void vf_238(); virtual void vf_23c();
    virtual void vf_240(); virtual void vf_244(); virtual void vf_248(); virtual void vf_24c(); virtual void vf_250(); virtual void vf_254();
    virtual void vf_258(); virtual void vf_25c(); virtual void vf_260(); virtual void vf_264(); virtual void vf_268(); virtual void vf_26c();
    virtual void vf_270(); virtual void vf_274(); virtual void vf_278(); virtual void vf_27c(); virtual void vf_280(); virtual void vf_284();
    virtual void vf_288(); virtual void vf_28c(); virtual void vf_290(); virtual void vf_294(); virtual void vf_298(); virtual void vf_29c();
    virtual void vf_2a0(); virtual void vf_2a4(); virtual void vf_2a8();
    virtual int kill_soldiers(int percent);  // +0x2ac: returns how many died

    char m_pad30[0x40 - 0x30];
    int m_40;                           // +0x40
    UnitUnk44* m_44;                    // +0x44
    Settlement* m_48;                   // +0x48: the settlement it's queued to retrain in, or 0 (a guess)
    char m_pad4c[0x50 - 0x4c];
    UnitUnk50* m_50;                    // +0x50
    char m_pad54[0x60 - 0x54];
    int m_60;                           // +0x60: its state in a battle (a guess); 18 to 21 count as out of it
    char m_pad64[0x278 - 0x64];
    Character* m_278;                   // +0x278: the character it's the bodyguard of, or 0
    float m_27c;                        // +0x27c
    unsigned int m_num_soldiers;        // +0x280
    char m_pad284[0x288 - 0x284];
    int m_288;                          // +0x288
    char m_pad28c[0x298 - 0x28c];
    void* m_298[4];                     // +0x298
    char m_pad2a8[0x2b8 - 0x2a8];
    int m_2b8;                          // +0x2b8
    char m_pad2bc[0x30c - 0x2bc];
    UnitSoldier* m_soldiers;            // +0x30c
};

#endif

// The interface's messages: the windows that event messages open (code\ui\ui_message_handler.cpp).
#ifndef UI_MESSAGE_HANDLER_H
#define UI_MESSAGE_HANDLER_H

#include "unsorted.h"

class EventMessage;

// A point or a size on the screen (not identified).
struct UIPoint {
    UIPoint() : x(0), y(0) {}

    int x;
    int y;
};

// A piece of the interface (not identified; 0x90 bytes, from the CA_DELETE that deletes one).
class UIPiece {
public:
    virtual ~UIPiece();
    virtual void unk_004();
    virtual void unk_008();
    virtual void unk_00c();
    virtual void unk_010();
    virtual void unk_014();
    virtual void unk_018(const UIPoint& point);  // +0x18
    virtual void unk_01c();
    virtual void unk_020();
    virtual void unk_024();
    virtual void unk_028();
    virtual void unk_02c();
    virtual void unk_030();
    virtual void unk_034();
    virtual void unk_038();
    virtual void unk_03c();
    virtual void unk_040();
    virtual void unk_044();
    virtual void unk_048();
    virtual void unk_04c();
    virtual void unk_050();
    virtual void unk_054();
    virtual void unk_058();                 // +0x58: before it's deleted (a guess: it closes)
    virtual void unk_05c();
    virtual void unk_060(bool shown);       // +0x60 (a guess)
    virtual void unk_064();
    virtual void unk_068();
    virtual void unk_06c();
    virtual void unk_070();
    virtual void unk_074();
    virtual void unk_078();
    virtual void unk_07c();
    virtual void unk_080();
    virtual void unk_084();
    virtual void unk_088();
    virtual void unk_08c();
    virtual void unk_090();
    virtual void unk_094();
    virtual void unk_098();
    virtual void unk_09c();
    virtual void unk_0a0();
    virtual void unk_0a4();
    virtual void unk_0a8();
    virtual void unk_0ac();
    virtual void unk_0b0();
    virtual void unk_0b4();
    virtual void unk_0b8();
    virtual void unk_0bc();
    virtual void unk_0c0();
    virtual void unk_0c4();
    virtual void unk_0c8();
    virtual void unk_0cc();
    virtual void unk_0d0();
    virtual void unk_0d4();
    virtual void unk_0d8();
    virtual void unk_0dc();
    virtual void unk_0e0();
    virtual void unk_0e4();
    virtual void unk_0e8();
    virtual void unk_0ec();
    virtual void unk_0f0();
    virtual void unk_0f4();
    virtual void unk_0f8();
    virtual void unk_0fc();
    virtual void unk_100();
    virtual void unk_104();
    virtual void unk_108();
    virtual void unk_10c();
    virtual void unk_110();
    virtual void unk_114();
    virtual void unk_118();
    virtual void unk_11c();
    virtual void unk_120();
    virtual void unk_124();
    virtual void unk_128();
    virtual void unk_12c();
    virtual void unk_130();
    virtual void unk_134();
    virtual void unk_138();
    virtual void unk_13c();
    virtual void unk_140();
    virtual void unk_144();
    virtual void unk_148();
    virtual void unk_14c();
    virtual void unk_150();
    virtual void unk_154();
    virtual void unk_158();
    virtual void unk_15c();
    virtual void unk_160();
    virtual void unk_164();
    virtual void unk_168();
    virtual void unk_16c();
    virtual void unk_170();
    virtual void unk_174();                 // +0x174

    char m_pad4[0x90 - 4];
};

// The window an event message opens (0x44c bytes).
class UIMessageBox : public UIPiece {
public:
    UIMessageBox(EventMessage* message);    // 0x00D1B299

    void unk_b0_174() { m_b0.unk_174(); }

    char m_pad90[0xb0 - 0x90];
    UIPiece m_b0;                           // +0xb0 (a guess: a piece of its own)
    char m_pad140[0x44c - 0x140];
};

// The open message windows (not identified beyond its unit).
class UIMessageHandler {
public:
    UIMessageBox* open(EventMessage* message, bool now, int);  // 0x00D1BF82: the window for the message, or 0
    void unk_00D1C7FA(UIMessageBox* box, int);  // 0x00D1C7FA

    char m_pad0[4];
    void* m_4;                              // +0x04: no windows open without it
    bool m_8;                               // +0x08
    char m_pad9[0x30 - 9];
    Array<UIMessageBox*> m_30;              // +0x30: the windows waiting to open (a guess)
};

#endif

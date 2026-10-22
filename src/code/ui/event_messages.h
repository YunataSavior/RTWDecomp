// The event messages (code\ui\event_messages.cpp).
#ifndef EVENT_MESSAGES_H
#define EVENT_MESSAGES_H

#include "unsorted.h"

class UIPiece;

// A message being made. Each << adds a value for its text.
class EventMessage {
public:
    EventMessage& operator<<(void* text);           // 0x00CC4CCA
    EventMessage& operator<<(const bool& value);    // 0x00CC4D26
    EventMessage& operator<<(const String& text);   // 0x00CC4D82

    UIPiece* m_window;                              // +0x00: a window it had already (a guess)
    char m_pad4[0x24 - 4];
    int m_type;                                     // +0x24
};

#endif

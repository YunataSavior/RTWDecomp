// Pointers that are cleared when their object goes (code\env\tracked_object.cpp).
#ifndef TRACKED_OBJECT_H
#define TRACKED_OBJECT_H

#include "unsorted.h"

// Something told when an object it points at goes, so the pointer can be cleared (not identified; its first word is
// probably a vtable).
class Watcher {
public:
    void watch(void* object);           // 0x00E4B15F: adds it to the object's watchers
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual bool valid();               // +0x0c: whether it points at something (a guess)
};

// A pointer that watches its object (8 bytes).
template <class T>
class WatchedPointer : public Watcher {
public:
    T* get() const { return m_object; }
    // After a load: the object's new address, watched again (the old one's watchers went with the old game).
    void restore(T* object)
    {
        m_object = object;
        if (m_object)
            watch(m_object);
    }

    T* m_object;
};

// A pointer to a character that watches it, so it's cleared when the character goes. Its code is shared with the same
// pointers to settlements (Campaign's Rome and Constantinople): the linker folded the copies.
class WatchedCharacter : public Watcher {
public:
    void set(Character* character);     // 0x0056B311
    void restore(Character* character); // 0x004D69C7: the same as WatchedPointer::restore
    Character* get() const { return m_character; }

    Character* m_character;             // +0x04
};

#endif

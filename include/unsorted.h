// Declarations whose source file is not known yet. Move each to its own header once its home is found.
// Unnamed functions and variables keep the names tools/split.py gives them (fn_/lbl_ and their address), declared
// extern "C" so that they compile to those names.
#ifndef UNSORTED_H
#define UNSORTED_H

#include <windows.h>
#include <new.h>

#include "for_scope.h"
#include "maths_random.h"
#include "core_strings.h"
// A list of pointers (12 bytes: its items, its capacity and its count; the fields' order is a guess).
template <class T>
class PointerList {
public:
    unsigned int size() const { return m_count; }
    T* operator[](unsigned int index) const { return m_items[index]; }

    class iterator {
    public:
        iterator(T** p) : m_p(p) {}
        T*& operator*() const { return *m_p; }
        iterator& operator++() { ++m_p; return *this; }
        bool operator==(const iterator& other) const { return m_p == other.m_p; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

        T** m_p;
    };

    iterator begin() { return m_items; }
    iterator end() { return m_items + m_count; }

    T** m_items;
    int m_capacity;
    int m_count;
};

// CA's growable array (probably ca_libs): its items, its capacity and its count. Its functions that are not inline
// are compiled once per item type, and the linker folded the copies whose code is the same.
template <class T>
class Array {
public:
    class iterator {
    public:
        iterator(T* p) : m_p(p) {}
        T& operator*() const { return *m_p; }
        T* operator->() const { return m_p; }
        iterator& operator++() { ++m_p; return *this; }
        bool operator==(const iterator& other) const { return m_p == other.m_p; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

        T* m_p;
    };

    Array();                                // 0x005F4004 (4-byte items), 0x00EB9407 (8-byte items)
    Array(unsigned int capacity);           // 0x004D5B5A (4-byte items): 8 when capacity is 0
    Array(const Array& other);              // 0x0083FB6F (4-byte items)
    ~Array();                               // 0x00A48114
    Array& operator=(const Array& other);   // 0x00608E5E (8-byte items)

    unsigned int size() const { return m_size; }
    T& operator[](int index) { return m_data[index]; }
    T* data() { return m_data; }
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    iterator last() { return m_data + m_size - 1; }
    void clear() { while (m_size != 0) pop_back(); }
    void pop_back()
    {
        m_size--;
        m_data[m_size].~T();
    }
    T& back() { return m_data[m_size - 1]; }
    bool empty() const { return m_size == 0; }

    T* add();                               // a new default item; 0x008DF9C6 (8-byte items)
    T* push_back(const T& item);            // 0x006FAF63 (4-byte items), 0x009DDF80 (8-byte items)
    void erase(int index);                  // 0x00D1CD66 (8-byte items)
    iterator erase(iterator& it)
    {
        erase(it.m_p - m_data);
        return it;
    }
    iterator find(const T& item);           // 0x0097344A (4-byte items)
    T* add_unique(const T& item);           // 0x004A2C76 (Array<int>)
    void insert(const T& item, int index);  // 0x004B6C3B, 0x006B74EF (4-byte items)

    T* m_data;
    unsigned int m_capacity;
    unsigned int m_size;

private:
    static T* allocate(unsigned int count)
    {
        T* data = (T*)::operator new((count * sizeof(T) + 3) / 4 * 4);
        return data;
    }

    static void deallocate(T* data)
    {
        void* block = data;
        ::operator delete(block);
    }
};

template <class T>
Array<T>::Array()
{
    m_capacity = 8;
    m_size = 0;
    m_data = allocate(m_capacity);
}

template <class T>
Array<T>::Array(unsigned int capacity)
{
    m_capacity = capacity > 0 ? capacity : 8;
    m_size = 0;
    m_data = allocate(m_capacity);
}

template <class T>
Array<T>::Array(const Array& other)
{
    m_capacity = other.m_size ? other.m_size : 8;
    m_size = 0;
    m_data = allocate(m_capacity);
    for (unsigned int i = 0; i < other.m_size; i++)
        push_back(other.m_data[i]);
}

template <class T>
Array<T>::~Array()
{
    while (m_size != 0)
        pop_back();
    deallocate(m_data);
}

template <class T>
T* Array<T>::add()
{
    if (m_size == m_capacity) {
        unsigned int old_size = m_size;
        T* old_data = m_data;
        m_capacity *= 2;
        m_size = 0;
        m_data = allocate(m_capacity);
        for (unsigned int i = 0; i < old_size; i++) {
            new (&m_data[m_size++]) T(old_data[i]);
            old_data[i].~T();
        }
        deallocate(old_data);
    }
    return new (&m_data[m_size++]) T;
}

template <class T>
T* Array<T>::push_back(const T& item)
{
    if (m_size == m_capacity) {
        unsigned int old_size = m_size;
        T* old_data = m_data;
        m_capacity *= 2;
        m_size = 0;
        m_data = allocate(m_capacity);
        for (unsigned int i = 0; i < old_size; i++) {
            new (&m_data[m_size++]) T(old_data[i]);
            old_data[i].~T();
        }
        deallocate(old_data);
    }
    return new (&m_data[m_size++]) T(item);
}

template <class T>
void Array<T>::erase(int index)
{
    m_size--;
    for (unsigned int i = index; i < m_size; i++)
        m_data[i] = m_data[i + 1];
}

template <class T>
Array<T>::iterator Array<T>::find(const T& item)
{
    iterator last = end();
    iterator first = begin();
    for (iterator it = first; it != last; ++it)
        if (item == *it)
            return it;
    return last;
}

template <class T>
T* Array<T>::add_unique(const T& item)
{
    // NONMATCHING - the original builds find's result in place and has five more unused slots, as if VC6 had started to inline find
    iterator found = find(item);
    iterator end_it = end();
    if (found == end_it)
        return push_back(item);
    return found.m_p;
}

// The game's allocator records each block's owner and the line that made it (probably ca_libs). Each source file
// declares THIS_FILE, its own path, for these.
void* operator new(unsigned int size, const char* owner, const char* file, int line, bool flag);  // 0x00F490F0
#define NEW new (THIS_FILE, __FILE__, __LINE__, true)

extern "C" {
void* fn_00F49080(void* p, const char* owner, const char* file, int line, int* count, int* size);  // find a block
void fn_00F49060(void* block);                                                                     // free a block
}

template <class T>
void debug_delete(T* p, const char* owner, const char* file, int line)
{
    int size;
    int count;
    void* allocation = fn_00F49080(p, owner, file, line, &count, &size);
    if (allocation) {
        for (int i = 0; i < count; i++)
            p[i].~T();
        fn_00F49060(allocation);
    }
}

#define CA_DELETE(p) debug_delete(p, THIS_FILE, __FILE__, __LINE__)

class Landmark;
class Region;
class Road;
class SaveStream;

extern "C" {
const char* fn_0040E567(const char* file);      // file's path in the data directory
}

// An object's tile on the strategy map.
struct TilePosition {
    TilePosition() { x = 0; y = 0; }

    bool operator!=(const TilePosition& other) const { return !(*this == other); }
    bool operator==(const TilePosition& other) const
    {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }

    unsigned int x;
    unsigned int y;
};

// Something placed on the strategy map (0x24 bytes; not identified: its constructor and vtable come after
// code\battle_site_marker.cpp's).
class Character;
class Settlement;

enum {
    OBJECT_CHARACTER = 0x1a,
};

struct TileUnk84;

class StrategyObject {
public:
    StrategyObject() : m_next(0), m_8(0), m_14(0), m_18(true), m_1c(1.0f), m_20(false) {}  // 0x0048F9B5
    virtual ~StrategyObject() {}
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual int object_type();                                        // +0x10: OBJECT_CHARACTER for a character
    virtual void unk_14();
    virtual TileUnk84* unk_18();                                      // +0x18 (not identified)
    virtual void unk_1c();
    virtual Settlement* settlement();                                 // +0x20: itself if it's a settlement, or 0
    virtual void unk_24();
    virtual Character* unk_28();                                      // +0x28: itself if it's a character (a guess)
    virtual void unk_2c();
    virtual void unk_30();
    virtual void unk_34();
    virtual void unk_38();
    virtual void unk_3c();
    virtual void unk_40();
    virtual void unk_44();
    virtual void unk_48();
    virtual void unk_4c();
    virtual void unk_50();
    virtual void unk_54();
    virtual void unk_58();
    virtual void unk_5c();
    virtual void unk_60();
    virtual void unk_64();
    virtual void unk_68();
    virtual void unk_6c();
    virtual TilePosition* position();                                 // +0x70

    StrategyObject* m_next;                   // +0x04: the next object on its tile
    int m_8;
    TilePosition m_position;                  // +0x0c
    int m_14;
    bool m_18;
    float m_1c;
    bool m_20;
};

class Army;
class Unit;
class Faction;
class AISenateFaction;
class SenateMissionImpl;
template <class T> class LinkedList;
class Character;
class LandmarkManager;
class Settlement;
class CharacterRecord;
struct RebelType;
struct RecruitmentItems;
struct UnitUnk50;
class BuildingEffects;
class Fort;
struct RegionColour;
struct RoadPoint;
struct SettlementDamage;
struct SettlementUnk5B5975;
class String;

// A region's share of each religion (BI; 10 of them).
struct RegionReligions {
    int m_values[10];
};

struct Vector3 {
    Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}

    float m_x;
    float m_y;
    float m_z;
};

// (Not identified: told when a character's record changes.)
class UnkC0A37E {
public:
    void unk_00C097F2(int faction);     // 0x00C097F2
    void unk_00C0A37E(Character* character);  // 0x00C0A37E
    void unk_00C0A6AF(Faction* faction);  // 0x00C0A6AF
};

// (Not identified: a parameter of CharacterRecord::command with a large vtable.)
class BattleUnk {
public:
    virtual void unk_000();
    virtual void unk_004();
    virtual void unk_008();
    virtual void unk_00c();
    virtual void unk_010();
    virtual void unk_014();
    virtual void unk_018();
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
    virtual void unk_058();
    virtual void unk_05c();
    virtual void unk_060();
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
    virtual void unk_174();
    virtual void unk_178();
    virtual void unk_17c();
    virtual void unk_180();
    virtual void unk_184();
    virtual void unk_188();
    virtual void unk_18c();
    virtual void unk_190();
    virtual void unk_194();
    virtual void unk_198();
    virtual void unk_19c();
    virtual void unk_1a0();
    virtual void unk_1a4();
    virtual void unk_1a8();
    virtual void unk_1ac();
    virtual void unk_1b0();
    virtual void unk_1b4();
    virtual void unk_1b8();
    virtual void unk_1bc();
    virtual void unk_1c0();
    virtual void unk_1c4();
    virtual void unk_1c8();
    virtual void unk_1cc();
    virtual void unk_1d0();
    virtual void unk_1d4();
    virtual void unk_1d8();
    virtual void unk_1dc();
    virtual void unk_1e0();
    virtual void unk_1e4();
    virtual void unk_1e8();
    virtual void unk_1ec();
    virtual void unk_1f0();
    virtual void unk_1f4();
    virtual void unk_1f8();
    virtual void unk_1fc();
    virtual void unk_200();
    virtual void unk_204();
    virtual void unk_208();
    virtual void unk_20c();
    virtual void unk_210();
    virtual void unk_214();
    virtual void unk_218();
    virtual void unk_21c();
    virtual void unk_220();
    virtual void unk_224();
    virtual void unk_228();
    virtual void unk_22c();
    virtual void unk_230();
    virtual void unk_234();
    virtual void unk_238();
    virtual void unk_23c();
    virtual void unk_240();
    virtual void unk_244();
    virtual void unk_248();
    virtual void unk_24c();
    virtual void unk_250();
    virtual void unk_254();
    virtual void unk_258();
    virtual void unk_25c();
    virtual void unk_260();
    virtual void unk_264();
    virtual void unk_268();
    virtual void unk_26c();
    virtual void unk_270();
    virtual void unk_274();
    virtual void unk_278();
    virtual void unk_27c();
    virtual void unk_280();
    virtual void unk_284();
    virtual void unk_288();
    virtual void unk_28c();
    virtual void unk_290();
    virtual void unk_294();
    virtual void unk_298();
    virtual void unk_29c();
    virtual void unk_2a0();
    virtual void unk_2a4();
    virtual void unk_2a8();
    virtual void unk_2ac();
    virtual void unk_2b0();
    virtual void unk_2b4();
    virtual void unk_2b8();
    virtual void unk_2bc();
    virtual void unk_2c0();
    virtual void unk_2c4();
    virtual void unk_2c8();
    virtual void unk_2cc();
    virtual void unk_2d0();
    virtual void unk_2d4();
    virtual void unk_2d8();
    virtual void unk_2dc();
    virtual void unk_2e0();
    virtual void unk_2e4();
    virtual bool unk_2e8();             // +0x2e8
    virtual bool unk_2ec();             // +0x2ec
};

// A save game being written or read (probably ca_libs).
#pragma warning(push)
#pragma warning(disable: 4172)  // read_int_ref
class SaveStream {
public:
    virtual void unk_0();
    virtual void unk_4();
    virtual void fail();                     // +0x08 (a guess)
    virtual bool is_open();                  // +0x0c (a guess)
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual bool refill();                   // +0x1c: moves the buffer to take in the cursor (a guess)

    bool unk_00F3D990();                     // 0x00F3D990 (checked before writing)
    int tell();                              // 0x00F3D970: the position written to
    void seek_write(int position);           // 0x00F3D8E0
    void write(const void* data, int size);  // 0x00F3D8B0
    void read(int& value);                   // 0x00412D90
    void read(void* data, int size);         // 0x00412DD7
    void read(unsigned short& value);        // 0x004682A6

    template <class T>
    void write_value(const T& value)
    {
        write(&value, sizeof(T));
    }

    int read_int()
    {
        int value;
        read(value);
        return value;
    }

    // Returns a reference to its own local, as the original does.
    const int& read_int_ref()
    {
        int value;
        read(value);
        return value;
    }

    unsigned short read_ushort()
    {
        unsigned short value;
        read(value);
        return value;
    }

    // The position read from.
    int offset() { return m_cursor - m_base; }

    bool seek(int position)
    {
        m_cursor = m_base + position;
        if (m_cursor < m_begin || m_end <= m_cursor)
            return refill();
        return true;
    }

    char m_pad4[4];
    char* m_begin;                           // +0x08: the buffer's part of the file
    char* m_end;                             // +0x0c
    char* m_base;                            // +0x10: the file's start
    char m_pad14[4];
    char* m_cursor;                          // +0x18
};
#pragma warning(pop)

// Writes text without its terminator.
inline SaveStream& operator<<(SaveStream& stream, const char* text)
{
    const char* at = text;
    while (*at++)
        ;
    int length = at - text - 1;
    if (length)
        stream.write(text, length);
    return stream;
}

class Trigger;
template <class T>
class LinkedList;
class TriggerPlace;

// What the triggers are told of (code\triggers.cpp): an event, and what its triggers' conditions can ask about it.
// Each kind of event gives what it has; the rest give "none".
class TriggerEvent {
public:
    virtual ~TriggerEvent() {}
    virtual int unk_04() const;
    virtual int unk_08() const;
    virtual int unk_0c() const;
    virtual int unk_10() const;
    virtual CharacterRecord* character_record() const;  // +0x14: whom Affects and AcquireAncillary are for
    virtual CharacterRecord* unk_18() const;
    virtual int unk_1c() const;
    virtual TriggerPlace* settlement() const;           // +0x20
    virtual int unk_24() const;
    virtual TriggerPlace* fort() const;                 // +0x28 (a guess: UngarrisonedFort's events give one)
    virtual Faction* faction() const;                   // +0x2c
    virtual int unk_30() const;
    virtual int unk_34() const;
    virtual int unk_38() const;                         // -1 for none
    virtual int unk_3c() const;
    virtual int unk_40() const;
    virtual int unk_44() const;
    virtual int unk_48() const;
    virtual int unk_4c() const;
    virtual int unk_50() const;                         // 4 for none
    virtual const String& unk_54() const;               // an empty String for none
    virtual int unk_58() const;                         // 6 for none
    virtual int character_type() const;                // +0x5c: 6 for none
    virtual int unk_60() const;                         // 0x2f for none
    virtual int unk_64() const;                         // 4 for none
    virtual int unk_68() const;                         // 6 for none
    virtual int unk_6c() const;                         // 0xa5 for none
    virtual bool unk_70() const;
    virtual bool unk_74() const;
    virtual LinkedList<Trigger>* triggers() const = 0;  // +0x78: the triggers that test its kind of event
    virtual const void* unk_7c() const = 0;
};

void fire_triggers(const TriggerEvent& event);         // 0x008AAE20

// A failed check's message: the first returns true to stop in the debugger (probably ca_libs).
extern "C" bool fn_00F40080(int, const char* format, ...);
extern "C" bool fn_00F3F290();

#define CA_CHECK_FAILED(format)                                                                             \
    if (fn_00F40080(0, format) && fn_00F3F290())                                                            \
        __asm int 3
#define CA_CHECK_FAILED1(format, arg)                                                                       \
    if (fn_00F40080(0, format, arg) && fn_00F3F290())                                                       \
        __asm int 3

// The rounding the game uses for float to int (a math header's inline assembly).
inline int float_to_int(float value)
{
    __asm {
        push eax
        fld value
        frndint
        fistp dword ptr [esp]
        pop eax
    }
}

// The same, rounding toward zero (the same header's).
inline int float_to_int_chop(float value)
{
    int result;
    int control;
    int chop;
    __asm {
        push eax
        fld value
        fstcw word ptr control
        mov eax, control
        or eax, 0xc00
        mov chop, eax
        fldcw word ptr chop
        frndint
        fistp result
        fldcw word ptr control
        pop eax
    }
    return result;
}

// float_to_int_chop through one more inline (ArmySpyingInfo::estimate's).
inline int chop_to_int(float value)
{
    return float_to_int_chop(value);
}

// The larger and the smaller of two values (a header's templates, as the spying code expands them).
template <class T>
inline T max_of(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <class T>
inline T min_of(const T& a, const T& b)
{
    return a < b ? a : b;
}

// How many units a revolt raises, at least and at most, by settlement level.
struct RevoltSize {
    int m_min;
    int m_max;
};

// Something about a settlement that the notices are told of (not identified; 16 bytes).
struct UnkB5DC7E {
    UnkB5DC7E(Settlement* settlement);  // 0x00B5DC7E

    char m_pad0[0x10];
};

// The same for a fort (4 bytes).
struct UnkB5C51D {
    UnkB5C51D(Fort* fort);              // 0x00B5C51D

    int m_0;
};

// What lbl_02921A38 is (not identified).
class Unk02921A38 {
public:
    void unk_00C09FDE(Character* character);  // 0x00C09FDE
    void unk_00C0A4F4(UnkB5DC7E unk);   // 0x00C0A4F4
    void unk_00C0A592(UnkB5C51D unk);   // 0x00C0A592
    void unk_00C098DC(Settlement* settlement);  // 0x00C098DC: passes the settlement on, when some flag (0x00C803D8) allows
    void unk_00C0A2EE(Unit* unit);      // 0x00C0A2EE: the same for a unit
    void unk_00C0A689(Fort* fort);      // 0x00C0A689
};

extern "C" {
extern int lbl_01181378;  // the rebel faction's index
extern RevoltSize lbl_00FEC4E8[];    // by settlement level
extern const float lbl_00FE4970;  // 1.0

// Landmarks (code\landmark.cpp's callees)
int fn_00E3C9F6(const char* name);                     // a landmark type from its name
void fn_00CB5468(Faction* faction, Landmark* landmark, TilePosition* position, int);
void fn_00CB54E7(Faction* faction, Landmark* landmark, TilePosition* position);
unsigned char fn_009CB2A7(unsigned int x, unsigned int y);

extern Unk02921A38* lbl_02921A38;

class UnkB4CFDA {
public:
    Faction* unk_00B4CFDA();            // 0x00B4CFDA: the player's faction (a guess)
    int unk_00B4CFFE();                 // 0x00B4CFFE: its index (code\local_player.cpp)
};
extern UnkB4CFDA* lbl_018DC3A4;         // the local player (code\local_player.cpp)

inline Unk02921A38* unk_02921A38()
{
    Unk02921A38* unk = lbl_02921A38;
    return unk;
}

// (Not identified: told when a character enters a settlement or a fort.)
class Unk02921A18 {
public:
    void unk_004BE7CB(Settlement* settlement);  // 0x004BE7CB
    void unk_004BE81C(Fort* fort);      // 0x004BE81C
};
extern Unk02921A18* lbl_02921A18;

inline Unk02921A18* unk_02921A18()
{
    Unk02921A18* unk = lbl_02921A18;
    return unk;
}

// Armies (code\army.cpp's callees)
extern bool lbl_02921AE0;
float fn_00432C9F(Unit* unit);      // a unit's value in a battle
float fn_004335A9(Unit* unit);      // a unit's value on the campaign map, from its type, soldiers and m_298 slots
void fn_00CB67A8(Faction* faction); // a message (type 0x60) to the faction: it lost its eagle (a guess)
int fn_008F99BC(const RegionColour* colour);   // from the descr_regions.txt entry of this colour (+0x4c)

// Settlements' changes of owner (code\settlement.cpp's callees)
void fn_008B4D04(CharacterRecord* captor, Settlement* settlement, Faction* faction, Faction* old_owner, int reason);  // a message, when some flag allows
void fn_008B529F(Settlement* settlement, Faction* faction);  // a message, when some flag allows

// Settlements' riots and revolts (code\settlement.cpp's callees)
void fn_008B4820(Settlement* settlement, Faction* faction, Character* governor);  // fires CityRebels and GovernorCityRebels
void fn_00CB42BB(const SettlementDamage* damage, Faction* faction);  // the revolt's message (a guess)
void fn_00CB4122(const SettlementDamage* damage);    // the riot's message (a guess)
void fn_008FF5CB(Random* random, int faction, bool male, int* first_name, int* surname);  // picks a name
const char* fn_008FF4A3(int kind, int faction, int id);  // a saved name's key
Character* fn_008E1FA5(String* type, Faction* faction, int name_faction, int age, int first_name, int surname);  // a new general
Character* fn_008E212F(int type, Faction* faction, int age, const char* first_name, const char* surname, int name_faction);  // a new agent
void fn_00CBB9AE(Character* character, Faction* old_faction, Faction* new_faction);  // a message (a guess)
unsigned int fn_008D078B();                    // non-zero when religions are enabled (BI)
int* fn_008E695D(const String& name);          // the culture of this name, or 0
void fn_005F4FEB(int* religion, int* religion_effects, const SettlementUnk5B5975& unk, BuildingEffects* effects,
                 int, int);
extern void* lbl_015FC11C;         // the aerial map, when there is one

// Saved pointers (code\romans_file.cpp)
void fn_0041369B(void* object, int saved_address);         // gives a saved road pointer its object
void* fn_0041374F(void* saved);                             // a saved pointer's new object (type 0)
void fn_0041391C(SaveStream* stream);
bool fn_0041393B(SaveStream* stream, int);

// The aerial map's roads (probably code\display\aerial_map_roads.cpp)
int fn_009CA89F(int region, Road* road);
int fn_009CA94C(int region, Road* road);
void fn_009CAAA4(int handle, Road* road);
void fn_009CAB2E(int region, unsigned char level);
void fn_009CAB82(int handle, float alpha);
void fn_009CABA1(int handle, Array<RoadPoint>& points);

bool fn_00EFA483(int);
void fn_00425B4F();
void fn_00F32ED0(int);
void fn_00402805();
void fn_00F3A500(const char*);
bool fn_00408DCF();
int fn_00403412(HINSTANCE, HINSTANCE);
int fn_00425F8F(HINSTANCE, HINSTANCE);

extern char* lbl_012CA6B0;
extern bool lbl_012CB1F2;
extern char lbl_012CB1F4[];
extern int lbl_012CB0D8;
extern bool lbl_012CE880;
}

inline bool religions_enabled()
{
    return fn_008D078B() > 0;
}

#include "common_constants.h"

inline bool UniString::operator==(const char* text) const
{
    const unsigned short* p = chars();
    unsigned short c;
    while (UNI_NULL != *p && (c = *p, (unsigned char)*text == c)) {
        p++;
        text++;
    }
    unsigned short end = *p;
    return end == (unsigned char)*text;
}

// A character followed by a string.
inline UniString operator+(UniChar c, const UniString& text)
{
    return (UniString(text.length() + 1) = UniString(c)).append(text);
}

#endif

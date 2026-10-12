// Declarations whose source file is not known yet. Move each to its own header once its home is found.
// Unnamed functions and variables keep the names tools/split.py gives them (fn_/lbl_ and their address), declared
// extern "C" so that they compile to those names.
#ifndef UNSORTED_H
#define UNSORTED_H

#include <windows.h>
#include <new.h>

#include "for_scope.h"

// A string of 16-bit characters (probably ca_libs\core\core_string.cpp).
struct UniChar;

class UniString {
public:
    UniString();                                // 0x00F34F70
    UniString(const char* text);                // 0x00F338B0
    UniString(const UniChar& c);                // 0x00F33F30
    UniString(const UniString& other);          // 0x00F34BE0
    explicit UniString(int capacity);           // 0x00F33380 (a guess)
    ~UniString();                               // 0x00F35440
    unsigned short length() const;              // 0x00F38200
    void truncate(unsigned short length);       // 0x00F37C00
    const unsigned short* chars() const;        // 0x00F37A20
    UniString& operator=(const UniString& other);   // 0x00F34B90
    UniString operator+(const UniString& other) const;  // 0x00F37360
    UniString& append(const UniString& other);  // 0x00F367E0
    UniString& operator+=(const UniString& other) { return append(other); }
    UniString& operator+=(const UniChar& c);    // 0x00F36C10
    bool operator==(const char* text) const;    // 0x00422DE4
    bool operator!=(const char* text) const { return !(*this == text); }

private:
    void* m_data;
};

// A list of pointers (12 bytes: its items, its capacity and its count; the fields' order is a guess).
template <class T>
class PointerList {
public:
    unsigned int size() const { return m_count; }

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

// (Not matching yet: the original builds find's result in place and has five more unused slots, as if VC6 had
// started to inline find.)
template <class T>
T* Array<T>::add_unique(const T& item)
{
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

// Each part of a save game has a version, registered at startup (code\version_registry.cpp).
class VersionRegistry {
public:
    static VersionRegistry* instance();                     // 0x0042654A
    static void add(const char* name, int version);         // 0x004265D2
    int version(const char* name);                          // 0x00426599: -1 if not registered
};

inline int saved_version(const char* name)
{
    return VersionRegistry::instance()->version(name);
}

struct VersionRegistrar {
    VersionRegistrar(const char* name, int version) { VersionRegistry::add(name, version); }
};

class Landmark;
class Region;
class Road;
class SaveStream;

// An 8-bit string (probably ca_libs).
class String {
public:
    String() { m_chars = 0; m_4 = 0; }
    explicit String(const char* text)
    {
        m_chars = 0;
        m_4 = 0;
        *this = text;
    }
    String(const String& other);                // 0x00E2C2F2
    ~String();                                  // 0x00E2C38E
    const char* c_str() const { return m_chars; }
    bool is_set() const { return m_chars != 0; }
    void clear();                               // 0x00E2C45F
    void assign(const char* text, int length);  // 0x00E2C56A
    String& operator=(const String& other);     // 0x00E2C673
    String& operator=(const char* text);        // 0x00E2C4EC
    bool operator==(const String& other) const; // 0x00439C41
    bool operator!=(const String& other) const; // 0x00407CD4
    void set_scratch(const char* text);         // 0x00E2C5C5: text in a shared buffer, to look up (a guess)
    void detach();                              // 0x00E2C4D6: forgets its characters without freeing them
    void save(SaveStream& stream);              // 0x00E2C836
    void load(SaveStream& stream);              // 0x00E2C8CD

    char* m_chars;
    int m_4;
};

// Reads a descr_*.txt script, a line at a time (probably ca_libs, beside String).
class ScriptReader {
public:
    ScriptReader(const UniString& path, int);   // 0x00E2C9C5
    ~ScriptReader();                            // 0x00E2CB52
    bool is_open() const { return m_data != 0; }
    bool at_end() const { return m_cur >= m_end; }
    const char* cur() const { return m_cur; }
    const char* path();                         // 0x00E2C950
    void position(int* line, int* column);      // 0x00E2DCD3
    bool match(const char* word);               // 0x00E2D1D1: the next word is word
    bool skip_word();                           // 0x00E2CEA5: false at the end of the line
    int word_length();                          // 0x00E2D0D4
    const char* word();                         // 0x00E2D501: the current word
    int read_int();                             // 0x00E2D3F0
    short read_short();                         // 0x00E2D408
    bool at_number();                           // 0x00E2DA40: the current word starts with a digit or "-digit"
    void next_line();                           // 0x00E2CF33

    char* m_data;
    char* m_end;
    int m_8;
    char* m_cur;
    int m_10;
    int m_14;
};

extern "C" {
const char* fn_0040E567(const char* file);      // file's path in the data directory
extern bool lbl_012CA5B8;                       // a script error has been reported
extern char lbl_010B3928[];                     // the script error's text
void fn_00F40060(int);
void fn_00F401D0(const char* format, ...);
}

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

// Reports an error in a script (once a run). The message is format, given arg.
#define SCRIPT_ERROR(reader, format, arg)                                                                   \
    if (!lbl_012CA5B8) {                                                                                    \
        int line;                                                                                           \
        int col;                                                                                            \
        (reader).position(&line, &col);                                                                     \
        sprintf(lbl_010B3928, "Script Error in %s, at line %i, column %i.\n\n" format, (reader).path(), line, \
                col, arg);                                                                                  \
        lbl_012CA5B8 = true;                                                                                \
        fn_00F40060(0);                                                                                     \
        fn_00F401D0("%s", lbl_010B3928);                                                                    \
    }

// The same, given two args.
#define SCRIPT_ERROR2(reader, format, arg1, arg2)                                                           \
    if (!lbl_012CA5B8) {                                                                                    \
        int line;                                                                                           \
        int col;                                                                                            \
        (reader).position(&line, &col);                                                                     \
        sprintf(lbl_010B3928, "Script Error in %s, at line %i, column %i.\n\n" format, (reader).path(), line, \
                col, arg1, arg2);                                                                           \
        lbl_012CA5B8 = true;                                                                                \
        fn_00F40060(0);                                                                                     \
        fn_00F401D0("%s", lbl_010B3928);                                                                    \
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
class StrategyObject {
public:
    StrategyObject() : m_4(0), m_8(0), m_14(0), m_18(true), m_1c(1.0f), m_20(false) {}  // 0x0048F9B5
    virtual ~StrategyObject() {}
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
    virtual void unk_20();
    virtual void unk_24();
    virtual void unk_28();
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

    int m_4;
    int m_8;
    TilePosition m_position;                  // +0x0c
    int m_14;
    bool m_18;
    float m_1c;
    bool m_20;
};

class Army;
class Unit;

// What a settlement's +0xdd8 is (not identified).
class SettlementDD8 {
public:
    void unk_005C8729(bool);            // 0x005C8729
};

// What a new unit's soldiers start with in a settlement (not identified; 0x30 bytes).
struct UnitUnk4D469D {
    UnitUnk4D469D();                    // 0x004D469D

    char m_pad0[0x1c];
    unsigned int m_1c;                  // +0x1c
    char m_pad20[4];
    int m_24;                           // +0x24
    char m_pad28[0x34 - 0x28];
};

// A settlement (probably code\settlement.cpp).
class Settlement : public StrategyObject {
public:
    int faction() const { return m_faction; }
    Army* garrison() const { return unk_40(); }
    Army* unk_40() const { return m_garrison; }

    bool unk_005BA3F0(TilePosition* position);              // 0x005BA3F0
    int unk_005BB0ED(Unit* unit, UnitUnk4D469D& experience);  // 0x005BB0ED

    char m_pad24[0x40 - 0x24];
    Army* m_garrison;                   // +0x40 (a guess)
    char m_pad44[0xa0 - 0x44];
    int m_faction;                      // +0xa0
    char m_pada4[0x1d0 - 0xa4];
    int m_wonder_order;                 // +0x1d0: public order for capturing a wonder, 4 then down by 1 a turn
    char m_pad1d4[0xdd8 - 0x1d4];
    SettlementDD8 m_dd8;                // +0xdd8
};

// A date of the campaign: a year and a season.
struct Date {
    Date() { m_year = 0; m_season = 2; }
    int season() const;                         // 0x0043C687: 2 in summer, 0 in winter (a guess)

    int m_year;
    int m_season;
};

struct Vector3 {
    Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}

    float m_x;
    float m_y;
    float m_z;
};

// A tile of the strategy map, by its coordinates (y from the south).
struct RoadPoint {
    RoadPoint() { x = 0; y = 0; }
    bool operator==(const RoadPoint& other) const
    {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }
    bool operator!=(const RoadPoint& other) const { return !(*this == other); }

    int x;
    int y;
};

// A region's entry for one of its neighbours (0x54 bytes).
struct RegionNeighbour {
    char m_pad0[4];
    int m_region;                       // +0x04
    char m_pad8[0x10 - 8];
    Road* m_road;                       // +0x10: the road this region planned towards it
    char m_pad14[0x54 - 0x14];
};

// A region's port (probably code\port.cpp).
struct PortUnkA98 {
    int unk_78() const { return m_78; }

    char m_pad0[0x78];
    int m_78;                           // +0x78
};

struct Port {
    int unk_184() const { return m_184; }
    PortUnkA98* unk_a98() const { return m_a98; }

    char m_pad0[0x184];
    int m_184;                          // +0x184
    char m_pad188[0xa98 - 0x188];
    PortUnkA98* m_a98;                  // +0xa98
};

// One tile of the strategy map's grid (0x34 bytes).
struct Tile {
    char m_pad0[4];
    Road* m_road;                       // +0x04: the road drawn on it
    char m_pad8[0x10 - 8];
    int m_10;                           // +0x10
    int m_region;                       // +0x14
    char m_pad18[4];
    unsigned int m_flags0 : 5;          // +0x1c
    unsigned int m_river_source : 1;
    unsigned int m_river : 1;
    unsigned int m_flags7 : 5;
    unsigned int m_road_level : 3;      // 4 for a settlement's tile (a guess)
    unsigned int m_flags15 : 2;
    unsigned int m_landmark : 1;        // a landmark stands on it
    unsigned int m_flags18 : 14;
    char m_pad20[4];
    unsigned char m_24;                 // +0x24
    unsigned char m_25;
    char m_pad26[2];
    char m_pad28[0x34 - 0x28];
};

// A region of the strategy map (0x1d8 bytes).
class Region {
public:
    int id() const { return m_id; }
    unsigned char road_level() const { return m_road_level; }
    Port* port() const { return m_port; }
    int num_neighbours() const { return m_neighbours.size(); }
    void set_port_road(Road* road) { m_port_road = road; }
    int unk_68() const { return m_68; }
    void unk_00483E16(unsigned char level);  // 0x00483E16
    void unk_00483E6B();                     // 0x00483E6B
    void unk_00484763();                     // 0x00484763

    char m_pad0[0x1c];
    int m_id;                           // +0x1c
    unsigned char m_road_level;         // +0x20: 0 none, 1 roads, 2 paved roads, 3 highways
    char m_pad21[0x58 - 0x21];
    Road* m_port_road;                  // +0x58
    char m_pad5c[0x68 - 0x5c];
    int m_68;                           // +0x68
    char m_pad6c[0x148 - 0x6c];
    Port* m_port;                       // +0x148
    Array<int> m_tiles;                 // +0x14c: its tiles, by index into the map's grid
    char m_pad158[0x164 - 0x158];
    Array<RegionNeighbour> m_neighbours;  // +0x164: in the order the map's scan first finds them
    char m_pad170[0x194 - 0x170];
    RoadPoint m_settlement_position;    // +0x194
    RoadPoint m_port_position;          // +0x19c
    char m_pad1a4[0x1a8 - 0x1a4];
    Settlement* m_settlement;           // +0x1a8
    char m_pad1ac[0x1d8 - 0x1ac];
};

// A set of factions, a bit each.
class FactionSet {
public:
    int bits() const { return m_bits; }
    unsigned int count() const
    {
        unsigned int bits = m_bits;
        unsigned int total = 0;
        while (bits) {
            if (bits & 1)
                total++;
            bits >>= 1;
        }
        return total;
    }

    int m_bits;
};

// A faction's entry in descr_sm_factions (probably code\faction_db.cpp).
struct FactionDescription40 {
    int m_0;                            // a culture? (a guess)
};

class FactionDescription {
public:
    bool has_18() const { return m_18 < 0xffffffff; }
    bool unk_a7() const { return m_a7; }

    char m_pad0[0x18];
    unsigned int m_18;                  // +0x18 (0xffffffff for most factions; not identified)
    char m_pad1c[0x40 - 0x1c];
    FactionDescription40* m_40;         // +0x40
    char m_pad44[0xa4 - 0x44];
    bool m_custom_battle_availability;  // +0xa4
    char m_pada5[2];
    bool m_a7;                          // +0xa7
};

class Character;
class CharacterRecord;
class TriggerEvent;

// What a faction's +0x180 is (not identified).
class FactionUnk180 {
public:
    void unk_00492498(Character* character, bool);  // 0x00492498
};

class AncillaryDescription;

// A faction's AI (not identified).
class FactionAI {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();
};

// A faction's campaign record (probably code\faction.cpp).
class Faction {
public:
    int unk_178() const { return m_178; }
    unsigned int num_regions() const { return m_regions.size(); }
    FactionSet neighbours() const { return m_neighbours; }
    bool no_cash_crisis() const { return m_4ec == 0; }  // (what +0x4ec means is not known)
    bool has_cash_crisis() const { return !no_cash_crisis(); }
    bool unk_4f4() const { return m_4f4; }
    int id() const { return m_id; }
    int culture() const { return m_culture; }
    FactionDescription* description() const { return m_description; }
    bool is_unk_a7() const { return m_description->m_a7; }
    FactionUnk180* unk_180() const { return m_180; }

    bool unk_00445B2B();                // 0x00445B2B: description's +0xa4 set and +0xa7 clear
    bool unk_004584F1();                // 0x004584F1: has_cash_crisis() or +0x4f0 is a faction
    Settlement* unk_004C44F0(CharacterRecord* father);  // 0x004C44F0
    int unk_004C4589(CharacterRecord* record);          // 0x004C4589
    void unk_004C4CE7(CharacterRecord* record);         // 0x004C4CE7
    void unk_004CA5BA(Army* army);                      // 0x004CA5BA
    void unk_004CB391();                                // 0x004CB391: works out m_neighbours (a guess)

    char m_pad0[0xa0];
    int m_id;                           // +0xa0
    int m_culture;                      // +0xa4
    char m_pada8[4];
    CharacterRecord* m_leader;          // +0xac
    char m_padb0[4];
    FactionDescription* m_description;  // +0xb4
    int m_control;                      // +0xb8 (1: human)
    FactionAI* m_ai;                    // +0xbc
    char m_padc0[0x100 - 0xc0];
    PointerList<Region> m_regions;      // +0x100
    char m_pad10c[0x13c - 0x10c];
    FactionSet m_neighbours;            // +0x13c: the factions whose regions border this one's
    char m_pad140[0x178 - 0x140];
    int m_178;                          // +0x178
    char m_pad17c[4];
    FactionUnk180* m_180;               // +0x180
    char m_pad184[0x4ec - 0x184];
    int m_4ec;                          // +0x4ec
    int m_4f0;                          // +0x4f0 (a faction index, 21 for none)
    bool m_4f4;                         // +0x4f4
    char m_pad4f5[0x504 - 0x4f5];
    int m_treasury;                     // +0x504
};

// The C runtime's rand(), kept per object (probably ca_libs).
class Random {
public:
    int percent();                      // 0x00F3AAD0: 0 to 99
    int range(int low, int high);       // 0x00F3ABB0

    int m_seed;
};

// How one faction stands towards another.
class DiplomaticRelation {
public:
    char m_pad0[4];
    int m_stance;                       // +0x04: 0 allied, 600 at war
    char m_pad8[0x74 - 8];
};

// One faction's relations with every faction.
class FactionRelations {
public:
    int stance(int other) const { return m_relations[other].m_stance; }

    DiplomaticRelation m_relations[21];
};

// Every pair of factions' relations (probably code\faction_diplomacy.cpp).
class DiplomacyMatrix {
public:
    int stance(int faction, int other) const { return m_factions[faction].stance(other); }
    bool is_at_war(int faction);        // 0x005A11E9: at war with anyone

    FactionRelations m_factions[21];
};

// The campaign (probably code\campaign.cpp).
class Campaign {
public:
    Faction* faction(int index);        // 0x004548DD
    int difficulty(int faction);        // 0x004529E3: 0 easy to 3 very hard; always 1 for the AI
    bool is_faction_alive(int faction); // 0x0045491C
    int human_faction(int index);       // 0x00457158: the index'th human faction, or 21
    bool is_prologue() const { return m_prologue; }
    int unk_2e4() const { return m_date.m_year; }
    int unk_4() const { return m_4; }
    bool unk_00458257(AncillaryDescription* ancillary);  // 0x00458257: whether someone has it (a guess)
    int unk_b8() const { return m_b8; }
    unsigned int num_factions() const { return m_num_factions; }
    Faction* faction_at(unsigned int index) const { return m_factions[index]; }
    Faction* unk_22c() const { return m_22c; }
    void unk_004531EF(const TriggerEvent& event);       // 0x004531EF: told of each event before the triggers
    void unk_00452674();                                // 0x00452674: before a load
    bool unk_00454CE1(SaveStream& stream);              // 0x00454CE1: loads the campaign
    bool unk_00455C12(SaveStream& stream);              // 0x00455C12: saves the campaign
    void unk_004562B8();                                // 0x004562B8: after a load

    char m_pad0[4];
    int m_4;                            // +0x04
    char m_pad8[0xb8 - 8];
    int m_b8;                           // +0xb8 (1: the advisor's triggers run)
    char m_padbc[0x168 - 0xbc];
    Faction* m_factions[42];            // +0x168 (how many is a guess)
    unsigned int m_num_factions;        // +0x210
    char m_pad214[0x22c - 0x214];
    Faction* m_22c;                     // +0x22c
    char m_pad230[0x2b5 - 0x230];
    bool m_prologue;                    // +0x2b5 (a guess: it lowers brigand spawns and changes taxes and garrisons)
    char m_pad2b6[0x2e4 - 0x2b6];
    Date m_date;                        // +0x2e4 (its year is compared with 100; perhaps the turn number)
    char m_pad2ec[0x528 - 0x2ec];
    Random m_random;                    // +0x528
    DiplomacyMatrix m_diplomacy;        // +0x52c
    char m_padcd00[0xcd4c - 0xcd00];
    char m_road_manager[0x10];          // +0xcd4c: a RoadManager (road.h's campaign_roads())
};

// The strategy map (probably code\strategy_map.cpp).
class StrategyMap {
public:
    int num_regions() const { return m_region_list.size(); }
    unsigned int width() const { return m_width; }
    unsigned int height() const { return m_height; }
    Tile* tile(int x, int y) { return &m_tiles[x + y * m_width]; }
    Tile* tile(int index) { return &m_tiles[index]; }
    Tile* tile(RoadPoint point) { return &m_tiles[point.x + point.y * m_width]; }
    Tile* tile(TilePosition position) { return &m_tiles[position.x + position.y * m_width]; }
    Region* region(int id) { return &m_regions[id]; }
    void add_road(int region, int other, Road* road);  // 0x004812B2
    void add_landmark(int region, Landmark* landmark); // 0x00481251
    void unk_0048204D(Army* army, int region);         // 0x0048204D
    Region* unk_00482A48(TilePosition* position);      // 0x00482A48
    void unk_0047FC44(int region);                     // 0x0047FC44
    bool unk_00481149(int region);                     // 0x00481149: the region's +0x50
    void unk_0048257A(int region);                     // 0x0048257A
    void unk_00482B16(int region);                     // 0x00482B16

    char m_pad0[0x2c];
    int m_width;                        // +0x2c
    int m_height;                       // +0x30
    char m_pad34[0x48 - 0x34];
    Tile* m_tiles;                      // +0x48: m_width * m_height, by rows from the south
    char m_pad4c[0x70 - 0x4c];
    Region m_regions[199];              // +0x70 (how many is a guess)
    char m_pad16f58[0x17128 - 0x16f58];
    PointerList<Region> m_region_list;  // +0x17128 (a guess: only its count, +0x17130, is known)
};

// The campaign map's path planning (code\strategy_map.cpp).
class PathFinder {
public:
    int find_road_path(const RoadPoint& from, const RoadPoint& to);  // 0x004AE64B
    void plan_roads(int region, unsigned char level);               // 0x004B1B5B
    void build_roads(int region, unsigned char level);              // 0x004B21E2
    bool unk_004A605B(const RoadPoint& point);                      // 0x004A605B
    Army* unk_004AAC55(TilePosition* position);                     // 0x004AAC55
    int unk_004A7EA9(Settlement* settlement, RoadPoint* point);     // 0x004A7EA9
    void unk_004ABED9(StrategyObject* object, TilePosition* position);  // 0x004ABED9
    unsigned char unk_004B3124(TilePosition position);              // 0x004B3124
    void unk_004AC914(const RoadPoint& point);                      // 0x004AC914
    void unk_004B64C3(int);                                         // 0x004B64C3
    void unk_004A4E73();                                            // 0x004A4E73: before a load
    void unk_004B3011();                                            // 0x004B3011
    void unk_004AFADF();                                            // 0x004AFADF

    char m_pad0[0x14];
    int m_status;                       // +0x14: of the last search (0 found, 1 bad goal, 2 no path)
    char m_pad18[0xbc - 0x18];
    Array<RoadPoint> m_path;            // +0xbc: the last path found, goal first
};

// A node of the A* search.
struct PathNode {
    char m_pad0[0x10];
    RoadPoint m_point;                  // +0x10
    PathNode* m_parent;                 // +0x18
};

// The campaign map's A* search (code\strategy_map_pathfinding.cpp).
class AStar {
public:
    PathNode* search(int mode, const RoadPoint& from, const RoadPoint& to, bool diagonal);  // 0x004B84E5
};

// A unit's soldier (4 bytes of bitfields; not identified).
struct UnitSoldier {
    unsigned int m_0 : 23;
    unsigned int m_23 : 3;              // experience? (a guess)
    unsigned int m_26 : 3;
    unsigned int m_29 : 3;
};

struct UnitUnk50 {
    char m_pad0[0x2c];
    int m_2c;                           // +0x2c
};

// A unit of an army (probably code\unit.cpp).
class Unit {
public:
    UnitUnk50* unk_50() const { return m_50; }
    int unk_278() const { return m_278; }
    unsigned int num_soldiers() const { return m_num_soldiers; }

    void unk_007C28A2();                // 0x007C28A2
    void unk_007C2E5E();                // 0x007C2E5E

    char m_pad0[0x50];
    UnitUnk50* m_50;                    // +0x50
    char m_pad54[0x278 - 0x54];
    int m_278;                          // +0x278: its general? (a guess)
    float m_27c;                        // +0x27c
    unsigned int m_num_soldiers;        // +0x280
    char m_pad284[0x30c - 0x284];
    UnitSoldier* m_soldiers;            // +0x30c
};

// Who an army belongs to (an army's +0xc; not identified).
class ArmyOwner {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual Faction* faction();         // +0x08
};

// An army's first 0xc bytes (not identified).
class Army0 {
public:
    virtual void unk_00();
    char m_pad4[8];
};

// An army (probably code\army.cpp).
class Army : public Army0, public ArmyOwner {
public:
    Army(Character* character, int);    // 0x00719056
    int num_units() const { return m_units.size(); }
    Unit* unit_at(int index) { return m_units[index]; }
    int unk_134() const { return m_134; }

    TilePosition* unk_00719BB2();       // 0x00719BB2
    Unit* unk_0071DADD(Character* character, int, int, int, int);  // 0x0071DADD
    void unk_007201B3(int, int);        // 0x007201B3
    Unit* unit(int index);              // 0x0072212A
    int unk_007225D5(bool, bool);       // 0x007225D5: how many units are ... (not identified)
    bool unk_007233E0(bool);            // 0x007233E0
    void unk_00724043(int, int, int, int);  // 0x00724043
    void unk_00725C23(Army* army);      // 0x00725C23

    char m_pad10[0x5c - 0x10];
    Array<Unit*> m_units;               // +0x5c
    char m_pad68[0x134 - 0x68];
    int m_134;                          // +0x134
    char m_pad138[0x16c - 0x138];
};

// One army of a battle's side (0x6c bytes).
struct BattleSideArmy {
    Army* m_army;
    char m_pad4[0x6c - 4];
};

// A side of a battle (not identified).
class BattleSide {
public:
    bool unk_0() const { return m_0; }
    Army* army(int index) { return m_armies[index].m_army; }
    unsigned int num_armies() const { return m_1b58; }
    unsigned int num_reinforcements() const { return m_1bfc; }

    bool m_0;                           // +0x00: attacking? (a guess)
    char m_pad1[0x5c - 1];
    BattleSideArmy m_armies[63];        // +0x5c
    char m_pad1af0[0x1b58 - 0x1af0];
    unsigned int m_1b58;                // +0x1b58
    char m_pad1b5c[0x1bfc - 0x1b5c];
    unsigned int m_1bfc;                // +0x1bfc
};

// The battle being fought (not identified).
class Battle {
public:
    int unk_8() const { return m_8; }
    bool unk_c() const { return m_c; }
    bool unk_24() const { return m_24; }

    int side_of(Army* army);            // 0x005FF179: -1 for neither
    BattleSide* side(int index);        // 0x005FF3BB

    char m_pad0[8];
    int m_8;                            // +0x08
    bool m_c;                           // +0x0c
    char m_padd[0x24 - 0xd];
    bool m_24;                          // +0x24
};

// (Not identified: told when a character's record changes.)
class UnkC0A37E {
public:
    void unk_00C097F2(int faction);     // 0x00C097F2
    void unk_00C0A37E(Character* character);  // 0x00C0A37E
};

// A list of names being built (not identified).

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

// What the AI knows about each faction (an AI unit's globals).
struct AIFactionSummary {
    int m_value;                        // positive when the faction is a threat to someone (not identified)
    char m_pad4[0x40 - 4];
};

struct AIFactionPair {
    bool m_strongest_neighbour;         // set in 0x004E18FA
    char m_pad1[0xc - 1];
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

// An event for the triggers (not identified; code\landmark.cpp makes one from a port's +0xa98 when a wonder is
// captured).
class UnkMessage : public TriggerEvent {
public:
    UnkMessage(int);                    // 0x008A4067
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    int m_4;
};

extern "C" {
extern Campaign* lbl_018CD818;
extern StrategyMap* lbl_018CD80C;
extern int lbl_01181378;  // the rebel faction's index
extern AIFactionSummary lbl_012D0F78[21];
extern AIFactionPair lbl_012D1552[21 * 21];
extern const float lbl_00FE4970;  // 1.0

int fn_004E335E(int faction);

// Landmarks (code\landmark.cpp's callees)
int fn_00E3C9F6(const char* name);                     // a landmark type from its name
void fn_00CB5468(Settlement* settlement, Landmark* landmark, TilePosition* position, int);
void fn_00CB54E7(Settlement* settlement, Landmark* landmark, TilePosition* position);
unsigned char fn_009CB2A7(unsigned int x, unsigned int y);

extern StrategyMap* lbl_018CD86C;  // the strategy map again (the one movement uses?)
extern StrategyMap* lbl_02921A94;  // the strategy map again (the aerial map's copy?)
extern PathFinder* lbl_029219E0;
extern AStar* lbl_012D07C0;
extern int lbl_012D0604;           // the movement rules in force (3 while planning roads)
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

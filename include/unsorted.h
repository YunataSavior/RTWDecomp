// Declarations whose source file is not known yet. Move each to its own header once its home is found.
// Unnamed functions and variables keep the names tools/split.py gives them (fn_/lbl_ and their address), declared
// extern "C" so that they compile to those names.
#ifndef UNSORTED_H
#define UNSORTED_H

#include <windows.h>
#include <new.h>

#include "for_scope.h"
#include "maths_random.h"

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
    T* operator[](unsigned int index) const { return m_items[index]; }

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
    float read_float();                         // 0x00E2D468
    const char* rest_of_line();                 // 0x00E2D6E9: to the end of the line, less its trailing spaces
    bool at_number();                           // 0x00E2DA40: the current word starts with a digit or "-digit"
    bool next_line();                           // 0x00E2CF33: false at the end of the file (a guess)

    char* m_data;
    char* m_end;
    int m_8;
    char* m_cur;
    int m_10;
    int m_14;
};

// Skips lines to the first that starts with key, then skips key; false when there's none.
bool find_key(ScriptReader& reader, const char* key);  // 0x00445BA4

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

// A settlement's figures for a turn (0x1c0 bytes): its growth factors and its public order, each as the lines of the
// breakdown the player sees.
// A region's share of each religion (BI; 10 of them).
struct RegionReligions {
    int m_values[10];
};

// The growth factors (SettlementDD8::compute_growth); each point is 0.5% a turn.
enum {
    GROWTH_FERTILITY = 0,               // the region's, from descr_regions.txt
    GROWTH_FARMING = 1,                 // the buildings' farming_level and the governor's Farming, 0-25
    GROWTH_GRAIN = 2,                   // 3 for each of the region's grain resources, and then the governor's
                                        //   GrainTrading
    GROWTH_HEALTH = 3,                  // the buildings' population_health_bonus and the governor's Health, 0-25
    GROWTH_BONUS = 4,                   // the buildings' population_growth_bonus
    GROWTH_LOW_TAXES = 5,               // 1 at low taxes
    GROWTH_SLAVES = 7,                  // the region's slaves resources, and then the governor's SlaveTrading
    GROWTH_SQUALOR = 8,                 // 0-50
    GROWTH_PLAGUE = 9,                  // 20 while it has plague
    GROWTH_HIGH_TAXES = 10,             // 1 at high taxes, 2 at very high
};

// The resources of descr_sm_resources.txt, by their IDs (0x00E3CAFB gives them).
enum {
    RESOURCE_GRAIN = 4,
    RESOURCE_SLAVES = 9,
};

struct SettlementStats {
    int growth(int factor) const { return m_growth[factor]; }
    const int& growth_ref(int factor) const { return m_growth[factor]; }
    void set_growth(int factor, int value) { m_growth[factor] = value; }
    Settlement* settlement() const { return m_settlement; }
    int order(int slot) const { return m_order[slot]; }
    void apply_growth(int faction, int level);  // 0x005C7DED

    Settlement* m_settlement;           // +0x00
    int m_growth[16];                   // +0x04: factors 0-7 add to growth, 8-15 take from it (GROWTH_*)
    unsigned int m_population;          // +0x44
    int m_order[25];                    // +0x48: in units of 5%; slots 0-9 add to order, 10-24 take from it
    unsigned int m_order_percent;       // +0xac
    char m_padb0[0xf8 - 0xb0];
    RegionReligions m_religion;         // +0xf8: the region's (BI), copied in each time
    char m_pad120[0x1c0 - 0x120];
};

// What the religion slots of the public order come from (BI; 0x30 bytes).
struct SettlementReligion {
    SettlementReligion();               // 0x005CF3B4

    int m_values[10];
    int m_28;                           // +0x28
    int m_2c;                           // +0x2c
};

// Building effects, by the index of their names in export_descr_buildings.txt's list (0xFEC790).
enum {
    EFFECT_POPULATION_GROWTH_BONUS = 0,
    EFFECT_POPULATION_HEALTH_BONUS = 2,
    EFFECT_FARMING_LEVEL = 8,
    EFFECT_ARMOUR = 14,
    EFFECT_STAGE_GAMES = 15,
    EFFECT_STAGE_RACES = 16,
    EFFECT_WEAPON_SIMPLE = 18,          // then missile, bladed, siege and other
    EFFECT_RECRUITS_EXP_BONUS = 25,
    EFFECT_HAPPINESS_BONUS = 26,
    EFFECT_LAW_BONUS = 27,
    NUM_EFFECTS = 36,
};

// One building effect's value: a level, and a bonus that counts only when the level isn't negative (a guess).
struct BuildingEffect {
    BuildingEffect() {}
    short total() const;                // 0x005D4BE6

    short m_level;
    short m_bonus;                      // +0x02
};

// A unit type a settlement's buildings can train, and the experience they give it.
struct BuildingRecruit {
    int m_unit;                         // its index in export_descr_unit.txt
    BuildingEffect m_experience;        // +0x04
};

// A settlement's building effects (0x308 bytes).
class BuildingEffects {
public:
    BuildingEffects();                  // 0x005D451D
    short unk_005D48AE(int effect);     // 0x005D48AE
    BuildingEffect effect(int effect);  // 0x005D4973: none (0, 0) for an ID from 36 up
    unsigned char unk_005D48F2(int effect);  // 0x005D48F2: the effect's total, from 0 to 255
    unsigned int num_recruits();        // 0x005D4ADF
    BuildingRecruit* recruit(BuildingRecruit* out, unsigned int index);  // 0x005D4AF1: none (unit count, -1) past the end

    char m_pad0[0xb4];
    BuildingRecruit m_recruits[0x40];   // +0xb4: the units its buildings can train
    unsigned int m_num_recruits;        // +0x2b4
    char m_pad2b8[0x308 - 0x2b8];
};

// What a settlement's +0xdd8 is: its turn's figures and how they're worked out (code\settlement_mechanics.cpp).
class SettlementDD8 {
public:
    void start_turn();                  // 0x005C865A
    unsigned int unk_005C8AD7();        // 0x005C8AD7: the population a riot can't take it below
    void unk_005C8CE1(int);             // 0x005C8CE1
    void unk_005C8EFF(unsigned int population);  // 0x005C8EFF: population lost (one of three causes; not identified)
    void unk_005C8F38(unsigned int population);  // 0x005C8F38: population lost to a riot or a revolt
    void unk_005C8F71(unsigned int population);  // 0x005C8F71: population lost (not identified)
    void update(bool mark_owner);       // 0x005C8729: works out m_current
    void set_faction(Faction* faction); // 0x005C8A72
    void compute_growth(SettlementStats* stats, BuildingEffects* effects);  // 0x005CC583
    void compute_order(SettlementStats* stats, BuildingEffects* effects, int* religion, int* religion_effects);  // 0x005CC9D7
    void unk_005CDB6A(SettlementStats* stats, BuildingEffects* effects);  // 0x005CDB6A: fills +0xb0 (income? a guess)
    void unk_005CE3E8(SettlementStats* stats, BuildingEffects* effects);  // 0x005CE3E8: BI's religion
    LandmarkManager* landmarks();       // 0x00B4E286

    Faction* m_faction;                 // the owner
    Settlement* m_settlement;           // +0x04
    int m_8;                            // +0x08: from the region's descr_regions.txt entry (not identified)
    int m_rebel_chance;                 // +0x0c: the chance of the region's rebel type in descr_rebel_factions.txt
    char m_pad10[0x54 - 0x10];
    SettlementStats m_last;             // +0x54: last turn's
    SettlementStats m_current;          // +0x214
    bool m_3d4;                         // +0x3d4: cleared by each update
};

// What a settlement's +0x184 is (not identified; its port?).
class SettlementUnk184 : public StrategyObject {
public:
    Army* unk_48() const { return m_48; }
    int unk_6c() const { return m_6c; }
    void unk_0049E140(Faction* faction, int);  // 0x0049E140: hands it to the settlement's new owner
    int unk_0049E906();                 // 0x0049E906: public order slot 18
    bool unk_0049E979();                // 0x0049E979
    SettlementUnk184* unk_0049EF76();   // 0x0049EF76: another one (its +0x78), or 0
    Faction* unk_0049E4B4();            // 0x0049E4B4: its settlement's owner

    char m_pad24[0x3c - 0x24];
    int m_3c;                           // +0x3c: a port the senate can ask to blockade has it above 0
    char m_pad40[0x48 - 0x40];
    Army* m_48;                         // +0x48: an army in it, or 0
    char m_pad4c[0x6c - 0x4c];
    int m_6c;                           // +0x6c
};

// What a settlement hands to its +0xe0 and +0x120 when it changes owner.
struct SettlementOwnerChange {
    Settlement* m_settlement;
    Faction* m_faction;                 // the new owner
};

// What a settlement's +0xe0 is (not identified).
class SettlementUnkE0 {
public:
    void unk_005BF3A1(SettlementOwnerChange* change);  // 0x005BF3A1

    char m_pad0[0x40];
};

// What a settlement's +0x120 is (not identified).
class SettlementUnk120 {
public:
    void unk_005BF675(SettlementOwnerChange* change);  // 0x005BF675

    char m_pad0[0x64];
};

// What a settlement's +0x1e0 is (not identified).
class SettlementUnk1E0 {
public:
    void unk_005C2497();                // 0x005C2497
    void unk_005C26DB();                // 0x005C26DB: after a load

    char m_pad0[0x138];
};

// What a settlement's +0x318 is (not identified).
class SettlementUnk318 {
public:
    void unk_005C5559();                // 0x005C5559
    void unk_005C56E9();                // 0x005C56E9: after a load

    char m_pad0[0x1ec];
};

// A type of building in export_descr_buildings.txt.
struct BuildingType {
    bool defenses() { return m_defenses; }
    bool port() { return m_port; }
    bool hinterland() { return m_hinterland; }

    char m_pad0[0x5c];
    bool m_defenses;                    // +0x5c: the defenses strand (walls)
    bool m_port;                        // +0x5d: the port_buildings strand
    bool m_5e;
    bool m_temple;                      // +0x5f: its name starts with temple_
    bool m_hinterland;                  // +0x60: its name starts with hinterland_ (roads, farms)
    char m_pad61[0x68 - 0x61];
    String m_name;                      // +0x68
};

// A building of a settlement.
struct Building {
    void damage(unsigned int amount);   // 0x005D755F: takes amount off its health; at 0 it's destroyed
    BuildingType* type() { return m_type; }
    unsigned int health() { return m_health; }

    char m_pad0[0x2c];
    BuildingType* m_type;               // +0x2c
    unsigned char m_level;              // +0x30: 0 for its type's first level
    char m_pad31[0x4c - 0x31];
    unsigned int m_health;              // +0x4c: 0 to 100
};

// What lbl_014DA2B0's +0x2ac is (not identified).
class BuildingTypeList {
public:
    unsigned int unk_005BEA85();        // 0x005BEA85: how many
};

// The building types of export_descr_buildings.txt (a guess, from what it holds).
class BuildingDB {
public:
    BuildingType* unk_008D8695(unsigned int index);  // 0x008D8695
    int unk_008DB1B9(const String& name);   // 0x008DB1B9: the index of this hidden resource, or -1 (a guess)

    char m_pad0[0x2ac];
    BuildingTypeList m_types;           // +0x2ac
};

// What a riot or a revolt does to a settlement: ranges to roll each loss from, and more (not identified; 0x38 bytes).
struct SettlementDamageParams {
    SettlementDamageParams();           // 0x005BE897: all 0
    unsigned int building_chance() { return m_building_chance; }
    unsigned int heavy_chance() { return m_heavy_chance; }

    int m_0;                            // +0x00: a range rolled for SettlementDD8::unk_005C8CE1 (not identified)
    int m_4;
    int m_population_min;               // +0x08: the population killed, in percent
    int m_population_max;
    int m_character_min;                // +0x10: the chance that each character in it is killed, in percent
    int m_character_max;
    int m_garrison_min;                 // +0x18: the garrison's losses (passed to Army::kill_soldiers)
    int m_garrison_max;
    unsigned int m_building_chance;     // +0x20: the chance that each building is damaged, in percent
    int m_building_min;                 // +0x24: the health it loses
    int m_building_max;
    unsigned int m_heavy_chance;        // +0x2c: below this roll (out of 100), it loses the heavy amount instead
    int m_heavy_min;                    // +0x30
    int m_heavy_max;
};

// Up to 50 characters or buildings a riot or a revolt killed or damaged.
struct SettlementDamageList {
    SettlementDamageList(const SettlementDamageList& other)
    {
        int unused = 0;
        for (unsigned int i = 0; i < 50; i++)
            m_values[i] = other.m_values[i];
    }

    int m_values[50];
};

// What a riot or a revolt did to a settlement, for the messages (not identified; 0x1a8 bytes). Its copy constructor
// (0x005BE7CA) is the compiler's.
struct SettlementDamage {
    SettlementDamage(Settlement* settlement, int, unsigned int population, int garrison);  // 0x005BCC29
    void add_character(Character* character);  // 0x005BCCD3: a character it killed
    void add_building(Building* building);      // 0x005BCD0B: a building it damaged

    Settlement* m_settlement;
    int m_4;
    unsigned int m_population;          // +0x08: the population it killed
    int m_garrison;                     // +0x0c: the garrison's losses
    unsigned int m_num_characters;      // +0x10: how many characters it killed
    SettlementDamageList m_characters;  // +0x14
    unsigned int m_num_buildings;       // +0xdc: how many buildings it damaged
    SettlementDamageList m_buildings;   // +0xe0
};

// A settlement's buildings (not identified).
class SettlementBuildings {
public:
    Building* unk_005C2B69(unsigned int index);  // 0x005C2B69: the same as unk_005C2B81
    Building* unk_005C2B81(int index);  // 0x005C2B81
    void damage(SettlementDamageParams* params, SettlementDamage* damage, bool skip_outside);  // 0x005C2D29
    unsigned int count() { return m_count; }

    char m_pad0[0x200];
    unsigned int m_count;               // +0x200
    char m_pad204[0x564 - 0x204];
};

// What Settlement::unk_005B5975 returns (not identified; 8 bytes).
struct SettlementUnk5B5975 {
    SettlementUnk5B5975(const SettlementUnk5B5975& other);

    int m_0;
    int m_4;
};

// What each level of settlement has (16 bytes; not identified).
struct SettlementLevel {
    int m_0;                            // public order: squalor grows faster above twice this population
    int m_4;
    int m_8;
    int m_c;                            // public order: and again above this population
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
// An army besieging a settlement (a guess, from how a settlement's list of them is checked for enemies).
class Siege {
public:
    Character* unk_00717C38();          // 0x00717C38: m_army's leader

    char m_pad0[0x10];
    Army* m_army;                       // +0x10
};

// One of a settlement's sieges (8 bytes).
struct SettlementSiege {
    Siege* m_siege;
    int m_4;
};

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

// What settlements and forts have in common (not identified).
class FogOfWarEntry;

class SettlementBase : public StrategyObject {
public:
    int faction() const { return m_faction; }
    Army* garrison() const { return unk_40(); }
    Army* unk_40() const { return m_garrison.m_object; }
    bool besieged() const { return m_84; }
    Siege* siege(int index) const { return m_sieges[index].m_object; }
    bool has_plague() const { return m_plague; }

    void catch_plague();                                    // 0x0049ABD1: and so do the characters on its tile
    void plague_tick();                                     // 0x0049ACAA: the fourth tick ends the plague
    void unk_0049A464();                                    // 0x0049A464: does nothing
    unsigned int num_characters(bool governor);             // 0x0049BE82: the characters in it (the governor too, when asked)

    virtual FogOfWarEntry* unk_74();                        // +0x74: its fog of war entry, new
    virtual void unk_78();
    virtual void unk_7c();
    virtual void unk_80();
    virtual void unk_84(Army* army);                        // +0x84
    virtual void unk_88();
    virtual int region_id();                                // +0x8c: 0x005BE77E
    virtual void unk_90();
    virtual void unk_94();
    virtual void unk_98();
    virtual Faction* unk_9c();                              // +0x9c: the same function as owner()
    virtual Faction* owner();                               // +0xa0: 0x005BE753
    virtual void unk_a4();
    virtual void change_owner(CharacterRecord* captor, Faction* faction, int reason);  // +0xa8: 0x005B7F5E
    virtual void unk_ac(Character* leader);                 // +0xac: 0x005B9745, after a revolt
    virtual void unk_b0();
    virtual Region* unk_b4();

    char m_pad24[0x3c - 0x24];
    WatchedPointer<Army> m_garrison;    // +0x3c (a guess)
    WatchedPointer<Siege> m_sieges[8];  // +0x44 (a guess; 8 fill the space before m_84)
    unsigned char m_84;                 // +0x84: how many of m_sieges there are; no riots while it's non-zero
    char m_pad85[0x8c - 0x85];
    int m_8c;                           // +0x8c: public order slot 17 is twice this
    char m_pad90[0x98 - 0x90];
    int m_98;                           // +0x98: after a trade (reason 5), the previous owner; otherwise 21
    char m_pad9c[0xa0 - 0x9c];
    int m_faction;                      // +0xa0
    char m_pada4[0xac - 0xa4];
    unsigned int m_plague_ticks;        // +0xac
    bool m_plague;                      // +0xb0
    bool m_b1;                          // +0xb1: cleared by a change of owner
    char m_padb2[0xb4 - 0xb2];
};

// A fort (code/fort.cpp).
class Fort : public SettlementBase {
public:
    void unk_00498B82(int faction);                         // 0x00498B82: at the start of each faction's turn
};

// The campaign's forts (not identified).
class CampaignForts {
public:
    void unk_0049954D(Fort* fort);      // 0x0049954D: takes it off the map (a guess)

    Fort** m_items;
    int m_capacity;
    unsigned int m_count;
};

class Settlement : public SettlementBase {
public:
    virtual FogOfWarEntry* unk_74();                        // +0x74: 0x005B1440
    virtual int region_id();                                // +0x8c: 0x005BE77E
    virtual Faction* unk_9c();                              // +0x9c: the same function as owner()
    virtual Faction* owner();                               // +0xa0: 0x005BE753
    virtual void change_owner(CharacterRecord* captor, Faction* faction, int reason);  // +0xa8: 0x005B7F5E
    virtual void unk_ac(Character* leader);                 // +0xac: 0x005B9745, after a revolt

    void unk_0049B680(Faction* faction, int reason);        // 0x0049B680
    int unk_005B1486();                                     // 0x005B1486: fn_008F6174 of the owner
    bool better_governor(Character* character);             // 0x005B67A7
    void set_governor(Character* character);                // 0x005B9959
    void set_rebels(RebelType* type, int faction);          // 0x0049C42E: m_rebel_type and m_rebel_faction

    Character* character(unsigned int index, bool governor);  // 0x0049C13F
    void start_turn();                                      // 0x005B6476
    void unk_005BD3CE(int faction);                         // 0x005BD3CE: at the start of each faction's turn
    void end_turn();                                        // 0x005B656F
    bool unk_005BC86B();                                    // 0x005BC86B: m_entertainment % 3 > 0
    void unk_005BC88A(int);                                 // 0x005BC88A
    void unk_005B68EA();                                    // 0x005B68EA
    bool unk_0049A46D();                                    // 0x0049A46D: whether it has sieges (also called on forts)
    unsigned int num_buildings() const { return m_buildings.m_count; }
    SettlementDamage riot_damage(SettlementDamageParams* params, bool, bool, bool, bool skip_outside);  // 0x005B26B0
    void check_riots(BuildingEffects* effects);             // 0x005B2AC0
    void largest_share(int* faction, unsigned char* share, bool not_owner);  // 0x005B3312
    int revolt_type();                                      // 0x005B33D0
    Army* revolt(int type, int, int reason, int faction);   // 0x005B38D1: the rebels' army, or 0
    float revolt_strength();                                // 0x005B4C85
    void find_defectors(Array<Character*>* defectors, Faction* faction);  // 0x005B4ED3: who goes over in a civil war
    int unit_cost(BuildingEffects* effects);                // 0x005B9BFC: in percent
    void recruitable_units(RecruitmentItems* units, BuildingEffects* effects, bool ships, bool owned_only);  // 0x005B9E7C
    void unk_005C58A1(int faction, BuildingEffects* effects);  // 0x005C58A1
    void unk_0049B043(Faction* faction, int reason, bool);  // 0x0049B043: ends with change_owner
    void unk_0049B15C(Character* leader, int, int reason, bool);  // 0x0049B15C: ends with change_owner
    void unk_0049A894(bool);                                // 0x0049A894
    void unk_0049BC60(Character* character);                // 0x0049BC60
    Character* governor() const { return m_governor.m_character; }
    int level() const { return m_level; }
    int tax_level() const { return m_tax_level; }
    int plague_deaths() const { return m_plague_deaths; }
    void check_spies();                                     // 0x005B69CF: may throw out each foreign spy inside
    void unk_005B6BDB(BuildingEffects* effects);            // 0x005B6BDB
    void unk_005B6C52(BuildingEffects* effects);            // 0x005B6C52
    void unk_005B6F77(Faction* faction);                    // 0x005B6F77
    bool unk_005B7AEA();                                    // 0x005B7AEA
    void unk_005BAE31(TilePosition* position);              // 0x005BAE31
    void unk_005BD74B();                                    // 0x005BD74B
    void unk_005BD77B(int, bool);                           // 0x005BD77B
    int unk_005BD9DA();                                     // 0x005BD9DA
    bool can_stage_games();                                 // 0x005BC748: has the stage_games effect
    bool can_stage_races();                                 // 0x005BC79D: has the stage_races effect
    void set_entertainment(int entertainment);              // 0x005BC824
    int entertainment();                                    // 0x005BC859
    void unk_005BCA50(BuildingEffects* effects);            // 0x005BCA50: works out its building effects
    bool unk_005BA3F0(TilePosition* position);              // 0x005BA3F0
    int unk_005BB0ED(Unit* unit, UnitUnk4D469D& experience);  // 0x005BB0ED
    void unk_005B9837(Unit* unit);      // 0x005B9837: updates its mechanics (SettlementDD8::update) after a unit leaves
    void unk_005BBDAA(Unit* unit, int, int);  // 0x005BBDAA: takes the unit out of its queue (+0x1e0)
    void unk_005B5D57();                                    // 0x005B5D57
    void unk_005B5F04();                                    // 0x005B5F04: at the end of a round
    int owner_id();                                         // 0x005B14AB
    Region* region();                                       // 0x005B7D41
    void unk_005B5849(int* religion, int* religion_effects);  // 0x005B5849: BI's religion
    int unk_005B5AEB();                                     // 0x005B5AEB
    int unk_005B5B56(SettlementReligion* religion);         // 0x005B5B56
    SettlementUnk5B5975 unk_005B5975();                     // 0x005B5975
    int unk_005BC80B();                                     // 0x005BC80B: public order slot 8, times 5
    int unk_005BCDC4();                                     // 0x005BCDC4: public order slot 11 (distance to the capital?)
    void after_load();                                      // 0x005B2241
    void unk_005BACCE(TilePosition* position);              // 0x005BACCE: puts it on the map there (a guess)

    RebelType* m_rebel_type;            // +0xb4: who rebels in it (a guess, from what sets it)
    int m_rebel_faction;                // +0xb8: the faction they're for (21 for none)
    char m_padbc[0xe0 - 0xbc];
    SettlementUnkE0 m_e0;               // +0xe0
    SettlementUnk120 m_120;             // +0x120
    SettlementUnk184* m_184;            // +0x184
    char m_pad188[0x190 - 0x188];
    UniString m_name;                   // +0x190: its name as shown (a guess)
    Faction* m_owner;                   // +0x194
    char m_pad198[0x19c - 0x198];
    unsigned char m_population_share[0x1b4 - 0x19c];  // +0x19c: each faction's share of the population, by faction
    int m_level;                        // +0x1b4: 0 for the smallest
    char m_pad1b8[4];
    int m_creator;                      // +0x1bc: the faction that built it (a guess)
    unsigned short m_owner_turns;       // +0x1c0: turns since its owner changed
    char m_pad1c2[2];
    int m_region_id;                    // +0x1c4
    char m_pad1c8[0x1e0 - 0x1c8];
    SettlementUnk1E0 m_1e0;             // +0x1e0
    SettlementUnk318 m_318;             // +0x318
    SettlementBuildings m_buildings;    // +0x504
    TilePosition m_a68;                 // +0xa68: a position (where it was last placed? a guess)
    char m_pada70[0xa78 - 0xa70];
    int m_plague_deaths;                // +0xa78: the population the plague killed in the last growth step

    bool m_force_revolt;                // +0xa7c: the next riot check revolts
    bool m_rioted;                      // +0xa7d: it rioted at the last check
    unsigned char m_a7e;                // +0xa7e: counts the revolts while the owner's unk_004584F1() holds
    char m_pada7f[1];
    unsigned int m_turns_since_revolt;  // +0xa80
    int m_entertainment;                // +0xa84: 0-2 hold games, 3-5 races, 6-8 both (at three levels? a guess)
    bool m_is_capital;                  // +0xa88: its owner's capital (Faction::set_capital)
    char m_pada89;
    bool m_a8a;                         // +0xa8a: cleared by a change of owner
    bool m_a8b;                         // +0xa8b: cleared by a change of owner
    char m_pada8c[0xa94 - 0xa8c];
    WatchedCharacter m_governor;        // +0xa94
    WatchedCharacter m_a9c;             // +0xa9c
    char m_padaa4[0xabc - 0xaa4];
    Array<void*> m_abc;                 // +0xabc: type 8 of the saved pointers (not identified)
    int m_tax_level;                    // +0xac8: 0 low to 3 very high
    char m_padacc[0xdd5 - 0xacc];
    bool m_dd5;                         // +0xdd5
    char m_padd6[0xdd8 - 0xdd6];
    SettlementDD8 m_dd8;                // +0xdd8
};

// A date of the campaign: a year and a season.
struct Date {
    Date() { m_year = 0; m_season = 2; }
    Date(int year, int season) { m_year = year; m_season = season; }
    int season() const;                         // 0x0043C687: 2 in summer, 0 in winter (a guess)
    void unk_0043C59F();                        // 0x0043C59F: the next season (a guess)
    Date& next_turn();                          // 0x0043BEFE: summer to winter, or winter to the next year's summer
                                                //   (skipping year 0)
    int turns_since(const Date& earlier) const; // 0x0043C046
    Date& previous_turn();                      // 0x0043BF44
    bool operator==(const Date& other) const;   // 0x0043BDDD
    bool operator!=(const Date& other) const;   // 0x0043BEA4
    void add_turns(int turns);                  // 0x0043BF77: back for a negative count
    bool operator>=(const Date& other) const;   // 0x0043BEE0

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
    RoadPoint(int index);               // 0x004A4309: the position of the map's tile index
    bool operator==(const RoadPoint& other) const
    {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }
    bool operator!=(const RoadPoint& other) const { return !(*this == other); }
    bool unk_004B61C2(const RoadPoint& other) const;  // 0x004B61C2

    int x;
    int y;
};

// A tile's road state in a saved game (0xc bytes).
struct TileRoadRecord {
    TileRoadRecord();                   // 0x0048ADD5

    Road* m_road;                       // the road's address when it was saved
    unsigned short m_road_level : 3;    // +0x04
    unsigned short m_flags15 : 2;
    unsigned short m_flags18 : 1;
    unsigned short m_flags21 : 1;
    unsigned short m_flags24 : 1;
    unsigned short m_flags27 : 1;
    unsigned short m_1c_30 : 1;
    unsigned short m_flags20 : 3;
    unsigned short m_20_3 : 1;
    unsigned short m_pad : 2;
    unsigned short m_x;                 // +0x06
    unsigned short m_y;                 // +0x08
};

// A land region's tile beside the sea (0xc bytes).
struct CoastTile {
    CoastTile(int tile) : m_tile(tile), m_4(-1.0f), m_8(999) {}
    bool operator==(const CoastTile& other) const
    {
        return m_tile == other.m_tile && m_4 == other.m_4 && m_8 == other.m_8;
    }

    int m_tile;                         // its index in the map's grid
    float m_4;
    int m_8;
};

// A region's entry for one of its neighbours (0x54 bytes).
struct RegionNeighbour {
    RegionNeighbour()
        : m_other(0), m_region(-1), m_8(-1.0f), m_c(false), m_road(0), m_14(0), m_18(0), m_24(0), m_30(0), m_3c(0),
          m_48(0)
    {
    }                                   // 0x0048B5D5
    RegionNeighbour(const RegionNeighbour& other)
        : m_other(other.m_other), m_region(other.m_region), m_8(other.m_8), m_c(other.m_c), m_road(other.m_road),
          m_14(other.m_14), m_18(other.m_18), m_24(other.m_24), m_30(other.m_30), m_3c(other.m_3c), m_48(other.m_48)
    {
        // NONMATCHING - the original reserves 4 bytes of stack, this 0x40
    }                                   // 0x0048DA4F
    void add_border_tile(int tile);     // 0x004863DB
    void unk_0048642E();                // 0x0048642E
    void unk_00486561(int region);      // 0x00486561
    void unk_00487915();                // 0x00487915

    Region* m_other;                    // +0x00
    int m_region;                       // +0x04
    float m_8;                          // +0x08
    bool m_c;                           // +0x0c
    Road* m_road;                       // +0x10: the road this region planned towards it
    int m_14;                           // +0x14
    Array<int> m_18;                    // +0x18
    Array<int> m_24;                    // +0x24
    Array<int> m_30;                    // +0x30
    Array<int> m_3c;                    // +0x3c
    Array<int> m_48;                    // +0x48
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
    bool is_sea() const { return m_10 >= 10 && m_10 <= 12; }
    bool is_bridge() const { return m_model >= 0x6c && m_model <= 0x6f; }
    bool is_river();                    // 0x0048A945: a river or a river's source
    int unk_0048A97F();                 // 0x0048A97F: 1 for ground type 8, 3 for type 7, else 0

    StrategyObject* m_objects;          // the first object on it
    Road* m_road;                       // +0x04: the road drawn on it
    char m_pad8[4];
    unsigned short m_model;             // +0x0c: 0x6c-0x6f for bridges, 0xffff for none
    char m_padE[2];
    int m_10;                           // +0x10: the ground type (10-12 sea)
    int m_region;                       // +0x14
    char m_pad18[4];
    unsigned int m_flags0 : 5;          // +0x1c
    unsigned int m_river_source : 1;
    unsigned int m_river : 1;
    unsigned int m_flags7 : 5;
    unsigned int m_road_level : 3;      // 4 for a settlement's tile (a guess)
    unsigned int m_flags15 : 2;
    unsigned int m_landmark : 1;        // a landmark stands on it
    unsigned int m_flags18 : 3;
    unsigned int m_flags21 : 3;         // doubles the cost of entering it
    unsigned int m_flags24 : 3;
    unsigned int m_flags27 : 3;
    unsigned int m_1c_30 : 1;
    unsigned int m_flags31 : 1;
    unsigned int m_flags20 : 3;         // +0x20
    unsigned int m_20_3 : 1;
    unsigned int m_flags20_4 : 4;
    unsigned int m_region_border : 1;   // beside a tile of another land region
    unsigned int m_coast : 1;           // a land tile beside a sea region's tile
    unsigned int m_sea_coast : 1;       // a sea tile beside a land region's tile
    unsigned int m_flags20_11 : 21;
    unsigned char m_24_0 : 1;           // +0x24
    unsigned char m_24_1 : 1;
    unsigned char m_24_2 : 1;           // a structure stands on it
    unsigned char m_24_3 : 1;
    unsigned char m_24_4 : 1;
    unsigned char m_24_5 : 1;
    unsigned char m_24_6 : 1;
    unsigned char m_24_7 : 1;
    unsigned char m_25;                 // the directions it can't be entered from (0xff: none)
    char m_pad26[2];
    unsigned int m_28;                  // +0x28
    int m_2c;                           // +0x2c: 0 when no character is on it (a guess)
    float m_movement_factor;            // +0x30
};

// A region's colour in map_regions.tga.
struct RegionColour {
    unsigned char m_rgb[3];
};

// A unit type and how many of it a revolt in a region raises (not identified).
struct RegionRevoltUnit {
    UnitUnk50* m_type;
    int m_experience;                   // +0x04
    char m_pad8[0x18 - 8];
    float m_count;                      // +0x18
};

// What a region's +0x88's +0x1c is (not identified).
class RegionRevoltUnitList {
public:
    unsigned int unk_004585E5();        // 0x004585E5: how many
};

// What a region's +0x88 is (not identified).
class RegionRevoltUnits {
public:
    RegionRevoltUnit* unk_005BE904(unsigned int index);  // 0x005BE904

    char m_pad0[0x1c];
    RegionRevoltUnitList m_1c;          // +0x1c
};

// A rebel type in descr_rebel_factions.txt (code\rebel_faction_db.cpp; 0xb74 bytes).
struct RebelType {
    char m_pad0[4];
    String m_name;                      // +0x04
    char m_padc[0x14 - 0xc];
    int m_chance;                       // +0x14
};

// What a region's +0x44 holds (not identified).
class RegionUnk44 {
public:
    int unk_38() const { return m_38; }
    void unk_004BF34E(bool, Faction* faction, int);  // 0x004BF34E: told of the settlement's new owner

    char m_pad0[0x38];
    int m_38;                           // +0x38
};

// A region of the strategy map (0x1d8 bytes).
class Region {
public:
    void unk_00483B67();                // 0x00483B67: at the start of each round
    int id() const { return m_id; }
    int unk_00485627(int resource, int);  // 0x00485627: how many of the resource it has (a guess)
    unsigned char road_level() const { return m_road_level; }
    Port* port() const { return m_port; }
    Settlement* settlement() const { return (Settlement*)m_port; }  // what StrategyMap::set_settlement keeps there
    int num_neighbours() const { return m_neighbours.size(); }
    void set_port_road(Road* road) { m_port_road = road; }
    int creator_faction() const { return m_creator_faction; }
    bool is_sea() const { return m_sea; }
    RegionNeighbour* neighbour(int region);  // 0x00483C05: its entry for a neighbouring region, added if missing
    void unk_00483CBF();                     // 0x00483CBF
    void unk_00486079(int tile);             // 0x00486079: adds the tile to m_a4
    void unk_00483D33();                     // 0x00483D33
    void set_road_level(unsigned char level);  // 0x00483E16: 2 at most
    void raise_road_level();                 // 0x00483E6B: 3 at most
    void set_farming_level(unsigned int level);  // 0x00483E8E: 5 at most
    void raise_farming_level();              // 0x00483EAF: 4 at most
    void set_famine_threat(int threat);      // 0x00483ECE: 4 at most
    void raise_famine_threat();              // 0x00483EF8: 4 at most
    int unk_00483F17();                      // 0x00483F17: the port's +0x184
    int unk_00483F36();                      // 0x00483F36: the port's +0x184
    void unk_00484763();                     // 0x00484763
    RebelType* rebel_type();                 // 0x00483BA6
    void unk_004841C6();                     // 0x004841C6
    bool has_rebel_type();                   // 0x00483BC5
    bool has_resource(unsigned int resource) const { return (m_resources[resource >> 5] & (1 << (resource & 0x1f))) != 0; }

    char m_pad0[8];
    String m_settlement_name;           // +0x08: its settlement's name in descr_regions.txt (a guess)
    char m_pad10[0x1c - 0x10];
    int m_id;                           // +0x1c
    unsigned char m_road_level;         // +0x20: 0 none, 1 roads, 2 paved roads, 3 highways
    char m_pad21[3];
    unsigned int m_farming_level;       // +0x24
    int m_famine_threat;                // +0x28
    char m_pad2c[0x44 - 0x2c];
    Array<RegionUnk44*> m_44;           // +0x44
    bool m_sea;                         // +0x50: a sea region
    char m_pad51[0x58 - 0x51];
    Road* m_port_road;                  // +0x58
    char m_pad5c[0x68 - 0x5c];
    int m_creator_faction;              // +0x68: the faction descr_regions.txt names for it (21 for none)
    char m_pad6c[0x88 - 0x6c];
    RegionRevoltUnits* m_88;            // +0x88: more units for a revolt there (not identified)
    char m_pad8c[0x98 - 0x8c];
    Array<CoastTile> m_coast;           // +0x98: its tiles beside the sea
    Array<int> m_a4;                    // +0xa4: tile indices
    RegionReligions m_religions;        // +0xb0 (BI)
    char m_padd8[0x148 - 0xd8];
    Port* m_port;                       // +0x148
    Array<int> m_tiles;                 // +0x14c: its tiles, by index into the map's grid
    char m_pad158[0x164 - 0x158];
    Array<RegionNeighbour> m_neighbours;  // +0x164: in the order the map's scan first finds them
    char m_pad170[0x188 - 0x170];
    unsigned int m_resources[3];        // +0x188: its hidden resources, a bit each
    RoadPoint m_settlement_position;    // +0x194
    RoadPoint m_port_position;          // +0x19c
    RegionColour m_colour;              // +0x1a4: its colour in map_regions.tga
    char m_pad1a7[1];
    Faction* m_owner;                   // +0x1a8: the faction that holds it, or 0
    bool m_1ac;                         // +0x1ac
    char m_pad1ad[3];
    int m_1b0;                          // +0x1b0: a revolt there is 1.5 times as strong while it's set (not identified)
    char m_pad1b4[0x1b8 - 0x1b4];
    UniString m_settlement_display_name;  // +0x1b8: its settlement's name as shown (a guess)
    char m_pad1bc[0x1c0 - 0x1bc];
    String m_rebel_type;                // +0x1c0: its rebel type's name
    char m_pad1c8[0x1d8 - 0x1c8];
};

// A set of factions, a bit each.
class FactionSet {
public:
    int bits() const { return m_bits; }
    void add(int faction) { m_bits |= 1 << faction; }
    bool contains(int faction) const { return (m_bits & (1 << faction)) != 0; }
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
    bool has_shadowed_by() const { return m_shadowed_by < 0xffffffff; }
    bool shadowed_by_set() const { return m_10 != 0; }
    int shadowed_by() const { return shadowed_by_set() ? m_shadowed_by : 21; }
    bool has_shadowing() const { return m_shadowing < 0xffffffff; }
    bool shadowing_set() const { return m_1c != 0; }
    int shadowing() const { return shadowing_set() ? m_shadowing : 21; }
    bool has_spawns_on_revolt() const { return m_spawns_on_revolt < 0xffffffff; }
    bool spawns_on_revolt_set() const { return m_28 != 0; }
    int spawns_on_revolt() const { return spawns_on_revolt_set() ? m_spawns_on_revolt : 21; }
    bool unk_a7() const { return m_a7; }

    int m_id;                           // +0x00: its faction's index
    char m_pad4[8];
    unsigned int m_shadowed_by;         // +0x0c: its shadowed_by faction (0xffffffff for none)
    int m_10;                           // +0x10: non-zero once m_shadowed_by is set (a guess)
    char m_pad14[4];
    unsigned int m_shadowing;           // +0x18: its shadowing faction (0xffffffff for none)
    int m_1c;                           // +0x1c
    char m_pad20[4];
    unsigned int m_spawns_on_revolt;    // +0x24: its spawns_on_revolt faction (0xffffffff for none)
    int m_28;                           // +0x28
    char m_pad2c[0x40 - 0x2c];
    FactionDescription40* m_40;         // +0x40
    char m_pad44[0xa4 - 0x44];
    bool m_custom_battle_availability;  // +0xa4
    char m_pada5[2];
    bool m_a7;                          // +0xa7
};

class Character;
class CharacterRecord;
class TriggerEvent;

// What FactionUnk180::unk_00493794 finds on a tile (not identified).
class FactionTileUnk {
public:
    virtual void unk_00();
    virtual void unk_04(Faction* faction);
    virtual int unk_08();
    virtual int unk_0c();
};

// What a faction's +0x180 is (not identified).
class FactionUnk180 {
public:
    unsigned char at(const RoadPoint& point) { return m_data[point.x + point.y * m_width]; }
    unsigned char at(const TilePosition& point) { return m_data[point.x + point.y * m_width]; }
    void unk_00492498(Character* character, bool);  // 0x00492498
    void unk_00492B0F(StrategyObject* object, bool);  // 0x00492B0F
    FactionTileUnk* unk_00493794(const RoadPoint& point);  // 0x00493794

    unsigned char* m_data;              // a byte per tile
    int m_width;                        // +0x04
};

class AncillaryDescription;

class FactionAIUnk34 {
public:
    void unk_0050625A(int, int);        // 0x0050625A
};

// A faction's AI's figures for one faction (not identified; 0x2c bytes).
struct FactionAIUnk20Entry {
    char m_pad0[8];
    int m_8;                            // +0x08
    int m_c;                            // +0x0c: AISenateFaction adds it to its view's m_threat
    bool m_10;                          // +0x10
    char m_pad11[0x1c - 0x11];
    int m_1c;                           // +0x1c: and this to m_c
    char m_pad20[0x28 - 0x20];
    int m_28;                           // +0x28: and this to m_10
};

// A faction's AI's war targets, probably (not identified; built at 0x00517247, on 0x00514BFB; 0x3cc bytes).
class FactionAIUnk20 {
public:
    bool unk_18() const { return m_18; }
    int unk_1c() const { return m_1c; }

    char m_pad0[0x18];
    bool m_18;                          // +0x18: m_1c is set
    char m_pad19[3];
    int m_1c;                           // +0x1c: a region's index
    char m_pad20[0x28 - 0x20];
    FactionAIUnk20Entry m_entries[21];  // +0x28: one per faction
    char m_pad3c4[0x3cc - 0x3c4];
};

// A faction's AI's figures for one faction in its global strategy director (not identified; 0x20 bytes).
struct FactionAIUnk38Entry {
    char m_pad0[0x1e];
    bool m_1e;                          // +0x1e
    char m_pad1f[1];
};

// A faction's AI's global strategy director, probably (codeii_global_strategy_director.cpp; built at
// 0x0050AFCA; 0x370 bytes).
class FactionAIUnk38 {
public:
    bool unk_1e(int faction) const { return m_entries[faction].m_1e; }

    char m_pad0[0xc];
    FactionAIUnk38Entry m_entries[21];  // +0x0c: one per faction
    char m_pad2ac[0x370 - 0x2ac];
};

// A faction's AI (not identified; 0x44 bytes).
class FactionAI {
public:
    FactionAI(Faction* faction);        // 0x004FD904
    FactionAIUnk20* unk_20() const { return m_20; }
    FactionAIUnk34* unk_34() const { return m_34; }
    FactionAIUnk38* unk_38() const { return m_38; }

    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
    virtual void unk_14();
    virtual void unk_18();
    virtual void unk_1c();

    char m_pad4[0x20 - 4];
    FactionAIUnk20* m_20;               // +0x20
    char m_pad24[0x34 - 0x24];
    FactionAIUnk34* m_34;               // +0x34
    FactionAIUnk38* m_38;               // +0x38
    char m_pad3c[0x44 - 0x3c];
};

// Something of a faction's, updated at the start of its turn (not identified).
class FactionUnk4F8 {
public:
    void unk_004DBCBD();                // 0x004DBCBD
};

// A faction's finances (not identified beyond its update).
// A faction's figures for one turn in its finances (not identified; 0x58 bytes).
class FinanceRecord {
public:
    void unk_004D7EAA();                // 0x004D7EAA

    char m_pad0[0x58];
};

// A faction's last turns' finances (0x378 bytes).
struct FinanceHistory {
    FinanceRecord& current() { return m_records[m_current]; }

    FinanceRecord m_records[10];
    int m_370;                          // +0x370
    unsigned int m_current;             // +0x374
};

// A faction's money (0x38c bytes).
class FactionFinance {
public:
    void end_turn();                    // 0x004D8197: adds the turn's income, and the AI's cash
    void unk_004D8396(bool, bool);      // 0x004D8396: at the start of its turn

    Faction* m_faction;                 // +0x00
    int m_treasury;                     // +0x04
    int m_income;                       // +0x08: the turn's
    FinanceHistory m_history;           // +0x0c
    char m_pad384[0x38c - 0x384];
};

// Something of a human faction's (not identified).
class FactionUnk88C {
public:
    void unk_004D2ACF();                // 0x004D2ACF
    unsigned int unk_004D3219();        // 0x004D3219: room for adoptions: 4 less the children of each man (alive or
                                        //   not) of 16 or more with m_flag16, or 1 when no record has m_flag17

    char m_pad0[0xc];
};

// Something of a faction's (not identified).
class FactionUnk898 {
public:
    void set_14(int value) { m_14 = value; }

    char m_pad0[0x14];
    int m_14;                           // +0x14: a faction (21 for none; set so when a named character dies in a riot)
};

struct FactionGoneEvent;

// A faction's listeners, told of events of its (not identified).
class FactionListeners {
public:
    void unk_004D71CF(const FactionGoneEvent& event);  // 0x004D71CF: tells each of them

    char m_pad0[0x10];
};

// Something of a faction's, told at the start of each round (not identified).
class FactionUnkD4 {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08();
    virtual void unk_0c();
    virtual void unk_10();
};

// A faction's campaign record (code\faction.cpp).
class Faction {
public:
    int unk_178() const { return m_178; }
    unsigned int num_regions() const { return m_regions.size(); }
    FactionSet neighbours() const { return m_neighbours; }
    bool no_cash_crisis() const { return m_4ec == 0; }  // (what +0x4ec means is not known)
    bool has_cash_crisis() const { return !no_cash_crisis(); }
    bool at_war_with(Faction* other);   // 0x004CFC65
    bool unk_4f4() const { return m_4f4; }
    Settlement* capital() const { return m_capital; }
    FactionAI* ai() const { return m_ai; }
    bool unk_a7() const { return m_description->m_a7; }
    bool unk_1d4() const { return m_1d4; }
    int id() const { return m_id; }
    int culture() const { return m_culture; }
    FactionDescription* description() const { return m_description; }
    bool is_unk_a7() const { return m_description->m_a7; }
    bool shadowed_by_set() const { return m_description->m_10 != 0; }
    int shadowed_by() const { return shadowed_by_set() ? m_description->m_shadowed_by : 21; }
    bool shadowing_set() const { return m_description->m_1c != 0; }
    int shadowing() const { return shadowing_set() ? m_description->m_shadowing : 21; }
    bool unk_4fc() const { return m_4fc; }
    FactionUnk180* unk_180() const { return m_180; }

    bool unk_00445B2B();                // 0x00445B2B: description's +0xa4 set and +0xa7 clear
    bool unk_004584F1();                // 0x004584F1: has_cash_crisis() or +0x4f0 is a faction
    Character* unk_004C41EF(const char* name);          // 0x004C41EF: its character on the map with that name
    Settlement* unk_004C44F0(CharacterRecord* father);  // 0x004C44F0
    int unk_004C4589(CharacterRecord* record);          // 0x004C4589
    void unk_004C4CE7(CharacterRecord* record);         // 0x004C4CE7
    bool unk_004C5093(Character* character, bool, bool);  // 0x004C5093
    void unk_004CA5BA(Army* army);                      // 0x004CA5BA
    void unk_004CA308(bool);                            // 0x004CA308: makes or replaces its AI (a guess)
    void unk_004CAC88();                                // 0x004CAC88: its turn update
    bool unk_004CAEDB();                                // 0x004CAEDB: its part of the end of a round
    void marian_reforms();                              // 0x004D411D
    void unk_004CB391();                                // 0x004CB391: works out m_neighbours
    void unk_004CB56C();                                // 0x004CB56C
    void start_turn();                                  // 0x004CB6EA
    void unk_004C9616(Character* spy);                  // 0x004C9616
    void plague_tick();                                 // 0x004CB040
    void catch_spy(Character* spy, float strength, Settlement* settlement);  // 0x004CB5A9
    void unk_004CF14F();                                // 0x004CF14F
    Army* unk_004C9515(Character* leader, int);         // 0x004C9515: an army for the new character
    void unk_004CDFCF(Character* leader);               // 0x004CDFCF: it comes back
    CharacterRecord* unk_004CE1E1();                    // 0x004CE1E1: its first record with m_flag17
    void set_leader(CharacterRecord* record);           // 0x004CEC05
    int stance(Faction* other);                         // 0x004CFC0B: 0 allied, 600 at war (DiplomaticRelation); 200 for itself or none
    int stance(int other);                              // 0x004CFBD3: the same, by the other's index
    void* display_name();                               // 0x004C3F6B: its name's text (a guess)
    CharacterRecord* leader() const { return m_leader; }
    int average_order();                                // 0x004CEB90: its settlements' public order percentages' average
    void give_first_cohort();                           // 0x004D245E: a "roman legionary first cohort i" or "ii", 50-50, once the +0x2b6 flag is set
    void unk_004D3E6F(int);                             // 0x004D3E6F
    void destroy();                                     // 0x004CE245: it's gone (the kill_faction command too)
    void unk_004CE839(int faction);                     // 0x004CE839
    void unk_004CE844(int faction, int);                // 0x004CE844
    void unk_004CE943(int faction, int);                // 0x004CE943: hands its ... to the faction (a guess)
    void unk_004CEA1C(int faction, int, int);           // 0x004CEA1C: the same (a guess)
    void unk_004CEAC8(int faction, int);                // 0x004CEAC8: the same (a guess)
    void unk_004CED7E();                                // 0x004CED7E
    void set_capital(Settlement* capital);              // 0x004CF09A: moves the settlements' m_is_capital too
    void unk_004D13E8(AISenateFaction* senate);         // 0x004D13E8: checks its senate missions, and drops those done
    void unk_004D0905(Array<Region*>* regions);         // 0x004D0905: its settlements' regions (a guess)
    void unk_004D6EC0(LinkedList<Character*>* characters, int type);  // 0x004D6EC0: its characters of a type

    char m_pad0[0x90];
    FactionListeners m_listeners;       // +0x90
    int m_id;                           // +0xa0
    int m_culture;                      // +0xa4
    Settlement* m_capital;              // +0xa8
    CharacterRecord* m_leader;          // +0xac
    CharacterRecord* m_heir;            // +0xb0 (a guess: its record's m_flag4 is cleared with it)
    FactionDescription* m_description;  // +0xb4
    int m_control;                      // +0xb8 (1: human)
    FactionAI* m_ai;                    // +0xbc
    int m_c0;                           // +0xc0
    int m_c4;                           // +0xc4
    char m_padc8[0xd4 - 0xc8];
    FactionUnkD4* m_d4;                 // +0xd4
    char m_padd8[4];
    PointerList<CharacterRecord> m_records;     // +0xdc
    PointerList<Character> m_characters;        // +0xe8
    PointerList<Army> m_armies;                 // +0xf4
    Array<int> m_regions;                       // +0x100: its regions' indices
    PointerList<Settlement> m_settlements;      // +0x10c
    PointerList<Fort> m_forts;                  // +0x118
    char m_pad124[0x130 - 0x124];
    PointerList<SettlementUnk184> m_ports;  // +0x130: its settlements' ports (a guess)
    FactionSet m_neighbours;            // +0x13c: the factions whose regions border this one's
    Array<int> m_bordering_regions;     // +0x140: the other factions' regions beside its own
    bool m_14c;                         // +0x14c: it holds a sea region (a guess)
    char m_pad14d[0x170 - 0x14d];
    float m_170;                        // +0x170: set from a global at the start of its turn
    char m_pad174[0x178 - 0x174];
    int m_178;                          // +0x178
    char m_pad17c[4];
    FactionUnk180* m_180;               // +0x180
    PointerList<SenateMissionImpl> m_missions;  // +0x184: its senate missions under way
    char m_pad190[0x1cc - 0x190];
    int m_triumph_order;                // +0x1cc: public order for a triumph, down by 1 a turn
    int m_wonder_order;                 // +0x1d0: public order for capturing a wonder, 4 then down by 1 a turn
    bool m_1d4;                         // +0x1d4
    char m_pad1d5[0x4e4 - 0x1d5];
    bool m_4e4;                         // +0x4e4: set when one of its settlements' figures changes
    char m_pad4e5[0x4ec - 0x4e5];
    int m_4ec;                          // +0x4ec
    int m_4f0;                          // +0x4f0 (a faction index, 21 for none)
    bool m_4f4;                         // +0x4f4
    char m_pad4f5[0x4f8 - 0x4f5];
    FactionUnk4F8* m_4f8;               // +0x4f8
    bool m_4fc;                         // +0x4fc: it took over from the faction it shadows, so its end has no message
    char m_pad4fd[0x500 - 0x4fd];
    FactionFinance m_finance;           // +0x500
    FactionUnk88C m_88c;                // +0x88c
    FactionUnk898 m_898;                // +0x898
};

// How one faction stands towards another.
class DiplomaticRelation {
public:
    char m_pad0[4];
    int m_stance;                       // +0x04: 0 allied, 600 at war
    unsigned int m_flags;               // +0x08: bit 1 is probably military access
    char m_padc[0x74 - 0xc];
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
    int stance(const int& faction, const int& other) const { return m_factions[faction].stance(other); }
    bool is_at_war(int faction);        // 0x005A11E9: at war with anyone
    void unk_005A2EEE();                // 0x005A2EEE: at the end of a round
    void unk_005A2C6A();                // 0x005A2C6A
    void unk_005A1417(int faction, int other, int incident);  // 0x005A1417: an incident between two factions
    bool unk_005A0ABE(int faction, int other);  // 0x005A0ABE: probably whether they have a trade agreement
    bool unk_005A0F3E(int faction, int other);  // 0x005A0F3E: bit 3 of their relation's flags
    bool unk_005A0FB1(int faction);     // 0x005A0FB1: 0x005A0F3E's flag with any faction

    FactionRelations m_factions[21];
};

// Something of the campaign's (not identified).
class CampaignUnkCD5C {
public:
    void unk_005F2A2F();                // 0x005F2A2F: at the end of a round
};

// The campaign (probably code\campaign.cpp).
// The campaign's settings, set up from the options when it starts (0x00415D33).
struct CampaignSettings {
    float m_unit_scale;                 // +0x00: the unit size option (20, 40, 80 or 160) / 40, kept to 0.33-4
    char m_pad4[0x20 - 4];
};

// A human player of the campaign (not identified beyond its faction).
struct CampaignHuman {
    int m_0;
    int m_faction;                      // +0x04
};

class AISenateFaction;

class Campaign {
public:
    int unk_004529FB(int faction);      // 0x004529FB: m_114
    Faction* faction(int index);        // 0x004548DD
    bool senate_at_war_with(Faction* faction);  // 0x0045425F: AISenateFaction::at_war_with, or false without a senate
    int difficulty(int faction);        // 0x004529E3: 0 easy to 3 very hard; always 1 for the AI
    float unit_scale();                 // 0x00452964
    CampaignSettings* settings();       // 0x00B4DC5F: lbl_018DC384's +0x29f8
    bool unk_00452977();                // 0x00452977: +0x238
    bool is_faction_alive(int faction); // 0x0045491C
    bool unk_00454978();                // 0x00454978: lbl_02921AE0 and the battle's +0x46 are set
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
    bool unk_0045460A();                                // 0x0045460A
    Faction* unk_00454960(int faction);                 // 0x00454960: by its index
    Settlement* unk_004543F1(const char* name);         // 0x004543F1: a settlement by its name (a guess)
    bool last_britain_region(Region* region);           // 0x00457A73
    void unk_004537F2();                                // 0x004537F2: the end of a round
    void unk_004544AA();                                // 0x004544AA
    Settlement* rome();                                 // 0x004576C4
    void set_rome(Settlement* settlement);              // 0x004576DD
    Settlement* constantinople();                       // 0x004576F9
    void set_constantinople(Settlement* settlement);    // 0x00457712
    Settlement* get_rome() const { return m_rome; }
    void unk_00457C3C();                                // 0x00457C3C
    void unk_00452A2D();                                // 0x00452A2D
    void unk_00452A62();                                // 0x00452A62
    void unk_0045361A();                                // 0x0045361A: the start of a round
    void unk_00453B9B();                                // 0x00453B9B: the start of a faction's turn
    void unk_0045732F();                                // 0x0045732F
    void unk_00453942();                                // 0x00453942

    bool m_0;                           // +0x00: set once the campaign has started (0x00452AD6); unk_00452A2D and unk_00452A62 clear it
    char m_pad1[3];
    int m_4;                            // +0x04
    char m_pad8[0x10 - 8];
    CampaignHuman m_humans[21];         // +0x10
    unsigned int m_b8;                  // +0xb8: how many human factions there are (a guess; 1: the advisor's triggers run)
    char m_padbc[0xc0 - 0xbc];
    int m_difficulty[21];               // +0xc0: each faction's difficulty
    int m_114[21];                      // +0x114: something for each faction
    Faction* m_factions[21];            // +0x168: in turn order
    Faction* m_factions_by_id[21];      // +0x1bc
    unsigned int m_num_factions;        // +0x210
    char m_pad214[0x218 - 0x214];
    AISenateFaction* m_senate;          // +0x218: or 0
    char m_pad21c[0x22c - 0x21c];
    Faction* m_22c;                     // +0x22c: the faction whose turn it is (a guess)
    unsigned int m_230;                 // +0x230: its index in m_factions (a guess)
    char m_pad234[0x24c - 0x234];
    unsigned int m_turn;                // +0x24c: the turn number (a guess: things are timed by it)

    unsigned int turn() { return m_turn; }

    char m_pad250[0x254 - 0x250];
    Watcher m_rome_watcher;             // +0x254: m_rome's (a WatchedCharacter's code sets it)
    Settlement* m_rome;                 // +0x258: the settlement named Rome (Settlement::after_load)
    Watcher m_constantinople_watcher;   // +0x25c
    Settlement* m_constantinople;       // +0x260: the settlement named Constantinople
    char m_pad264[0x2b4 - 0x264];
    bool m_2b4;                         // +0x2b4: the families' popularity is kept to 65 or less (not identified)
    bool m_prologue;                    // +0x2b5 (a guess: it lowers brigand spawns and changes taxes and garrisons)
    bool unk_2b8() const { return m_2b8; }

    bool m_marian_reforms;              // +0x2b6: set by descr_strat.txt's marian_reforms_activated, or by unk_004537F2
    bool m_marian_reforms_disabled;     // +0x2b7: descr_strat.txt's marian_reforms_disabled
    bool m_2b8;                         // +0x2b8: characters can go over in a civil war (not identified)
    bool m_2b9;                         // +0x2b9: every revolt is to rebels (not identified)
    char m_pad2ba[0x2c4 - 0x2ba];
    bool m_2c4;                         // +0x2c4
    char m_pad2c5[0x2e4 - 0x2c5];
    Date m_date;                        // +0x2e4 (its year is compared with 100; perhaps the turn number)
    char m_pad2ec[0x528 - 0x2ec];
    Random m_random;                    // +0x528
    DiplomacyMatrix m_diplomacy;        // +0x52c
    char m_padcd00[0xcd28 - 0xcd00];
    CampaignForts m_forts;              // +0xcd28
    char m_padcd34[0xcd4c - 0xcd34];
    char m_road_manager[0x10];          // +0xcd4c: a RoadManager (road.h's campaign_roads())
    CampaignUnkCD5C m_cd5c;             // +0xcd5c
};

// The strategy map (probably code\strategy_map.cpp).
class StrategyMap {
public:
    int num_regions() const { return m_region_list.size(); }
    unsigned int width() const { return m_width; }
    unsigned int height() const { return m_height; }
    bool in_bounds(int x, int y) { return x >= 0 && x < (int)width() && y >= 0 && y < (int)height(); }
    bool contains(const RoadPoint& point) { return point.x < width() && point.y < height(); }
    Tile* tile(int x, int y) { return &m_tiles[x + y * m_width]; }
    Tile* tile(int index) { return &m_tiles[index]; }
    Tile* tile(RoadPoint point) { return &m_tiles[point.x + point.y * m_width]; }
    Tile* tile(TilePosition position) { return &m_tiles[position.x + position.y * m_width]; }
    Region* region(int id) { return &m_regions[id]; }
    void add_road(int region, int other, Road* road);  // 0x004812B2
    bool can_cross_river(int from_x, int from_y, int to_x, int to_y);  // 0x0047C6B8: at a bridge, while planning roads
    void add_landmark(int region, Landmark* landmark); // 0x00481251
    void set_settlement(int region, Settlement* settlement);  // 0x004811B7: and the region's owner follows it
    void unk_0048204D(Army* army, int region);         // 0x0048204D
    void unk_00482CC0();                               // 0x00482CC0: at the end of each round in summer
    void unk_00479BE9();                               // 0x00479BE9: filter_coastlines
    unsigned char unk_00471495(RoadPoint position);    // 0x00471495: the tile's unk_0048A97F()
    void unk_00481FA4(int region);                     // 0x00481FA4: sets the region's m_1ac
    void unk_004826A1(TilePosition* position);         // 0x004826A1
    unsigned char unk_00481334(int tile, int* neighbours);  // 0x00481334: the tile's neighbouring tiles (8 at most)
    Region* unk_004829FD(TilePosition* position);      // 0x004829FD: the region of the tile at position
    Region* unk_00482A48(TilePosition* position);      // 0x00482A48: the same
    void unk_004823F9(int region, Faction* faction);   // 0x004823F9: a region's new owner
    void unk_0047FC44(int region);                     // 0x0047FC44
    bool is_sea_region(int region);                    // 0x00481149
    void build_neighbours();                           // 0x0047F003
    bool load_tile_road(SaveStream& stream);           // 0x0047D489
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

// What PathFinder::unk_004A76B7 finds on a tile (not identified).
class TileStructureOwner : public StrategyObject {
public:
    virtual void unk_74();
    virtual void unk_78();
    virtual void unk_7c();
    virtual void unk_80();
    virtual void unk_84();
    virtual void unk_88();
    virtual void unk_8c();
    virtual void unk_90();
    virtual void unk_94();
    virtual void unk_98();
    virtual void unk_9c();
    virtual Faction* unk_a0();          // +0xa0: its faction (a guess)
};

struct TileStructure {
    char m_pad0[0x30];
    TileStructureOwner* m_30;           // +0x30
};

// What PathFinder::unk_004A74BA returns (not identified).
struct TileUnk84 {
    char m_pad0[0x84];
    unsigned char m_84;                 // +0x84
};

// What a PathFinder's +0x6c is (not identified).
class PathFinderUnk6C {
public:
    void unk_0056CEC4(Character* character);  // 0x0056CEC4

    char m_pad0[4];
};

// Probably code\extents_manager.cpp's (not identified).
class ExtentsManager {
public:
    void unk_00490EE4();                // 0x00490EE4: at the end of a round

    char m_pad0[4];
};

// The campaign map's path planning (code\strategy_map.cpp).
class PathFinder {
public:
    int find_road_path(const RoadPoint& from, const RoadPoint& to);  // 0x004AE64B
    void unk_004AFB1E(TilePosition* position);                      // 0x004AFB1E
    void plan_roads(int region, unsigned char level);               // 0x004B1B5B
    void build_roads(int region, unsigned char level);              // 0x004B21E2
    bool unk_004A605B(const RoadPoint& point);                      // 0x004A605B
    int unk_004A5231(RoadPoint* settlement, int);                   // 0x004A5231
    int unk_004A5879(RoadPoint* port);                              // 0x004A5879
    Army* unk_004AAC55(TilePosition* position);                     // 0x004AAC55
    int unk_004A7EA9(Faction* faction, RoadPoint* point);           // 0x004A7EA9
    void unk_004ABED9(StrategyObject* object, TilePosition* position);  // 0x004ABED9
    unsigned char unk_004B3124(TilePosition position);              // 0x004B3124
    void unk_004AC914(const RoadPoint& point);                      // 0x004AC914
    void set_character(Character* character);                       // 0x004B64C3: 0 for roads
    void unk_004A4E73();                                            // 0x004A4E73: before a load
    void unk_004B3011();                                            // 0x004B3011
    void unk_004B31E9();                                            // 0x004B31E9: at the start of each round
    void unk_004A434A();                                            // 0x004A434A
    void unk_004AFADF();                                            // 0x004AFADF
    bool is_blocked(const RoadPoint& from, const RoadPoint& to, Tile* tile);  // 0x004A6862: by the terrain
    int unk_004AB253(const RoadPoint& from, const RoadPoint& to);   // 0x004AB253
    bool unk_004AB325(Character* character);                        // 0x004AB325
    Army* unk_004AADAE(TilePosition* position);                     // 0x004AADAE: the army there, or 0
    void foreign_spies(Settlement* settlement, Array<Character*>* spies);  // 0x004A9D01
    void unk_004A9C59(Settlement* settlement, Array<Character*>* spies);   // 0x004A9C59: the same, by unk_00580C1E
    void unk_004A99D9(TilePosition* position, Array<Character*>* characters, Faction* faction, int type);  // 0x004A99D9:
                                                                    //   the faction's living characters of a type there
    void unk_004AB5CC(Settlement* settlement);                      // 0x004AB5CC
    void unk_004A4C7F();                                            // 0x004A4C7F: at the start of each faction's turn
    bool unk_004B13A2(int, int from, int to, Array<int>* path, float* cost, int);  // 0x004B13A2: a land route
                                                                    //   between two regions (a guess)
    void unk_004ACEB2(float* cost);                                 // 0x004ACEB2: a cost per turn (a guess)
    void unk_004AC808(Character* character);                        // 0x004AC808
    void unk_004AC1DB(Settlement* settlement);                      // 0x004AC1DB
    void unk_004AC371(Settlement* settlement);                      // 0x004AC371
    int unk_004A7D84(Faction* faction, const RoadPoint& point);     // 0x004A7D84
    int can_move(const RoadPoint& from, const RoadPoint& to, const RoadPoint& goal);  // 0x004B52BA: 1 when it can
    TileStructure* unk_004A76B7(const RoadPoint& point);           // 0x004A76B7: the tile's object of type 0x1d
    int unk_004A7CA9(Faction* faction, const RoadPoint& point);     // 0x004A7CA9
    TileUnk84* unk_004A74BA(const RoadPoint& point);                // 0x004A74BA
    int unk_004AB135(const RoadPoint& point, int);                  // 0x004AB135
    bool unk_004A6DA0(const RoadPoint& point, Character* character);  // 0x004A6DA0
    bool unk_004A7097(const RoadPoint& point, int, int);            // 0x004A7097
    Character* unk_004A95F3(Faction* faction, const RoadPoint& point, bool* flag);  // 0x004A95F3
    bool unk_004AC9E5(Faction* faction, Character* character);      // 0x004AC9E5
    bool unk_004AA85D(const RoadPoint& point);                      // 0x004AA85D: m_25 isn't 0xff, and bits 0, 1, 3, 5
                                                                    //   and 6 of +0x24 are clear
    void unk_004B68A4(Character* character, const RoadPoint& point, Array<int>* tiles, int);  // 0x004B68A4
    int unk_004A7F3C(Faction* faction, const RoadPoint& point);     // 0x004A7F3C
    float step_cost(const RoadPoint& from, const RoadPoint& to);    // 0x004B634D

    Character* character() const { return m_watched.m_character; }
    bool is_mode_3() const { return m_c == 3; }

    char m_pad0[4];
    WatchedCharacter m_watched;         // +0x04: the character whose path it is, or 0 (for roads)
    int m_c;                            // +0x0c: 3 for ships? (a guess)
    Faction* m_faction;                 // +0x10: the character's faction, or 0 (for roads)
    int m_status;                       // +0x14: of the last search (0 found, 1 bad goal, 2 no path)
    char m_pad18[4];
    int m_1c;                           // +0x1c: set to 3 at the start of each faction's turn
    char m_pad20[0x6c - 0x20];
    PathFinderUnk6C m_6c;               // +0x6c
    char m_pad70[0x84 - 0x70];
    ExtentsManager m_84;                // +0x84
    char m_pad88[0xbc - 0x88];
    Array<RoadPoint> m_path;            // +0xbc: the last path found, goal first
};

// A node of the A* search.
struct PathNode {
    float m_f;                          // +0x00: m_g + m_h
    float m_h;                          // +0x04: the estimate of the cost left
    float m_g;                          // +0x08: the cost from the start
    int m_index;                        // +0x0c: its tile's index in the map's grid
    RoadPoint m_point;                  // +0x10
    PathNode* m_parent;                 // +0x18
    PathNode* m_children[8];            // +0x1c
};

// The search's nodes by tile index (not identified).
class PathNodeTable {
public:
    void insert(int index, PathNode* node);  // 0x004B81B4
    PathNode** find(int index);         // 0x004B81CD
    void remove(int index);             // 0x004B81FE

    void* m_0;
    int m_4;
};

// Orders the open list's heap: the node with the higher f is lower.
struct PathNodeGreater {
    bool operator()(PathNode* a, PathNode* b) const { return a->m_f > b->m_f; }
};

// The open list: a binary heap of nodes, cheapest first.
class PathNodeHeap {
public:
    bool empty() const { return m_items.m_size == 0; }
    PathNode* top() const { return m_items.m_data[0]; }
    void clear() { m_items.m_size = 0; }
    void push(PathNode* const& node);   // 0x004BA44B
    void pop();                         // 0x004BA51D

    Array<PathNode*> m_items;
    PathNodeGreater m_greater;          // +0x0c
};

class PathNodePool {
public:
    PathNode* alloc();                  // 0x004BA1F5
};

// An entry of propagate's stack of nodes to update.
struct PathNodeStackEntry {
    PathNode* m_node;
    PathNodeStackEntry* m_next;
    PathNodeStackEntry* m_free;         // +0x08: the next free entry, while in the pool's free list
};

// A pool of stack entries, allocated 64 at a time (romans_utils.h).
class PathNodeStackPool {
public:
    PathNodeStackEntry* alloc();        // 0x004BA34B
    void free(PathNodeStackEntry* entry)
    {
        entry->m_free = m_free;
        m_free = entry;
        m_count--;
    }

    void* m_blocks;
    int m_num_blocks;                   // +0x04
    PathNodeStackEntry* m_free;         // +0x08
    int m_count;                        // +0x0c: entries in use
};

// A step to a neighbouring tile.
struct TileStep {
    short x;
    short y;
};

// The campaign map's A* search (code\strategy_map_pathfinding.cpp).
class AStar {
public:
    PathNode* search(int mode, const RoadPoint& from, const RoadPoint& to, bool diagonal);  // 0x004B84E5
    void reset();                       // 0x004B83E1
    PathNode* pop();                    // 0x004B8B27
    void expand(PathNode* node, const RoadPoint& goal);  // 0x004B8B85
    void relax(PathNode* parent, const RoadPoint& point, const RoadPoint& goal);  // 0x004B8DE0
    void propagate(PathNode* node);     // 0x004B9092
    void push_stack(PathNode* node);    // 0x004B920B
    PathNode* pop_stack();              // 0x004B92C2

    bool m_orthogonal;                  // +0x00: only the 4 orthogonal directions
    PathNodeTable m_open;               // +0x04
    PathNodeTable m_closed;             // +0x0c
    int m_start_region;                 // +0x14
    int m_goal_region;                  // +0x18
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
    int m_2c;                           // +0x2c: its category (0 infantry, 1 cavalry, 4 ships, 5 animals; a guess)
    char m_pad30[0x80 - 0x30];
    unsigned short m_turns;             // +0x80: stat_cost's turns to train it
    unsigned short m_cost;              // +0x82: stat_cost's cost
    char m_pad84[0xc0 - 0x84];
    unsigned short m_c0;                // +0xc0: its soldiers before the unit size scales them (a guess)
    char m_padc2[0xd0 - 0xc2];
    unsigned int m_d0;                  // +0xd0: bit 0 when its weapon can be upgraded (a guess)
    char m_padd4[0xe0 - 0xd4];
    int m_weapon;                       // +0xe0: its weapon's kind, 0 simple to 4 other (a guess)
    char m_pade4[0x154 - 0xe4];
    int m_154;                          // +0x154: added to its soldiers (not identified)
    char m_pad158[0x1f4 - 0x158];
    unsigned int m_ownership;           // +0x1f4: the factions that own it, a bit each
    char m_pad1f8[0x200 - 0x1f8];
    unsigned int m_attributes_0 : 7;    // +0x200: its attributes from export_descr_unit.txt, one bit each
    unsigned int m_attribute_7 : 1;     // bit 7 (not identified)
    unsigned int m_attributes_8 : 6;
    unsigned int m_command : 1;         // bit 14: command (an eagle or standard)
    unsigned int m_attributes_15 : 8;
    unsigned int m_is_peasant : 1;      // bit 23: is_peasant
    unsigned int m_general_unit : 1;    // bit 24: general_unit (general_unit_upgrade sets it too)
    unsigned int m_general_unit_upgrade : 1;  // bit 25: general_unit_upgrade
    unsigned int m_attributes_26 : 6;
    char m_pad204[0x20c - 0x204];

    int soldiers();                     // 0x00918878: m_c0 scaled by the unit size (a guess)

    bool owned_by(int faction) const { return (m_ownership & (1 << faction)) != 0; }
    bool is_animal() const { return m_2c == 5; }
};

// The unit types of export_descr_unit.txt (code\unit_db.cpp).
class UnitDB {
public:
    int count() const { return m_count; }
    bool has_general_upgrade(int faction);  // 0x0091B995

    char m_pad0[4];
    UnitUnk50 m_types[500];             // +0x04
    int m_count;                        // +0x3ff74
};

// A unit in a settlement's recruitment queue, or one it could recruit (code\recruitment_item.cpp; 0x20 bytes).
struct RecruitmentItem {
    RecruitmentItem();                  // 0x005AE26D
    void reset();                       // 0x005AE2D6
    void unk_005AE4F0(Settlement* settlement, UnitUnk50* type, unsigned char experience, unsigned char armour,
                      unsigned char weapon, int percent);  // 0x005AE4F0
    void unk_005AE6C3(Settlement* settlement, UnitUnk50* type, unsigned char experience, unsigned char armour,
                      unsigned char weapon, int percent);  // 0x005AE6C3: the same, with m_0d set (for a ship; a guess)

    UnitUnk50* m_type;                  // +0x00
    char m_pad4[4];
    Settlement* m_settlement;           // +0x08
    unsigned char m_0c;                 // +0x0c
    unsigned char m_0d;                 // +0x0d: set by unk_005AE6C3
    unsigned char m_experience;         // +0x0e
    unsigned char m_armour;             // +0x0f
    unsigned char m_weapon;             // +0x10
    unsigned char m_11;                 // +0x11
    unsigned char m_12;                 // +0x12
    unsigned char m_13;                 // +0x13
    unsigned char m_turns;              // +0x14: the turns it takes to train
    unsigned short m_cost;              // +0x16
    unsigned short m_soldiers;          // +0x18
    unsigned short m_turn;              // +0x1a: the turn it was made on
    unsigned char m_1c;                 // +0x1c
};

// The units a settlement can recruit (a revolt's too), up to 256 (not identified).
struct RecruitmentItems {
    RecruitmentItems()
    {
        m_data = (RecruitmentItem*)m_items;
        m_count = 0;
        m_2008 = 0;
    }
    ~RecruitmentItems()
    {
        while (m_count != 0)
            m_count--;
    }
    unsigned int count();               // 0x005AF300
    RecruitmentItem* add();             // 0x005AF319

    char m_items[256 * sizeof(RecruitmentItem)];  // made as they're added
    RecruitmentItem* m_data;            // +0x2000
    unsigned int m_count;               // +0x2004
    int m_2008;
};

// What a unit's +0x44 points to (not identified).
class UnitUnk44 {
public:
    virtual void unk_00();
    virtual void unk_04();
    virtual void unk_08(Unit* unit);    // +0x08: the unit leaves (a guess)
};

// A unit of an army (code/unit.cpp).
class Unit {
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

    char m_pad4[0x40 - 0x4];
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
    bool unk_ac() const { return m_ac; }
    Region* unk_007199A3();             // 0x007199A3: the region it's in (a guess)
    int unk_007216D2(int, int, TilePosition* position);  // 0x007216D2
    void unk_0071EBF6(int faction, int);    // 0x0071EBF6
    int unk_bc() const { return m_bc; }

    TilePosition* unk_00719BB2();       // 0x00719BB2
    void unk_00719E61();                // 0x00719E61
    Unit* unk_0071DADD(Character* character, int, int, int, int);  // 0x0071DADD
    void unk_007201B3(RebelType* type, int faction);  // 0x007201B3: m_rebel_type and m_rebel_faction
    Unit* unit(int index);              // 0x0072212A
    int garrison_soldiers();            // 0x00722341
    Character* leader() const { return m_leader; }

    void remove_unit(Unit* unit);       // 0x0071E460: and deletes it; a command unit lost in the field gives the enemy its eagle
    StrategyObject* unk_11c() const { return m_11c; }
    void unk_0071E2C8(Unit* unit);      // 0x0071E2C8: takes it out of m_units
    void update_strength();             // 0x0071B41D
    unsigned int compute_strength();    // 0x0071B451: the sum of its units' values, rounded
    int unk_00722430();                 // 0x00722430: its soldiers (each unit's +0x280 and unk_007B9438())
    int kill_soldiers(int percent);     // 0x0071FE0D: each unit loses that share of its soldiers; returns the dead
    unsigned int strength();            // 0x0071B3EB
    void unk_00720D26(int type, float strength, int min_units, int max_units, RecruitmentItems* units);  // 0x00720D26
    void unk_007201D6(int type, float strength, int min_units, int max_units, RebelType* rebel);  // 0x007201D6
    int unk_007225D5(bool, bool);       // 0x007225D5: how many units are ... (not identified)
    bool unk_007233E0(bool);            // 0x007233E0
    void unk_00724043(int, int, int, int);  // 0x00724043
    void unk_00725C23(Army* army);      // 0x00725C23
    Faction* unk_00721587();            // 0x00721587: its faction (+0x54)
    Character* unk_007216B9();          // 0x007216B9: its leader (+0xf0)
    void unk_0071B214(Character* character, int);  // 0x0071B214: takes it out of m_characters (a guess)
    void unk_0071E3F1(Unit* unit);      // 0x0071E3F1: takes it out (a guess)
    bool unk_0072309A();                // 0x0072309A

    char m_pad10[0x54 - 0x10];
    Faction* m_faction;                 // +0x54
    char m_pad58[0x5c - 0x58];
    Array<Unit*> m_units;               // +0x5c
    char m_pad68[0xac - 0x68];
    bool m_ac;                          // +0xac
    char m_padad[0xbc - 0xad];
    int m_bc;                           // +0xbc
    char m_padc0[0xf0 - 0xc0];
    Character* m_leader;                // +0xf0
    char m_padf4[0x10c - 0xf4];
    PointerList<Character> m_characters;  // +0x10c: the characters with it besides its leader (a guess)
    char m_pad118[0x11c - 0x118];
    StrategyObject* m_11c;              // +0x11c: where it is (a settlement or fort it's in?)
    char m_pad120[0x129 - 0x120];
    bool m_129;                         // +0x129: set when a battle is set up with it (0x0071C970), cleared at 0x0071CFA4
    bool m_in_battle;                   // +0x12a: set by the battle code (a guess)
    char m_pad12b[0x134 - 0x12b];
    int m_134;                          // +0x134
    bool m_captured_eagle;              // +0x138: it took an enemy's eagle (a guess)
    bool m_139;                         // +0x139: while it's set, its strength is 0
    char m_pad13a[0x140 - 0x13a];
    unsigned int m_strength;            // +0x140
    char m_pad144[0x164 - 0x144];
    RebelType* m_rebel_type;            // +0x164: who a settlement it takes rebels for (a guess)
    int m_rebel_faction;                // +0x168
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
    Army* first_army() { return army(0); }
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
    bool unk_46() const { return m_46; }

    int side_of(Army* army);            // 0x005FF179: -1 for neither
    BattleSide* side(int index);        // 0x005FF3BB
    BattleSide* unk_005FF3D9(int index);  // 0x005FF3D9: the same as side()
    int side_of_faction(int faction);   // 0x005FF3F7
    void force_victory(int side);       // 0x005FD5D1: every other side loses (the force_battle_ commands)
    unsigned int num_sides() const { return m_num_sides; }

    char m_pad0[8];
    int m_8;                            // +0x08
    bool m_c;                           // +0x0c
    char m_padd[0x24 - 0xd];
    bool m_24;                          // +0x24
    char m_pad25[0x46 - 0x25];
    bool m_46;                          // +0x46
    char m_pad47[0xe1f8 - 0x47];
    int m_side_of_faction[21];          // +0xe1f8
    unsigned int m_num_sides;           // +0xe24c
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

// The same, rounding toward zero (the same header's).
inline int float_to_int_chop(float value)
{
    int control;
    int chop;
    int result;
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

// The OfferedForAdoption event.
class UnkMessage8A4586 : public UnkMessage {
public:
    UnkMessage8A4586(CharacterRecord* record);  // 0x008A4586
};

// The SettlementTurnStart event, at the start of the owner's turn.
class LeaderSenateMissionSuccessEvent : public TriggerEvent {
public:
    LeaderSenateMissionSuccessEvent(CharacterRecord* leader);  // 0x008A7CCE
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

class LeaderSenateMissionFailedEvent : public TriggerEvent {
public:
    LeaderSenateMissionFailedEvent(CharacterRecord* leader);  // 0x008A7D9D
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_leader;
};

// The disasters of descr_events.txt, by their index in the exe's list of their names (earthquake, flood, ...).
enum {
    DISASTER_PLAGUE = 8,
};

// What a disaster did to a faction, for its turn report (0x84 bytes).
struct DisasterReport {
    DisasterReport(int disaster, int);  // 0x0043871F
    ~DisasterReport();                  // 0x00439F54
    int& soldiers_lost(int faction) { return m_soldiers_lost[faction]; }

    int m_disaster;
    int m_4;
    Array<Character*> m_characters;     // +0x08: the characters it struck
    int m_soldiers_lost[21];            // +0x14: by faction
    Array<Settlement*> m_settlements;   // +0x68: the settlements it struck
    int m_population_lost;              // +0x74
    Array<int> m_78;                    // +0x78
};

// The CharacterDamagedByDisaster event.
class CharacterDamagedByDisasterEvent : public TriggerEvent {
public:
    CharacterDamagedByDisasterEvent(CharacterRecord* record, int disaster);  // 0x008A43D0
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
    int m_disaster;
};

// The SettlementTurnEnd event.
class SettlementTurnEndEvent : public TriggerEvent {
public:
    SettlementTurnEndEvent(Settlement* settlement);  // 0x008A8465
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Character* m_governor;
};

class SettlementTurnStartEvent : public TriggerEvent {
public:
    SettlementTurnStartEvent(Settlement* settlement);  // 0x008A8396
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
};

// The CeasedFactionLeader event.
class CeasedFactionLeaderEvent : public TriggerEvent {
public:
    CeasedFactionLeaderEvent(CharacterRecord* record);  // 0x008A4F3A
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
};

// The BecomesFactionLeader event.
class BecomesFactionLeaderEvent : public TriggerEvent {
public:
    BecomesFactionLeaderEvent(CharacterRecord* record);  // 0x008A48C2
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_record;
};

// The CityRebels event.
class CityRebelsEvent : public TriggerEvent {
public:
    CityRebelsEvent(Settlement* settlement, Faction* faction);  // 0x008A9319
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Faction* m_faction;
};

// The GovernorCityRebels event.
class GovernorCityRebelsEvent : public TriggerEvent {
public:
    GovernorCityRebelsEvent(CharacterRecord* governor, Settlement* settlement, Faction* faction);  // 0x008A9400
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_governor;
    Settlement* m_settlement;
    Faction* m_faction;
};

// The FactionTurnStart event.
class FactionTurnStartEvent : public TriggerEvent {
public:
    FactionTurnStartEvent(Faction* faction);  // 0x008A3B75
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Faction* m_faction;
};

// The CityRiots event.
class CityRiotsEvent : public TriggerEvent {
public:
    CityRiotsEvent(Settlement* settlement, Faction* faction);  // 0x008A9133
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    Settlement* m_settlement;
    Faction* m_faction;
};

// The GovernorCityRiots event.
class GovernorCityRiotsEvent : public TriggerEvent {
public:
    GovernorCityRiotsEvent(CharacterRecord* governor, Settlement* settlement, Faction* faction);  // 0x008A921A
    virtual LinkedList<Trigger>* triggers() const;
    virtual const void* unk_7c() const;

    CharacterRecord* m_governor;
    Settlement* m_settlement;
    Faction* m_faction;
};

// Something about a place on the campaign map, at a date (not identified; 0x3c bytes).
class EventUnk437A36 {
public:
    EventUnk437A36(int type, TilePosition* position, Date date);  // 0x00437A36
    ~EventUnk437A36();                  // 0x00439CCA

    char m_pad0[0x3c];
};

// What lbl_012CED70 is (not identified): it keeps EventUnk437A36s.
class EventsUnk12CED70 {
public:
    void unk_0043900D();                // 0x0043900D: at the start of each round
    void unk_004392F9(const EventUnk437A36& event);  // 0x004392F9: adds it, unless the campaign's settings say not to
};

// What lbl_02921A38 is (not identified).
// The strength of the armies in some regions, split by whose they are (not identified).
struct ArmyStrengths {
    void clear();                       // 0x004DD16A

    int m_own;                          // +0x00: the faction's own armies
    int m_own_count;
    int m_enemy;                        // +0x08: the armies of factions it's at war with
    int m_enemy_count;
    int m_other;                        // +0x10: the rest
    int m_other_count;
};

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
extern Campaign* lbl_018CD818;
extern StrategyMap* lbl_018CD80C;
extern int lbl_01181378;  // the rebel faction's index
extern unsigned int lbl_015C95B4;    // how many unit types there are
extern UnitUnk50 lbl_01589644[];     // the unit types
extern UnitDB lbl_01589640;          // the same, with their count
extern RevoltSize lbl_00FEC4E8[];    // by settlement level
extern AIFactionSummary lbl_012D0F78[21];
extern AIFactionStrength lbl_012D0F74[21];  // the total strength of each faction's armies
extern AIFactionStrength lbl_012D0F68[21];  // the total strength of each faction's fleets
extern AIFactionBorder lbl_012D1548[21 * 21];
extern AIFactionPair lbl_012D1552[21 * 21];
extern const float lbl_00FE4970;  // 1.0

int fn_004E335E(int faction);

// Landmarks (code\landmark.cpp's callees)
int fn_00E3C9F6(const char* name);                     // a landmark type from its name
void fn_00CB5468(Faction* faction, Landmark* landmark, TilePosition* position, int);
void fn_00CB54E7(Faction* faction, Landmark* landmark, TilePosition* position);
unsigned char fn_009CB2A7(unsigned int x, unsigned int y);

extern StrategyMap* lbl_018CD86C;  // the strategy map again (the one movement uses?)
extern StrategyMap* lbl_02921A94;  // the strategy map again (the aerial map's copy?)
extern PathFinder* lbl_029219E0;

// Settlements' public order (code\settlement_mechanics.cpp's callees)
extern Campaign* lbl_02921A30;      // the campaign again
extern StrategyMap* lbl_02921A34;   // the strategy map again
extern PathFinder* lbl_02921A1C;    // the path finder again

inline PathFinder* path_finder()
{
    return lbl_02921A1C;
}
extern Unk02921A38* lbl_02921A38;

class UnkB4CFDA {
public:
    Faction* unk_00B4CFDA();            // 0x00B4CFDA: the player's faction (a guess)
};

inline Campaign* campaign()
{
    Campaign* campaign = lbl_02921A30;
    return campaign;
}
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

// Armies (codermy.cpp's callees)
extern Battle* lbl_018BF5E0;        // the battle being fought
extern bool lbl_02921AE0;
float fn_00432C9F(Unit* unit);      // a unit's value in a battle
float fn_004335A9(Unit* unit);      // a unit's value on the campaign map, from its type, soldiers and m_298 slots
void fn_00CB67A8(Faction* faction); // a message (type 0x60) to the faction: it lost its eagle (a guess)
extern SettlementLevel lbl_012DAFC8[];
RebelType* fn_00903C7E(const String& name);    // the rebel type of this name, or 0
int fn_008F99BC(const RegionColour* colour);   // from the descr_regions.txt entry of this colour (+0x4c)

// Settlements' changes of owner (code\settlement.cpp's callees)
void fn_008B4D04(CharacterRecord* captor, Settlement* settlement, Faction* faction, Faction* old_owner, int reason);  // a message, when some flag allows
void fn_008B529F(Settlement* settlement, Faction* faction);  // a message, when some flag allows

// Settlements' riots and revolts (code\settlement.cpp's callees)
void fn_008B4820(Settlement* settlement, Faction* faction, Character* governor);  // fires CityRebels and GovernorCityRebels
void fn_00CB42BB(const SettlementDamage* damage, Faction* faction);  // the revolt's message (a guess)
void fn_00CB4122(const SettlementDamage* damage);    // the riot's message (a guess)
extern EventsUnk12CED70 lbl_012CED70;
extern BuildingDB lbl_014DA2B0;
FactionDescription* fn_008F60D5(const String& name);  // the faction description of this name
int fn_008F6278(String name);       // the index of the faction of this name
void fn_008FF5CB(Random* random, int faction, int, int* first_name, int* surname);  // picks a name
void fn_004E4403(int faction, int region, ArmyStrengths* strengths);  // the armies in the region (and its settlements)
int fn_004E282B(Faction* faction, int* count);  // the faction's armies' total strength (those without +0xac set)
const char* fn_008FF4A3(int kind, int faction, int id);  // a saved name's key
Character* fn_008E1FA5(String* type, Faction* faction, int name_faction, int age, int first_name, int surname);  // a new general
Character* fn_008E212F(int type, Faction* faction, int age, const char* first_name, const char* surname, int name_faction);  // a new agent
void fn_00CBB9AE(Character* character, Faction* old_faction, Faction* new_faction);  // a message (a guess)
unsigned int fn_008D078B();                    // non-zero when religions are enabled (BI)
int* fn_008E695D(const String& name);          // the culture of this name, or 0
void fn_005F4FEB(int* religion, int* religion_effects, const SettlementUnk5B5975& unk, BuildingEffects* effects,
                 int, int);
extern AStar* lbl_012D07C0;
extern int lbl_012D08A4;            // the A* search's mode (2 roads, 5 region to region)
extern TileStep lbl_012D08A8[8];    // the 8 directions, clockwise from north
extern PathNodeHeap lbl_012D08E0;
extern PathNodePool lbl_012D0890;
extern PathNodeStackPool lbl_012D08C8;
extern PathNodeStackEntry* lbl_012D08DC;  // the head of propagate's stack
extern float lbl_012D05F0;          // the cost factor of a diagonal step
extern const float lbl_010D01A0[14];  // the cost of each ground type
extern const unsigned char lbl_010D0A00[9];  // the edge-mask bit of each direction, by (dy+1)*3 + dx+1
extern const float lbl_00FE60A0;    // 1.0
extern const float lbl_00FE60A4;    // 2.0
extern const float lbl_00FE60A8;    // 4.0
extern const float lbl_00FE60AC;    // 7.0
extern const float lbl_00FE609C;    // 0.4: the least a step costs
extern const float lbl_00FE60C0[8]; // the cost factor of each road level
extern const float lbl_00FE6154;    // 0.5
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

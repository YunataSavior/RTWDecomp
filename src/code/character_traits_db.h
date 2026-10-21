// The character traits' definitions, from export_descr_character_traits.txt (code\character_traits_db.cpp; the
// original's name, which its "Too many levels" error gives).
#ifndef CHARACTER_TRAITS_DB_H
#define CHARACTER_TRAITS_DB_H

#include "unsorted.h"
#include "core_strings.h"
#include "romans_utils.h"

// The original expands inlines deeper than VC6's default of 8 calls here: the destructors that Array<TraitDescription>
// tries to expand leave the slots of 11 levels.
#pragma inline_depth(255)

// A fixed number of items (probably ca_libs). The items are a base class's, so that its implicit constructor and
// destructor build and destroy them.
template <class T, int N>
class FixedArrayBase {
public:
    T m_items[N];
};

template <class T, int N>
class FixedArray : public FixedArrayBase<T, N> {
public:
    FixedArray() {}
    FixedArray(const T& value)
    {
        for (unsigned int i = 0; i < N; i++)
            m_items[i] = value;
    }
    FixedArray(const FixedArray& other)
    {
        for (unsigned int i = 0; i < N; i++)
            m_items[i] = other.m_items[i];
    }

    T& operator[](int index) { return m_items[index]; }
    void fill(const T& value);
};

template <class T, int N>
void FixedArray<T, N>::fill(const T& value)
{
    for (unsigned int i = 0; i < N; i++)
        m_items[i] = value;
}

// A set of small numbers, a bit each (probably ca_libs).
template <int BITS>
class BitSet {
public:
    BitSet();                                   // 0x00910A65
    BitSet(const BitSet& other);                // 0x00C7377B
    void set(unsigned int bit);                 // 0x00910AA4
    bool test(unsigned int bit) const
    {
        unsigned int element = bit >> 5;
        unsigned int mask = 1 << (bit & 31);
        return (m_words[element] & mask) != 0;
    }

    void set_all()
    {
        for (unsigned int i = 0; i < m_num_words; i++)
            m_words[i] = 0xffffffff;
    }
    void clear()
    {
        for (unsigned int i = 0; i < m_num_words; i++)
            m_words[i] = 0;
    }

    unsigned int m_num_words;
    unsigned int m_words[(BITS + 31) / 32];
};

template <int BITS>
BitSet<BITS>::BitSet()
{
    m_num_words = (BITS + 31) / 32;
    for (unsigned int i = 0; i < m_num_words; i++)
        m_words[i] = 0;
}

template <int BITS>
BitSet<BITS>::BitSet(const BitSet& other)
{
    m_num_words = (BITS + 31) / 32;
    for (unsigned int i = 0; i < m_num_words; i++)
        m_words[i] = other.m_words[i];
}

template <int BITS>
void BitSet<BITS>::set(unsigned int bit)
{
    unsigned int word = bit >> 5;
    unsigned int mask = 1 << (bit & 31);
    m_words[word] |= mask;
}

// A religious belief or order a trait level gives (8 bytes; not identified).
struct TraitReligion {
    TraitReligion() { m_0 = 0; m_4 = 0; }
    TraitReligion& operator=(const TraitReligion& other);  // 0x005F476E
    int type() const { return m_0; }
    int value() const { return m_4; }

    int m_0;
    int m_4;
};

// What code\religion.cpp reads a religion into (not identified; its destructor is why VC6 builds it in place).
struct ReligionValue : public TraitReligion {
    ~ReligionValue() {}
};

ReligionValue read_religion(ScriptReader& reader, bool& ok);  // 0x005F479F: a religion and its level

inline ReligionValue read_religion(ScriptReader& reader)
{
    bool ok;
    return read_religion(reader, ok);
}

// One of a level's effects: an attribute (Command, Influence, ...) and how much it adds.
struct TraitEffect {
    int m_attribute;                    // 55 for an unknown name
    int m_value;
};

// One level of a trait (0x3c bytes). Level 0, which every trait has, is the trait not showing.
struct TraitLevel {
    TraitLevel()
        : m_level(0), m_name(0), m_description(0), m_epithet(0), m_gain_message(0), m_lose_message(0),
          m_threshold(0), m_effects_description(0)
    {
    }

    int m_level;                                    // +0x00
    const UniString* m_name;                        // +0x04: from export_VnVs.txt
    const UniString* m_description;                 // +0x08
    const UniString* m_epithet;                     // +0x0c
    const UniString* m_gain_message;                // +0x10
    const UniString* m_lose_message;                // +0x14
    int m_threshold;                                // +0x18: the points it takes
    Array<TraitEffect> m_effects;                   // +0x1c
    TraitReligion m_religious_belief;               // +0x28
    TraitReligion m_religious_order;                // +0x30
    const UniString* m_effects_description;         // +0x38
};

#define MAX_TRAIT_LEVELS 10

// A trait as the game uses it (0x2d4 bytes).
class TraitDefinition {
public:
    TraitDefinition();

    int m_index;                                    // +0x000
    String m_name;                                  // +0x004
    FixedArray<TraitLevel, MAX_TRAIT_LEVELS> m_levels;  // +0x00c
    unsigned int m_num_levels;                      // +0x264: counting level 0
    FixedArray<TraitDefinition*, 20> m_anti_traits;     // +0x268
    unsigned int m_num_anti_traits;                 // +0x2b8
    BitSet<32> m_characters;                        // +0x2bc: the character types that can have it
    int m_no_going_back_level;                      // +0x2c4
    BitSet<32> m_exclude_cultures;                  // +0x2c8
    bool m_hidden;                                  // +0x2d0
};

// A trait as export_descr_character_traits.txt gives it: its anti-traits by name, until they are all read
// (0x378 bytes).
class TraitDescription : public TraitDefinition {
public:
    TraitDescription() { m_num_anti_trait_names = 0; }

    FixedArray<String, 20> m_anti_trait_names;      // +0x2d4
    unsigned int m_num_anti_trait_names;            // +0x374
};

class TraitDatabase {
public:
    static Array<TraitDescription> s_traits;        // 0x014DD2C0
};

bool load_traits();                                 // 0x008E3F49
void unload_traits();                               // 0x008E4C5D
TraitDescription* find_trait(const char* name);     // 0x008E4C92: ignoring case; 0 for none
TraitDescription* find_trait(String name);          // 0x008E4CF9: 0 for none

#endif

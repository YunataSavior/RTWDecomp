// An open-addressing hash table (probably ca_libs; its file's name is not known). Its slots are made once, with the
// unused key; a removed item's key becomes the deleted key. It doubles itself when it is full.
#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "unsorted.h"

// A text's characters, compared to another's: the second minus the first.
inline int text_compare(const char* a, const char* b)
{
    while (*(unsigned char*)a && *(unsigned char*)a == *(unsigned char*)b) {
        a++;
        b++;
    }
    return *(unsigned char*)b - *(unsigned char*)a;
}

// A text as a key, by its address: hash tables of these compare texts with TextEqual.
struct TextKey {
    TextKey(const char* text) : m_text(text) {}
    operator const char*() const { return m_text; }

    const char* m_text;
};

struct TextHash {
    // The sum of the even characters, and above it of the odd ones.
    unsigned int operator()(const TextKey& key) const
    {
        unsigned int hash = 0;
        const unsigned char* p = (const unsigned char*)key.m_text;
        while (*p) {
            ((unsigned short*)&hash)[0] = ((unsigned short*)&hash)[0] + *p;
            p++;
            if (*p) {
                ((unsigned short*)&hash)[1] = ((unsigned short*)&hash)[1] + *p;
                p++;
            }
        }
        return hash;
    }
};

struct TextEqual {
    bool operator()(TextKey a, TextKey b) const
    {
        return a.m_text == b.m_text || (a.m_text && b.m_text && ~(unsigned int)a.m_text && ~(unsigned int)b.m_text &&
                                        text_compare(a.m_text, b.m_text) == 0);
    }
};

// A String as a key: Strings keep their hash.
struct StringHash {
    unsigned int operator()(const String& key) const { return key.m_4; }
};

struct StringEqual {
    bool operator()(const String& a, const String& b) const { return a == b; }
};

template <class K, class V, class H, class E>
class HashMap {
public:
    struct Entry {
        K m_key;
        V m_value;
    };

    class iterator {
    public:
        iterator(Entry* before, Entry* cur, Entry* end, const K& unused, const K& deleted, const E& equal)
            : m_cur(cur), m_before(before), m_end(end), m_unused(unused), m_deleted(deleted), m_equal(equal)
        {
        }
        ~iterator() {}
        iterator& operator++()
        {
            do
                m_cur++;
            while (m_cur != m_end && (m_equal(m_cur->m_key, m_unused) || m_equal(m_cur->m_key, m_deleted)));
            return *this;
        }
        bool operator==(const iterator& other) const { return m_cur == other.m_cur; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

        Entry* m_cur;
        Entry* m_before;
        Entry* m_end;
        K m_unused;
        K m_deleted;
        E m_equal;
    };

    HashMap(const K& unused, const K& deleted, unsigned int capacity, const H& hash = H(), const E& equal = E());
    ~HashMap();
    V* find(const K& key);
    bool insert(const K& key, const V& value);
    void resize(unsigned int capacity);
    iterator begin();
    Entry* entry(unsigned int i) { return &m_entries[i]; }
    iterator end()
    {
        return iterator(m_entries - 1, m_entries + m_capacity, m_entries + m_capacity, m_unused, m_deleted, m_equal);
    }

    unsigned int m_capacity;
    Entry* m_entries;
    H m_hash;
    E m_equal;
    K m_unused;
    K m_deleted;
};

template <class T>
inline void safe_delete(T*& p)
{
    if (p)
        delete p;
    p = 0;
}

template <class K, class V, class H, class E>
HashMap<K, V, H, E>::~HashMap()
{
    for (unsigned int i = 0; i < m_capacity; i++) {
        if (m_unused != m_entries[i].m_key && m_deleted != m_entries[i].m_key)
            m_entries[i].m_value.~V();
        m_entries[i].~Entry();
    }
    safe_delete((int*&)m_entries);
}

template <class K, class V, class H, class E>
V* HashMap<K, V, H, E>::find(const K& key)
{
    unsigned int start = m_hash(key) % m_capacity;
    for (unsigned int i = start; i < m_capacity; i++) {
        if (m_equal(key, m_entries[i].m_key))
            return &m_entries[i].m_value;
        if (m_equal(m_unused, m_entries[i].m_key))
            return 0;
    }
    for (unsigned int j = 0; j < start; j++) {
        if (m_equal(key, m_entries[j].m_key))
            return &m_entries[j].m_value;
        if (m_equal(m_unused, m_entries[j].m_key))
            return 0;
    }
    return 0;
}

template <class K, class V, class H, class E>
bool HashMap<K, V, H, E>::insert(const K& key, const V& value)
{
    // NONMATCHING - in the TextKey instantiation the original calls TextEqual out of line, where VC6 inlines it here
    unsigned int start = m_hash(key) % m_capacity;
    for (unsigned int i = start; i < m_capacity; i++) {
        if (m_equal(key, entry(i)->m_key)) {
            entry(i)->m_value = value;
            return false;
        }
        if (m_equal(m_unused, entry(i)->m_key)) {
            entry(i)->m_key = key;
            new (&entry(i)->m_value) V(value);
            return true;
        }
        if (m_equal(m_deleted, entry(i)->m_key)) {
            unsigned int slot = i;
            while (i < m_capacity) {
                if (m_equal(key, entry(i)->m_key)) {
                    entry(i)->m_value = value;
                    return false;
                }
                if (m_equal(m_unused, entry(i)->m_key)) {
                    entry(slot)->m_key = key;
                    new (&entry(slot)->m_value) V(value);
                    return true;
                }
                i++;
            }
            for (unsigned int j = 0; j < start; j++) {
                if (m_equal(key, entry(j)->m_key)) {
                    entry(j)->m_value = value;
                    return false;
                }
                if (m_equal(m_unused, entry(j)->m_key)) {
                    entry(slot)->m_key = key;
                    new (&entry(slot)->m_value) V(value);
                    return true;
                }
            }
            entry(slot)->m_key = key;
            new (&entry(slot)->m_value) V(value);
            return true;
        }
    }
    for (unsigned int k = 0; k < start; k++) {
        if (m_equal(key, entry(k)->m_key)) {
            entry(k)->m_value = value;
            return false;
        }
        if (m_equal(m_unused, entry(k)->m_key)) {
            entry(k)->m_key = key;
            new (&entry(k)->m_value) V(value);
            return true;
        }
        if (m_equal(m_deleted, entry(k)->m_key)) {
            unsigned int slot = k;
            while (k < start) {
                if (m_equal(key, entry(k)->m_key)) {
                    entry(k)->m_value = value;
                    return false;
                }
                if (m_equal(m_unused, entry(k)->m_key)) {
                    entry(slot)->m_key = key;
                    new (&entry(slot)->m_value) V(value);
                    return true;
                }
                k++;
            }
            entry(slot)->m_key = key;
            new (&entry(slot)->m_value) V(value);
            return true;
        }
    }
    resize(m_capacity * 2 + 1);
    return insert(key, value);
}

template <class K, class V, class H, class E>
void HashMap<K, V, H, E>::resize(unsigned int capacity)
{
    int* old_entries = (int*)m_entries;
    Entry* e = entry(0);
    Entry* end = entry(m_capacity);
    m_capacity = capacity;
    void* memory = ::operator new((m_capacity * sizeof(Entry) + 3) / 4 * 4);
    unsigned int index;
    m_entries = (Entry*)memory;
    for (index = 0; index < m_capacity; index++)
        new (&m_entries[index].m_key) K(m_unused);
    while (e != end) {
        if (e->m_key != m_unused && e->m_key != m_deleted)
            insert(e->m_key, e->m_value);
        e->~Entry();
        e++;
    }
    safe_delete(old_entries);
}

template <class K, class V, class H, class E>
HashMap<K, V, H, E>::iterator HashMap<K, V, H, E>::begin()
{
    iterator it(m_entries - 1, m_entries - 1, m_entries + m_capacity, m_unused, m_deleted, m_equal);
    ++it;
    return it;
}

#endif

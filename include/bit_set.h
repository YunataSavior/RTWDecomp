// A set of small numbers, a bit each (probably ca_libs).
#ifndef BIT_SET_H
#define BIT_SET_H

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

#endif

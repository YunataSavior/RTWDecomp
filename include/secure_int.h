// Ints kept scrambled in memory (not identified; perhaps against memory editors). The game holds some of its
// constants this way: the code has the scrambled bits, and value() unscrambles them.
#ifndef SECURE_INT_H
#define SECURE_INT_H

// A scrambled int.
class ScrambledInt {
public:
    ScrambledInt() { m_scrambled = 0; }
    ScrambledInt(unsigned int scrambled) : m_0(0), m_scrambled(scrambled) {}
    ScrambledInt(int value, int) { m_scrambled = scramble(value); }
    int value();                        // 0x004247FE
    unsigned int scramble(int value);   // 0x004249C0

    int m_0;
    unsigned int m_scrambled;           // +0x04
};

// A scrambled constant.
struct Scrambled {
    Scrambled(unsigned int bits) : m_bits(bits) {}

    unsigned int m_bits;
};

// A ScrambledInt with a virtual for its arithmetic (vtable 0x00FE4730; not identified).
class SecureInt {
public:
    SecureInt(Scrambled value) { m_value.m_scrambled = value.m_bits; }
    SecureInt(int value);               // 0x00533DEF
    // The operation that the key picks (0xa8b35b36 adds, 0xa8b35b37 subtracts), on a and b.
    virtual ScrambledInt apply(const unsigned int& key, ScrambledInt& a, ScrambledInt& b);    // 0x004243D4
    SecureInt& operator+=(SecureInt& other);    // 0x00533E38
    SecureInt operator-(SecureInt& other);      // 0x00533EA0
    int value() { return m_value.value(); }

    ScrambledInt m_value;               // +0x04
};

// The same with another scrambling (not identified).
class ScrambledInt2 {
public:
    ScrambledInt2() { m_scrambled = 0; }
    void set(unsigned int scrambled) { m_scrambled = scrambled; }
    int value();                        // 0x0042477C

    int m_0;
    unsigned int m_scrambled;           // +0x04
};

// A SecureInt with ScrambledInt2's scrambling (vtable 0x00FE473C; not identified).
class SecureInt2 {
public:
    SecureInt2(const Scrambled& value) { m_value.set(value.m_bits); }
    virtual void unk_00();              // 0x00423D06: its arithmetic, as SecureInt::apply (a guess)
    int value() { return m_value.value(); }

    ScrambledInt2 m_value;              // +0x04
};

#endif

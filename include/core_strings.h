// The 16-bit strings (libs_daily\ca_libs\core\core_strings.cpp).
#ifndef CORE_STRINGS_H
#define CORE_STRINGS_H

// A string of 16-bit characters (probably ca_libs\core\core_string.cpp).
struct UniChar;

class UniString {
public:
    UniString();                                // 0x00F34F70
    UniString(const char* text);                // 0x00F338B0
    UniString(const char* text, unsigned short length);  // 0x00F33D40: its first length characters
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

UniString uni_format(UniString format, ...);   // 0x00F386C0: a formatted text (a guess)

#endif

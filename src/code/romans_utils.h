// The 8-bit strings and the script reader (code\romans_utils.cpp; the exe names code\romans_utils.h).
#ifndef ROMANS_UTILS_H
#define ROMANS_UTILS_H

#include "unsorted.h"
#include "core_strings.h"

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
    unsigned int read_uint();                   // 0x00E2D408
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

extern "C" {
extern bool lbl_012CA5B8;                       // a script error has been reported
extern char lbl_010B3928[];                     // the script error's text
void fn_00F40060(int);
void fn_00F401D0(const char* format, ...);
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

#endif

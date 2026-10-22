// The console (the ~ key in the game; code\env\dev_console.cpp).
#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "unsorted.h"

class ConsoleCommand;

// The console's commands, and the last 10 lines it was given (0x40 bytes).
class Console {
public:
    Console();                                                // 0x00DE167A
    ~Console();                                               // 0x00DE16CF

    bool run(const char* text, char* out);                    // 0x00DE1718
    void add(ConsoleCommand* command);                        // 0x00DE18FD
    void write_commands(const char* path);                    // 0x00DE19E7: each command and its help
    void list(char* out);                                     // 0x00DE1BFE
    void help(const char* name, char* out);                   // 0x00DE1D23

    Array<ConsoleCommand*> m_commands;          // +0x00: sorted by name
    char* m_history[10];                        // +0x0c
    unsigned int m_num_history;                 // +0x34
    unsigned int m_mode;                        // +0x38: 0 until the battle, campaign or battle editor sets it
    bool m_restricted;                          // +0x3c: set while the game's console runs a line
};

extern Console g_console;

#endif

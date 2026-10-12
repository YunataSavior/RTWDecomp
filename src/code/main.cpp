// The game's entry point. (Its source file's name is not known: no path in the executable names it.)
#include <windows.h>
#include <fstream.h>

#include "unsorted.h"
#include "map_constants.h"

inline void copy_to_ascii(char* dst, const unsigned short* src)
{
    for (;;) {
        char c = (char)*src;
        *dst = c;
        unsigned short w = *src;
        if (w == 0)
            break;
        dst++;
        src++;
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show)
{
    if (!fn_00EFA483(0x1298))
        return 0;
    fn_00425B4F();
    fn_00F32ED0(2500000);
    fn_00402805();

    char* full_command_line = GetCommandLineA();
    UniString arguments(full_command_line);
    char buffer[2001] = {0};
    if (arguments.length() > 2000)
        arguments.truncate(2000);
    copy_to_ascii(buffer, arguments.chars());
    lbl_012CA6B0 = buffer;

    fn_00F3A500(command_line);
    if (!fn_00408DCF())
        return 0;

    if (lbl_012CB1F2 == true) {
        const int size = 0x800;
        ifstream input;
        input.flags(0);
        input.open(lbl_012CB1F4, ios::in);
        char line[size];
        while (!input.eof())
            input.getline(line, size);
        input.close();
        return 0;
    } else if (lbl_012CB1F2 == true) {
        const int size = 0x800;
        ifstream input;
        input.flags(0);
        input.open(lbl_012CB1F4, ios::in);
        char line[size];
        while (!input.eof())
            input.getline(line, size);
        input.close();
        return 0;
    }

    bool is_server = lbl_012CB0D8 == 2 || lbl_012CB0D8 == 3;
    if (!is_server)
        return fn_00403412(instance, prev_instance);
    lbl_012CE880 = lbl_012CB0D8 == 2;
    return fn_00425F8F(instance, prev_instance);
}

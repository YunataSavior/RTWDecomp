#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\romans_utils.cpp"
// (The original's file has only one assert, at 0xE2AAB5, so whether the word functions below are in it is a guess.)
static char THIS_FILE[] = __FILE__;

// Whether c ends a word in the middle of a line: a space, a tab, the end of the string, a carriage return, a comma
// or a colon.
extern "C" bool fn_00E2DB5B(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\0' || c == '\r' || c == ',' || c == ':';
}

// Whether c ends a line: a line feed or a semicolon, which starts a comment.
extern "C" bool fn_00E2DBA6(unsigned char c)
{
    return c == '\n' || c == ';';
}

// Whether c ends a word.
extern "C" bool fn_00E2DBCE(unsigned char c)
{
    return fn_00E2DB5B(c) || fn_00E2DBA6(c);
}

// Whether the first words of a and b are the same. The match is case-sensitive.
extern "C" bool fn_00E2DDF0(const char* a, const char* b)
{
    while (*(const unsigned char*)a != '\0' && *(const unsigned char*)a != '\r' && *(const unsigned char*)a != '\n'
           && *(const unsigned char*)a != ' ' && *(const unsigned char*)a != '\t' && *(const unsigned char*)a != ';'
           && *(const unsigned char*)b != '\0') {
        if (*(const unsigned char*)a != *(const unsigned char*)b)
            return false;
        a++;
        b++;
    }
    return fn_00E2DBCE(*a) && fn_00E2DBCE(*b);
}

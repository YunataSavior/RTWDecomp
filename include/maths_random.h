// CA's random number generator: the C runtime's rand(), with its seed kept in the object.
#ifndef MATHS_RANDOM_H
#define MATHS_RANDOM_H

class Random {
public:
    Random(int seed);                   // 0x00F3AA90
    int next();                         // 0x00F3AAA0: 0 to 0x7fff, as rand()
    int percent();                      // 0x00F3AAD0: 0 to 100, as up_to(100)
    float unit();                       // 0x00F3AB20: 0 to 1
    int up_to(int high);                // 0x00F3AB60: 0 to high
    int range(int low, int high);       // 0x00F3ABB0: low to high

    int m_seed;
};

#endif

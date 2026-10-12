// An 8-byte value of bit fields (not identified), and the constants its header gives each file that includes it.
#ifndef UNK_ID_H
#define UNK_ID_H

struct UnkId {
    UnkId();                                                                // 0x0040617F
    UnkId(unsigned int a, unsigned int b, unsigned int c, unsigned int d);  // 0x004061CB

    unsigned int m_a : 32;
    unsigned int m_b : 20;
    unsigned int m_d : 4;
    unsigned int m_c : 8;
};

static const UnkId UNK_ID_DEFAULT;
static const UnkId UNK_ID_0(0, 0, 0, 0);
static const UnkId UNK_ID_1(0, 0, 1, 0);
static const UnkId UNK_ID_2(1, 0, 0, 0);
static const UnkId UNK_ID_3(2, 0, 0, 0);

#endif

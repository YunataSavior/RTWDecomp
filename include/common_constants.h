// The constants every game file has its own copy of. A header (or a few) included everywhere defines them at
// namespace scope, so each file gets them as file statics, and the 36 initializers of the class-typed and computed
// ones (the run of 10-byte thunks at the start of each file's code) come before the file's own. Which header that is,
// and the constants' names, are not known: the names here say what the values are.
#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

// A UTF-16 character. It has a constructor, so its constants are initialized at startup rather than folded.
struct UniChar {
    UniChar(unsigned short c) : m_c(c) {}

    unsigned short m_c;
};

struct ConstVector2 {
    ConstVector2(float x, float y) : m_x(x), m_y(y) {}

    float m_x;
    float m_y;
};

struct ConstInt {
    ConstInt(int value) : m_value(value) {}

    int m_value;
};

// value / divisor, rounded, written back to value's temporary.
inline const short& divide_round(const short& value, const float& divisor)
{
    int rounded = float_to_int(value / divisor);
    (short&)value = (short)rounded;
    return value;
}

extern "C" unsigned short lbl_02B1834E;

// In .rdata, in this order. Only those a file reads are emitted, so each file's copies of the others must be read by
// header inlines it compiles out of line (whose copies the linker drops).
const float PI = 3.14159265f;
const float k_20 = 20.0f;
const float k_2 = 2.0f;
const float k_300_a = 300.0f;
const float k_300_b = 300.0f;
const float k_0_4 = 0.4f;
const float k_1_7 = 1.7f;
const float k_1 = 1.0f;
const float k_10 = 10.0f;
const float k_0_1 = 0.1f;
const float k_40 = 40.0f;
const float k_75 = 75.0f;
const float k_4000 = 4000.0f;  // road traffic: a road is drawn with alpha 4000 / its traffic

static const UniChar UNI_NULL(0);
static const UniChar UNI_BYTE_ORDER_MARK(0xfeff);
static const UniChar UNI_SPACE(' ');
static const UniChar UNI_UNDERSCORE('_');
static const UniChar UNI_NO_BREAK_SPACE(0xa0);
static const UniChar UNI_ZERO_WIDTH_SPACE(0x200b);
static const UniChar UNI_REVERSED_BYTE_ORDER_MARK(0xfffe);
static const UniChar UNI_TAB('\t');
static const UniChar UNI_CARRIAGE_RETURN('\r');
static const UniChar UNI_LINE_FEED('\n');
static const UniChar UNI_LINE_SEPARATOR(0x2028);
static const UniChar UNI_PARAGRAPH_SEPARATOR(0x2029);
static const UniChar UNI_DIGIT_0('0');
static const UniChar UNI_DIGIT_9('9');
static const UniChar UNI_FULL_STOP('.');
static const UniChar UNI_UPPER_A('A');
static const UniChar UNI_UPPER_Z('Z');
static const UniChar UNI_LOWER_A('a');
static const UniChar UNI_LOWER_Z('z');
static const UniChar UNI_HEX_DIGITS_UPPER[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};
static const UniChar UNI_HEX_DIGITS_LOWER[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static const float ONE_OVER_PI = 1.0f / PI;
static const float RADIANS_TO_ANGLE = 32768.0f / PI;  // a 16-bit angle, 0x8000 a half turn
static const float ANGLE_TO_RADIANS = PI / 32768.0f;
static const float ONE_OVER_20 = 1.0f / k_20;
static const float k_20_over_10 = k_20 / 10.0f;
static const float k_6553_6 = 65536.0f * k_0_1;
static const ConstVector2 VECTOR2_ZERO(0.0f, 0.0f);
static const float k_40_squared = k_40 * k_40;
static const float k_75_squared = k_75 * k_75;
static const short s_unknown_twelfth = divide_round(lbl_02B1834E, 12.0f);
static const ConstInt s_unknown_160(160);
static const ConstInt s_unknown_minus_1(-1);
static const float RADIANS_30_DEGREES = PI / 180.0f * 30.0f;
static const float RADIANS_100_DEGREES = PI / 180.0f * 100.0f;
static const float k_16000 = k_4000 * 4.0f;

#endif

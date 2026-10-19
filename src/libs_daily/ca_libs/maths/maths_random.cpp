#include "maths_random.h"

// The scaled functions spread 0 to 0x7fff over high + 1 values as (r * (high + 1) - 1) / 0x7fff, which gives each
// value about the same share and reaches high only when r is near 0x7fff. So percent() is 0 to 100, not 0 to 99.

Random::Random(int seed)
{
    m_seed = seed;
}

int Random::next()
{
    m_seed = m_seed * 214013 + 2531011;
    return (m_seed >> 16) & 0x7fff;
}

int Random::percent()
{
    m_seed = m_seed * 214013 + 2531011;
    return (((m_seed >> 16) & 0x7fff) * 101 - 1) / 0x7fff;
}

float Random::unit()
{
    m_seed = m_seed * 214013 + 2531011;
    return ((m_seed >> 16) & 0x7fff) * (1.0f / 0x7fff);
}

int Random::up_to(int high)
{
    m_seed = m_seed * 214013 + 2531011;
    return (((m_seed >> 16) & 0x7fff) * (high + 1) - 1) / 0x7fff;
}

int Random::range(int low, int high)
{
    m_seed = m_seed * 214013 + 2531011;
    return (((m_seed >> 16) & 0x7fff) * (high - low + 1) - 1) / 0x7fff + low;
}

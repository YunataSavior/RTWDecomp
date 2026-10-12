// The two constants most game files initialize after common_constants.h's (the 34th and 35th of their header
// initializers). The trait database (character_traits_db.cpp) does not, so they come from another header: include
// this right after unsorted.h. (Which header that is, and the constants' names, are not known.)
#ifndef ANGLE_CONSTANTS_H
#define ANGLE_CONSTANTS_H

#include "common_constants.h"

static const float RADIANS_30_DEGREES = PI / 180.0f * 30.0f;
static const float RADIANS_100_DEGREES = PI / 180.0f * 100.0f;

#endif

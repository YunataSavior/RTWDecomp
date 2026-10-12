// The constant most game files initialize after common_constants.h's (the 36th of their header initializers). The
// database files (landmark_db.cpp) do not, so it comes from another header, one those files do not include: include
// this right after unsorted.h. (Which header that is, and the constant's name, are not known.)
#ifndef MAP_CONSTANTS_H
#define MAP_CONSTANTS_H

#include "angle_constants.h"

static const float k_16000 = k_4000 * 4.0f;

#endif

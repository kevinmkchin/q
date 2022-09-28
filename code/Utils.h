#pragma once

#include "Common.h"

/* Call after OpenGL calls to check for errors. */
bool GLHasErrors();

void PickThreeRandomInts(int* x, int* y, int* z, int cap);

/* Pick random integer in range [min, max] inclusive. */
int RandomInt(int min, int max);

i32 ModifyASCIIBasedOnModifiers(i32 keycodeASCII, bool shift);

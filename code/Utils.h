#pragma once

/* Call after OpenGL calls to check for errors. */
bool GLHasErrors();

void PickThreeRandomInts(int* x, int* y, int* z, int cap);

/* Pick random integer in range [min, max] inclusive. */
int RandomInt(int min, int max);

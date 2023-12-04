#pragma once

#define CONSOLE false

#if CONSOLE == true
#include <iostream>
#endif

#include <fstream>
#include "Point.h"

#define MODULO(x, a) ((((x) % (a)) + (a)) % (a))

using uint = unsigned int;
using c_uint = const uint;
using c_uint8 = const uint8_t;

inline int slide_int(c_int x1, c_int x2, c_int t, c_int ds)
{
	return x1 + (x2 - x1) * t / ds;
}

inline int slide_int8(c_int x1, c_int x2, c_uint8 t)
{
	return x1 + (x2 - x1) * t / UINT8_MAX;
}

inline c_uint8 slide_int8(c_uint8 x1, c_uint8 x2, c_uint8 t)
{
	return x1 + (x2 - x1) * t / UINT8_MAX;
}

inline int get_sign(c_int x)
{
	return (x >> 31) | 1;
}

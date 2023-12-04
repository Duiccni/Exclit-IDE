#pragma once

#include <ostream>
#include <algorithm>

struct point
{
	int x, y;
};

using cpt = const point;
using c_int = const int;

inline point operator -(cpt a) { return { -a.x, -a.y }; }

inline point operator ~(cpt a) { return { a.y, a.x }; }

inline void swap_point_if(point& s, point& b)
{
	if (s.x > b.x) std::swap(s.x, b.x);
	if (s.y > b.y) std::swap(s.y, b.y);
}

inline std::ostream& operator <<(std::ostream& os, cpt p)
{
	os << '(' << p.x << ',' << ' ' << p.y << ')';
	return os;
}

inline void clamp_point(point& p, cpt a, cpt b)
{
	p.x = std::clamp(p.x, a.x, b.x);
	p.y = std::clamp(p.y, a.x, b.x);
}


#define OPERATOR_POINT_ARITHMETIC(SYMBOL) inline point operator SYMBOL(cpt a, cpt b) { return { a.x SYMBOL b.x, a.y SYMBOL b.y }; }
#define OPERATOR_INT_ARITHMETIC(SYMBOL) inline point operator SYMBOL(cpt a, c_int b) { return { a.x SYMBOL b, a.y SYMBOL b }; }
#define OPERATOR_BOOL(SYMBOL, JUNCTION) inline bool operator SYMBOL(cpt a, cpt b) { return a.x SYMBOL b.x JUNCTION a.y SYMBOL b.y; }
#define OPERATOR_POINT_VOID(SYMBOL) inline void operator SYMBOL(point& a, cpt b) { a.x SYMBOL b.x; a.y SYMBOL b.y; }
#define OPERATOR_INT_VOID(SYMBOL) inline void operator SYMBOL(point& a, c_int b) { a.x SYMBOL b; a.y SYMBOL b; }

OPERATOR_POINT_ARITHMETIC(+)
OPERATOR_POINT_ARITHMETIC(-)
OPERATOR_POINT_ARITHMETIC(*)
OPERATOR_POINT_ARITHMETIC(/)
OPERATOR_POINT_ARITHMETIC(%)

OPERATOR_INT_ARITHMETIC(+)
OPERATOR_INT_ARITHMETIC(-)
OPERATOR_INT_ARITHMETIC(*)
OPERATOR_INT_ARITHMETIC(/)
OPERATOR_INT_ARITHMETIC(%)
OPERATOR_INT_ARITHMETIC(&)
OPERATOR_INT_ARITHMETIC(|)
OPERATOR_INT_ARITHMETIC(<<)
OPERATOR_INT_ARITHMETIC(>>)

OPERATOR_BOOL(==, &&)
OPERATOR_BOOL(!=, ||)
OPERATOR_BOOL(<, &&)
OPERATOR_BOOL(>, &&)
OPERATOR_BOOL(<=, &&)
OPERATOR_BOOL(>=, &&)

OPERATOR_POINT_VOID(+=)
OPERATOR_POINT_VOID(-=)
OPERATOR_INT_VOID(+=)
OPERATOR_INT_VOID(-=)
OPERATOR_INT_VOID(<<=)
OPERATOR_INT_VOID(>>=)
#pragma once
#include <utility>
#include <Windows.h>

COORD makeCoord(const int X, const int Y);
COORD positionalDiff(const COORD& pos1, const COORD& pos2);
COORD operator-(const COORD& pos1, const COORD& pos2);
COORD operator+(const COORD& pos1, const COORD& pos2);
COORD operator*(const COORD& pos1, const int);
bool operator<(const COORD& pos1, const COORD& pos2);
bool operator>(const COORD& pos1, const COORD& pos2);
bool operator==(const COORD& pos1, const COORD& pos2);
bool operator!=(const COORD& pos1, const COORD& pos2);
bool operator<=(const COORD& pos1, const COORD& pos2);
bool operator>=(const COORD& pos1, const COORD& pos2);
std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2);
bool posBetween(const COORD& pos, const COORD& first, const COORD& second);
bool segmentsIntersect(const COORD& s1, const COORD& e1, const COORD& s2, const COORD& e2);
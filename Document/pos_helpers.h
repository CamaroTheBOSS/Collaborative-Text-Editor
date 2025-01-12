#pragma once
#include <utility>
#include <Windows.h>

COORD operator-(const COORD& pos1, const COORD& pos2);
COORD operator+(const COORD& pos1, const COORD& pos2);
bool operator<(const COORD& pos1, const COORD& pos2);
bool operator>(const COORD& pos1, const COORD& pos2);
bool operator==(const COORD& pos1, const COORD& pos2);
bool operator!=(const COORD& pos1, const COORD& pos2);
bool operator<=(const COORD& pos1, const COORD& pos2);
bool operator>=(const COORD& pos1, const COORD& pos2);
std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2);
bool posBetween(const COORD& pos, const COORD& first, const COORD& second);
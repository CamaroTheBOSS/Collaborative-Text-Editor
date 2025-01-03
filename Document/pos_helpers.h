#pragma once
#include <utility>
#include <Windows.h>

std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2);
bool smallerPos(const COORD& pos1, const COORD& pos2);
bool equalPos(const COORD& pos1, const COORD& pos2);
COORD diffPos(const COORD& pos1, const COORD& pos2);
COORD sumPos(const COORD& pos1, const COORD& pos2);
bool posBetween(const COORD& pos, const COORD& first, const COORD& second);
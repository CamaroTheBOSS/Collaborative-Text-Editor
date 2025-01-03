#include "pos_helpers.h"

bool smallerPos(const COORD& pos1, const COORD& pos2) {
	if (pos1.Y < pos2.Y) {
		return true;
	}
	else if (pos1.Y == pos2.Y && pos1.X < pos2.X) {
		return true;
	}
	return false;
}

std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2) {
	if (smallerPos(pos1, pos2)) {
		return { &pos1, &pos2 };
	}
	return { &pos2, &pos1 };
}

bool equalPos(const COORD& pos1, const COORD& pos2) {
	return pos1.X == pos2.X && pos1.Y == pos2.Y;
}

COORD diffPos(const COORD& pos1, const COORD& pos2) {
	return COORD{ static_cast<SHORT>(pos1.X - pos2.X), static_cast<SHORT>(pos1.Y - pos2.Y) };
}

COORD sumPos(const COORD& pos1, const COORD& pos2) {
	return COORD{ static_cast<SHORT>(pos1.X + pos2.X), static_cast<SHORT>(pos1.Y + pos2.Y) };
}

bool posBetween(const COORD& pos, const COORD& first, const COORD& second) {
	auto [left, right] = getAscendingOrder(first, second);
	return smallerPos(*left, pos) && smallerPos(pos, *right);
}
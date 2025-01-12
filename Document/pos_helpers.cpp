#include "pos_helpers.h"

COORD operator-(const COORD& pos1, const COORD& pos2) {
	return COORD{ static_cast<SHORT>(pos1.X - pos2.X), static_cast<SHORT>(pos1.Y - pos2.Y) };
}

COORD operator+(const COORD& pos1, const COORD& pos2) {
	return COORD{ static_cast<SHORT>(pos1.X + pos2.X), static_cast<SHORT>(pos1.Y + pos2.Y) };
}

bool operator<(const COORD& pos1, const COORD& pos2) {
	if ((pos1.Y < pos2.Y) || (pos1.Y == pos2.Y && pos1.X < pos2.X)) {
		return true;
	}
	return false;
}

bool operator>(const COORD& pos1, const COORD& pos2) {
	if ((pos1.Y > pos2.Y) || (pos1.Y == pos2.Y && pos1.X > pos2.X)) {
		return true;
	}
	return false;
}

bool operator==(const COORD& pos1, const COORD& pos2) {
	return pos1.Y == pos2.Y && pos1.X == pos2.X;
}

bool operator!=(const COORD& pos1, const COORD& pos2) {
	return pos1.Y != pos2.Y || pos1.X != pos2.X;
}

bool operator<=(const COORD& pos1, const COORD& pos2) {
	return pos1 < pos2 || pos1 == pos2;
}

bool operator>=(const COORD& pos1, const COORD& pos2) {
	return pos1 > pos2 || pos1 == pos2;
}

std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2) {
	if (pos1 <= pos2) {
		return { &pos1, &pos2 };
	}
	return { &pos2, &pos1 };
}

bool posBetween(const COORD& pos, const COORD& first, const COORD& second) {
	auto [left, right] = getAscendingOrder(first, second);
	return *left <= pos && pos <= *right;
}
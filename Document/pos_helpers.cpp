#include "pos_helpers.h"

COORD makeCoord(const int X, const int Y) {
	return COORD{ static_cast<SHORT>(X), static_cast<SHORT>(Y) };
}

COORD positionalDiff(const COORD& pos1, const COORD& pos2) {
	if (pos1.Y == pos2.Y) {
		return pos1 - pos2;
	}
	if (pos1 > pos2) {
		return COORD{ pos1.X, static_cast<SHORT>(pos1.Y - pos2.Y) };
	}
	return COORD{ static_cast<SHORT>(-pos1.X), static_cast<SHORT>(pos1.Y - pos2.Y) };
}

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
	return *left < pos && pos < *right;
}

bool segmentsIntersect(const COORD& s1, const COORD& e1, const COORD& s2, const COORD& e2) {
	return posBetween(s1, s2, e2) || posBetween(e1, s2, e2) || posBetween(s2, s1, e1) || posBetween(e2, s1, e1) || (s1 == s2 && e1 == e2);
}